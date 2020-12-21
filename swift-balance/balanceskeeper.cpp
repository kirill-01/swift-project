#include "balanceskeeper.h"
#include <QList>

class BalancesKeeperData : public QSharedData
{
public:
    QMap<quint32,double> _available;
    QMap<quint32,double> _total;
    QMap<quint32,double> _requerements;
    QMap<QString, QJsonObject> _withdraws;
    QMap<QString, QJsonObject> _deposits;
    QMap<QString, SwiftBot::Transaction> _transactions;
    QJsonObject getAllJson();
    QDateTime last_bals_saved_ts;
    SwiftBot::Balances _balances_storage;
};

BalancesKeeper::BalancesKeeper(QObject *parent) : QObject(parent),
    update_bals_timer( new QTimer(this) ),
    update_deposits_timer( new QTimer(this) ),
    update_withdraws_timer( new QTimer(this) ),
    session(nullptr),
    data(new BalancesKeeperData),
    requirements_timer( new QTimer(this) ),
    is_active_module(true),
    is_debug( SwiftBot::moduleParam("is_debug", false).toBool() )
{

    QSettings * settings( SwiftCore::getModuleSettings("balance") );
    const quint32 bals_interval = settings->value("balance_update_interval", 30000).toUInt();
    if ( bals_interval > 0 ) {
        update_bals_timer->setInterval( bals_interval );
        QObject::connect( update_bals_timer, &QTimer::timeout, this, &BalancesKeeper::requestBals );

    }
    const quint32 deps_interval = settings->value("deposits_update_interval", 360000).toUInt();
    if ( deps_interval > 0 ) {
        update_deposits_timer->setInterval( deps_interval );
        QObject::connect( update_deposits_timer, &QTimer::timeout, this, &BalancesKeeper::requestDeposits );

    }
    const quint32 with_interval = settings->value("withdraws_update_interval", 420000).toUInt();
    if ( with_interval > 0 ) {
        update_withdraws_timer->setInterval( with_interval );
        QObject::connect( update_withdraws_timer, &QTimer::timeout, this, &BalancesKeeper::requestWithdraws );

    }

    const quint32 requirements_interval = settings->value("requirements_update_interval", 1500000).toUInt();
    if ( requirements_interval > 0 ) {
        requirements_timer->setInterval( requirements_interval );
        QObject::connect( requirements_timer, &QTimer::timeout, this, &BalancesKeeper::calculateRequirements );

    }

    use_only_exchange_accs =  settings->value("use_only_exchange_account", true).toBool();
    connect( this, &BalancesKeeper::updateEvent, this, &BalancesKeeper::onUpdateEvent );
    connect( this, &BalancesKeeper::withdrawsEvent, this, &BalancesKeeper::onWithdrawsEvent );
    connect( this, &BalancesKeeper::depositsEvent, this, &BalancesKeeper::onDepositsEvent );

    QSqlQuery q("SELECT * FROM deposits");
    if ( q.exec() ) {
        while ( q.next() ) {
            data->_transactions.insert( q.value("remote_id").toString(), SwiftBot::Transaction( q.record(), 1 ) );
        }
    }
    if ( q.exec("SELECT * FROM withdtaws") ) {
        while ( q.next() ) {
            data->_transactions.insert( q.value("remote_id").toString(), SwiftBot::Transaction( q.record(), 0 ) );
        }
    }

    connect( this, &BalancesKeeper::sendBalanceInfo, this, &BalancesKeeper::saveBalancesHistory );
    QTimer::singleShot( 60000, this, &BalancesKeeper::saveBalsInterval );
}

BalancesKeeper::BalancesKeeper(const BalancesKeeper &rhs) : QObject(nullptr), data(rhs.data)
{

}

BalancesKeeper &BalancesKeeper::operator=(const BalancesKeeper &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

BalancesKeeper::~BalancesKeeper()
{

}

void BalancesKeeper::setBalanceRequired(const quint32 &cid, const double &amount)
{
    data->_requerements[ cid ] = amount;
}

double BalancesKeeper::getBalanceRequired(const quint32 &cid) const
{
    return data->_requerements.value( cid, 0 );
}

void BalancesKeeper::setBalanceAvailable(const quint32 &cid, const double &amount) {
    data->_available[ cid ] = amount;
}

double BalancesKeeper::getBalanceAvailable(const quint32 &cid) const {
    return data->_available.value( cid, 0 );
}

void BalancesKeeper::setBalanceTotal(const quint32 &cid, const double &amount) {
    data->_total[ cid ] = amount;
}

double BalancesKeeper::getBalanceTotal(const quint32 &cid) const {
    return data->_total.value( cid, 0 );
}

void BalancesKeeper::calculateRequirements() {
    QSqlQuery q("SELECT sell_pair_id as sell_pair, c1.id as sell_currency, SUM(max_amount) / count(*) as sell_amount , buy_pair_id as buy_pair_id, c2.id as buy_currency, ( SUM(max_amount) / count(*) * AVG(max_buy_rate) )  as buy_price,  avg(max_profit) as profit  FROM arbitrage_events e left join pairs m1 on m1.id=sell_pair_id left join currencies c1 on c1.id=m1.base_currency_id left join pairs m2 on m2.id=buy_pair_id left join currencies c2 on c2.id=m2.market_currency_id where ts >  date_sub(NOW(), interval 1 hour) GROUP by sell_pair_id, buy_pair_id");
    if ( q.exec() ) {
        QMap<quint32, double> _curs_req;
        while ( q.next() ) {
            const quint32 scur( q.value("sell_currency").toUInt() );
            const quint32 bcur( q.value("buy_currency").toUInt() );
            if ( SwiftCore::getAssets()->isCurrencyActive( scur ) && SwiftCore::getAssets()->isCurrencyActive( bcur ) ) {
                const double sreq( q.value("sell_amount").toDouble() );
                if ( _curs_req.value( scur ) < sreq ) {
                    _curs_req[ scur ] = sreq;
                }

                const double breq( q.value("buy_price").toDouble() );
                if ( _curs_req.value( bcur ) < breq ) {
                    _curs_req[ bcur ] = breq;
                }
            }
        }
        data->_requerements.swap( _curs_req );
    }
    if ( session != nullptr && session->isJoined() ) {

        QString msg("<u><b>Required balances:</b></u>\n\n");
        const QMap<quint32,double> _rb( data->_requerements );
        for( auto it = _rb.begin(); it != _rb.end(); it++ ) {
            const quint32 curid( it.key() );
            const double required( it.value() );
            const QString ename( SwiftCore::getAssets()->getCurrencyExchangeName( curid ) );
            const QString cname( SwiftCore::getAssets()->getCoinName( SwiftCore::getAssets()->getCurrencyCoin( curid ) ) );

            if ( required > 0 ) {
                msg+="<u>"+ename+"</u>: "+QString::number( required,'f',4)+" "+cname+"\n";
            }
        }
        msg += "\n<u><b>Total balances:</b></u>\n\n";
        const QMap<quint32,double> _ab( data->_total );
        for( auto it = _ab.begin(); it != _ab.end(); it++ ) {
            const quint32 curid( it.key() );
            const double available( it.value() );
            const QString ename( SwiftCore::getAssets()->getCurrencyExchangeName( curid ) );
            const QString cname( SwiftCore::getAssets()->getCoinName( SwiftCore::getAssets()->getCurrencyCoin( curid ) ) );
            if ( available > 0 ) {
                msg+="<u>"+ename+"</u>: "+QString::number( available,'f',6)+" "+cname+"\n";
            }
        }
        session->call( RCP_TELEGRAM_NOTIFY, {msg} );
    }
}

void BalancesKeeper::onUpdateEvent(const QJsonObject &j_data)
{
    if ( is_debug ) {
        qInfo() << "Balances update event" << j_data;
    }
    const QJsonArray items( j_data.value("items").toArray() );
    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            SwiftBot::Balance bal = it->toObject();
            bal.requested = data->_requerements.value( bal.currency_id, 0 );
            //data->_balances_storage[ bal.currency_id ] = bal;
            setBalanceAvailable( bal.currency_id, bal.available );
            setBalanceTotal( bal.currency_id, bal.total );
        }
    }
}

void BalancesKeeper::onDepositsEvent(const QJsonObject &j_data)
{
    if ( is_debug ) {
        qInfo() << "Deposits event" << j_data;
    }
    const QJsonArray items( j_data.value("items").toArray() );
    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            QJsonObject j( it->toObject() );
            j["type"] = "1";
            const QString remid( j.value("remote_id").toString() );
            if ( data->_transactions.contains( remid ) ) {
                data->_transactions[ remid ].update( j );
            } else {
                data->_transactions.insert( remid, SwiftBot::Transaction( j ) );
                data->_transactions[remid].save();
            }
            data->_deposits[ remid ] = it->toObject();
        }
    }
}

void BalancesKeeper::onWithdrawsEvent(const QJsonObject &j_data)
{
    if ( is_debug ) {
        qInfo() << "Withdraws event" << j_data;
    }
    const QJsonArray items( j_data.value("items").toArray() );
    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            QJsonObject j( it->toObject() );
            j["type"] = "0";
            const QString remid( j.value("remote_id").toString() );
            if ( data->_transactions.contains( remid ) ) {
                data->_transactions[ remid ].update( j );
            } else {
                data->_transactions.insert( remid, SwiftBot::Transaction( j ) );
                data->_transactions[remid].save();
            }

            data->_withdraws[ remid ] = it->toObject();
        }
    }
}

void BalancesKeeper::requestBals() {
    if ( is_debug ) {
        qInfo() << "Requesting updated balances";
    }
    if ( !_active_clients.isEmpty() ) {

        for( auto it = _active_clients.begin(); it != _active_clients.end(); it++ ) {
            if ( session != nullptr && session->isJoined() ) {
                QVariant resp = session->call("swift.api.balances."+*it);
                if ( resp.toULongLong() < 100 ) {
                    qWarning() << "Error receive balances" << *it;
                    // Request error, pause exchange
                }
            }
        }
    } else {
        if ( is_debug ) {
            qWarning() << "Updating clients list";
        }
        _active_clients = getConnectedExchanges();
    }
}

void BalancesKeeper::requestWithdraws() {
    if ( !_active_clients.isEmpty() ) {
        for( auto it = _active_clients.begin(); it != _active_clients.end(); it++ ) {
            if ( session != nullptr && session->isJoined() ) {
                QVariant resp = session->call("swift.api.withdraw.history."+*it);
                if ( resp.toULongLong() < 100 ) {
                    // Request error, pause exchange
                }
            }
        }
    }
}

void BalancesKeeper::requestDeposits() {
    if ( !_active_clients.isEmpty() ) {
        for( auto it = _active_clients.begin(); it != _active_clients.end(); it++ ) {
            if ( session != nullptr && session->isJoined() ) {
                QVariant resp = session->call("swift.api.deposits.history."+*it);
                if ( resp.toULongLong() < 100 ) {
                    // Request error, pause exchange
                }
            }
        }
    }
}

void BalancesKeeper::onWampSession(Wamp::Session *sess) {
    session = sess;

    // Events listener
    session->subscribe( FEED_EVENTS_BALANCES, [=]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m)
        const QString event_name( v.at(0).toString() );
        const QJsonObject j_data( QJsonDocument::fromJson( v.at(1).toString().toUtf8() ).object() );
        if ( event_name == EVENTS_NAME_BALANCES_UPDATE ) {
            emit updateEvent( j_data );
        } else if ( event_name == EVENTS_NAME_WITHDRAWS_HISTORY ) {
            emit withdrawsEvent( j_data );
        } else if ( event_name == EVENTS_NAME_DEPOSITS_HISTORY ) {
            emit depositsEvent( j_data );
        }
    });


    // Getter available
    session->provide( RPC_BALANCE_GET,  [&]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m);
        const quint32 cid = v.at(0).toUInt();
        const double bal = getBalanceAvailable( cid );
        return bal;
    });

    // Getter total
    session->provide( RPC_BALANCE_GET_TOTAL,  [&]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m);
        const quint32 cid = v.at(0).toUInt();
        const double bal = getBalanceTotal( cid );
        return bal;
    });

    // Getter all balances
    session->provide( RPC_BALANCE_ALL,  [=]( const QVariantList& v, const QVariantMap& m ) {
        QJsonObject j_ret( data->getAllJson() );
        if ( QDateTime::currentDateTime().toSecsSinceEpoch() - last_bals_saved_ts.toSecsSinceEpoch() >= 60 ) {
            emit sendBalanceInfo( j_ret.value("totalusd").toString().toDouble(), j_ret.value("ch3").toString().toDouble() );
        }
        Q_UNUSED(m);
        Q_UNUSED(v);
        const QString str_ret( QJsonDocument(j_ret).toJson( QJsonDocument::Compact ) );
        return str_ret;
    });

    session->provide("swift.balance.sendinfo", [=]( const QVariantList&v, const QVariantMap&m ) {
        Q_UNUSED(m);
        Q_UNUSED(v);
        QTimer::singleShot( 100, this, &BalancesKeeper::calculateRequirements );
        return 1;
    });

    // Getter deposits history
    session->provide( RPC_BALANCE_DEPOSITS,  [&]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m);
        Q_UNUSED(v);
        QJsonArray ret;
        QList<QJsonObject> src( data->_deposits.values() );
        for( auto it = src.begin(); it != src.end(); it++ ) {
            ret.push_back( *it );
        }
        const QString r( QJsonDocument( ret ).toJson( QJsonDocument::Compact ) );
        return r;
    });

    // Getter withdraws history
    session->provide( RPC_BALANCE_WITHDRAWS,  [&]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m);
        Q_UNUSED(v);
        QJsonArray ret;
        QList<QJsonObject> src( data->_withdraws.values() );
        for( auto it = src.begin(); it != src.end(); it++ ) {
            ret.push_back( *it );
        }
        const QString r( QJsonDocument( ret ).toJson( QJsonDocument::Compact ) );
        return r;
    });

    QTimer::singleShot( 15000, this, &BalancesKeeper::calculateRequirements );
    QTimer::singleShot( 10000, this, &BalancesKeeper::checkPauseCancel );
    update_deposits_timer->start();
    update_bals_timer->start();
    update_withdraws_timer->start();
    requirements_timer->start();
}

void BalancesKeeper::checkPauseCancel() {
    if ( !is_active_module ) {
        _active_clients = getConnectedExchanges();
        if ( _active_clients.isEmpty() ) {
            QTimer::singleShot( 600000, this, &BalancesKeeper::checkPauseCancel );
        } else {
            QSettings * settings( SwiftCore::getModuleSettings("balance") );
            const quint32 bals_interval = settings->value("balance_update_interval", 30000).toUInt();
            if ( bals_interval > 0 ) {
                update_bals_timer->setInterval( bals_interval );
                update_bals_timer->start();
            }
            const quint32 deps_interval = settings->value("deposits_update_interval", 360000).toUInt();
            if ( deps_interval > 0 ) {
                update_deposits_timer->setInterval( deps_interval );
                update_deposits_timer->start();
            }
            const quint32 with_interval = settings->value("withdraws_update_interval", 420000).toUInt();
            if ( with_interval > 0 ) {
                update_withdraws_timer->setInterval( with_interval );
                update_withdraws_timer->start();
            }

            const quint32 requirements_interval = settings->value("requirements_update_interval", 1500000).toUInt();
            if ( requirements_interval > 0 ) {
                requirements_timer->setInterval( requirements_interval );
                requirements_timer->start();
            }
            is_active_module = true;
            qWarning() << "BALANCES: Module restarted!";
        }
    }
}

void BalancesKeeper::saveBalancesHistory(const double &b, const double &p) {
    if ( b == 0 && p == 0 ) {
        return;
    }
    if( is_debug ) {
        qWarning() << "Saving balances history" << b << p;
    }
    QSqlQuery q("INSERT INTO `balance_history`( `profit`, `total` ) VALUES ("+QString::number(p,'f',8)+","+QString::number(b,'f',8)+") ON DUPLICATE KEY UPDATE profit=VALUES(profit),total=VALUES(total);");
    if ( !q.exec() ) {
        qWarning() << q.lastError().text();
    } else {
        last_bals_saved_ts = QDateTime::currentDateTime();
    }

}

void BalancesKeeper::saveBalsInterval() {
    QJsonObject j_ret( data->getAllJson() );
    if ( QDateTime::currentDateTime().toSecsSinceEpoch() - last_bals_saved_ts.toSecsSinceEpoch() >= 60 ) {
        emit sendBalanceInfo( j_ret.value("totalusd").toString().toDouble(), j_ret.value("ch3").toString().toDouble() );
    }
    QTimer::singleShot( 60000, this, &BalancesKeeper::saveBalsInterval );
}

void BalancesKeeper::pauseModule() {
    if ( is_active_module ) {
        if ( session != nullptr && session->isJoined() ) {
            QString msg("<u><b>No accounts data</b></u>\nInformation about balances unavailable now.\n<u>Suspending balances module.</u>");
            QVariant id = session->call( RCP_TELEGRAM_NOTIFY, {msg} );
        }
        update_bals_timer->stop();
        update_deposits_timer->stop();
        update_withdraws_timer->stop();
        requirements_timer->stop();
        is_active_module = false;
    }
}

QStringList BalancesKeeper::getConnectedExchanges() {
    QMutexLocker lock( &m );
    qWarning() << "Updating active exchanges list";
    if ( session != nullptr && session->isJoined() ) {
        const QString _targetsstr = session->call( RPC_EXCHANGES_LIST_COMMAND ).toString();
        const QStringList exchsList( _targetsstr.split(",") );
        qWarning() << "Updating active exchanges list : " << _targetsstr;
        if ( !exchsList.isEmpty() ) {
            for( auto it = exchsList.begin(); it!= exchsList.end(); it++ ) {
                QVariant res = session->call("swift.api.status."+*it );
                const QString res_str( res.toString() );
                qWarning() << res_str;
                if ( !res_str.isEmpty() ) {
                    const QJsonObject j_api_state( QJsonDocument::fromJson( res_str.toUtf8() ).object() );
                    if ( j_api_state.value("public_methods").toBool( false ) &&
                         j_api_state.value("private_methods").toBool( false ) ) {
                        _active_clients.push_back( *it );
                    } else {
                        qWarning() << "BALANCES" << *it << "Exchange client is unavailable" << j_api_state;
                    }
                }
            }
        }
        if ( _active_clients.isEmpty() && is_active_module ) {
            pauseModule();
            QTimer::singleShot( 600000, this, &BalancesKeeper::checkPauseCancel );
        }
    }
    return _active_clients;
}

QJsonObject BalancesKeeperData::getAllJson() {
    QJsonObject j_obj;
    QJsonArray j_res_items;
    double totalusd = 0;
    double totalchange = 0;
    SwiftBot::Coin::forEachCoin([&totalusd, &totalchange, &j_res_items, this]( SwiftBot::Coin coin ) {
        const QString coinname( coin.name );
        double balance = 0;
        const double rate = coin.id == 3 ? 1 : coin.usd_rate();
        const double rate_diff = coin.usd_rate_changed();
        QJsonArray j_curs;
        QJsonArray j_addresses;
        coin.forEachCurrency( [&balance, &rate, &j_curs, &j_addresses, this ]( SwiftBot::Currency cur ) {
            balance += _total.value( cur.id );
            const QString ename = cur.exchange().name;
            QJsonObject jcuritm;
            jcuritm["name"] = ename;
            jcuritm["amount"] = _total.value( cur.id );
            jcuritm["usd"] = _total.value( cur.id ) * rate;
            j_curs.push_back( jcuritm );
            QJsonObject jaddritem;
            jaddritem["exchange_name"] = ename;
            jaddritem["address"] = SwiftCore::getAssets()->getCurrencyAddress( cur.id );
            j_addresses.push_back( jaddritem );
        });

        const double changes_by_rate = ( rate * balance ) - ( ( rate - rate_diff ) * balance );
        totalchange += changes_by_rate;
        QJsonObject jitm;
        jitm["name"] = coinname;
        jitm["p1"] = QString::number( balance, 'f', 8 );
        jitm["p2"] = QString::number( coin.usd_rate(), 'f', 8 );
        jitm["p3"] = QString::number( balance * rate );
        jitm["p4"] = QString::number( rate_diff, 'f', 8 );
        jitm["p5"] = QString::number( rate_diff / rate * 100, 'f', 2 );
        jitm["sl"] = coin.getSparkLine();
        jitm["exchanges"] = j_curs;
        jitm["addresses"] = j_addresses;
        QJsonArray jtxs;
        for ( auto it = _transactions.begin(); it != _transactions.end(); it++ ) {
            if( coin.id == SwiftCore::getAssets()->getCurrencyCoin( it.value().currency_id ) ) {
                QJsonObject rec( it.value().toJson() );

                rec["target"] = SwiftCore::getAssets()->getCurrencyExchangeName( it.value().currency_id  );
                rec["coin"] = coinname;
                rec["amount_usd"] = QString::number( rec.value("amount").toString().toDouble() * rate, 'f', 8);
                jtxs.push_back( rec );
            }
        }
        jitm["txs"] = jtxs;
        totalusd += balance * rate;
        j_res_items.push_back( jitm );
    });

    QSqlQuery q("SELECT * FROM swiftbot.balance_history WHERE ts < date_sub(NOW(), interval 24 hour) ORDER BY ts DESC LIMIT 1");
    if ( q.exec() && q.next() ) {
        const double prev_history_record = q.value("total").toDouble();
        totalchange = totalusd - prev_history_record;
    }

    j_obj["items"] = j_res_items;
    j_obj["ch1"] = QString::number(totalchange,'f', 4 ); // Changes by rate 24
    j_obj["ch2"] = QString::number(totalchange / totalusd * 100,'f', 2 );  // Changes by rate 24 in pcnt
    j_obj["ch3"] = QString::number(0,'f', 4 );  // Profit received
    j_obj["ch4"] = QString::number(0,'f', 2 );  // ROI
    j_obj["totalusd"] = QString::number( totalusd, 'f', 8 );

    return j_obj;
}
