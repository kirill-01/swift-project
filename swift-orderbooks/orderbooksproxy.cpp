#include "orderbooksproxy.h"

OrderBooksProxy::OrderBooksProxy(QObject *parent) : QObject(parent),
    send_rates_interval(new QTimer(this)),session(nullptr),
    pub_timer(nullptr),  limit_records_count(10),
    _rates_logger( new QTimer ),
    assets( SwiftCore::getAssets() )
{

    _rates_logger->setInterval( 3600000 );

    //orderbooks_interval = new QTimer(this);
    //orderbooks_interval->setInterval( SwiftCore::getModuleSettings("orderbooks")->value("request_interval", 1500 ).toUInt() );

    connect( _rates_logger, &QTimer::timeout, this, &OrderBooksProxy::logRates );
    //connect( orderbooks_interval, &QTimer::timeout, this, &OrderBooksProxy::getAllOrderBooks, Qt::QueuedConnection );
    //connect( this, &OrderBooksProxy::getOrderBooks, this, &OrderBooksProxy::onRequestOrderBooks, Qt::QueuedConnection );
    //connect( this, &OrderBooksProxy::orderBooks, this, &OrderBooksProxy::mergeOrderbooks  );

    //const quint32 publish_interval( SwiftCore::getModuleSettings("orderbooks")->value("publish_interval", 0 ).toUInt() );
    limit_records_count = SwiftCore::getModuleSettings("orderbooks")->value("limit_records_cnt", 10 ).toUInt();

    //if ( publish_interval > 0 ) {
    //    pub_timer = new QTimer( this );
    //    pub_timer->setInterval( publish_interval );
    //    connect( pub_timer, &QTimer::timeout, this, &OrderBooksProxy::publish );
    //}

    valid_interval = SwiftCore::getModuleSettings("orderbooks")->value("orderbooks_valid_time", 4500 ).toUInt();
    send_rates_interval->setInterval( SwiftCore::getModuleSettings("orderbooks")->value("send_rates_interval", 900000 ).toUInt() );

    connect( send_rates_interval, &QTimer::timeout, this, &OrderBooksProxy::sendRates );

    qInfo() << "Orderbooks valid timer: " << valid_interval;
    qInfo() << "Orderbooks update interval: " << SwiftCore::getModuleSettings("orderbooks")->value("request_interval", 1500 ).toUInt() ;
    qInfo() << "Orderbooks publishing interval: " << SwiftCore::getModuleSettings("orderbooks")->value("publish_interval", 1500 ).toUInt() ;
    qInfo() << "Orderbooks slicing: " << QString::number( limit_records_count );

    send_rates_interval->start();
}

bool OrderBooksProxy::isDebug() {
    return SwiftCore::getSettings()->value( SETTINGS_NAME_MODULES_DEBUG, false ).toBool();
}

void OrderBooksProxy::addLog(const QString &msg) {
    if ( session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_LOGS, {"orderbooks","INFO",msg});
    }
}

void OrderBooksProxy::addError(const QString &msg) {
    if ( session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_LOGS, {"orderbooks","WARNING",msg});
    }
}

void OrderBooksProxy::addDebug(const QString &msg) {
    if ( session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_LOGS, {"orderbooks","DEBUG",msg});
    }
}

void OrderBooksProxy::processQueue() {

}

void OrderBooksProxy::publish() {

}

void OrderBooksProxy::getAllOrderBooks() {

}

void OrderBooksProxy::onRequestOrderBooks(const quint32 &_pair) {
    Q_UNUSED(_pair)

}

void OrderBooksProxy::onWampSession(Wamp::Session *sess) {
    _pairs = assets->getActiveMarkets();
    session = sess;
    reloadTargets();

    qInfo() << _pairs.count() << "active markets";

    // Get actual rates info
    session->provide( RPC_CURRENT_RATES, [=]( const QVariantList&v, const QVariantMap& m ) {
       Q_UNUSED(m)
       Q_UNUSED(v)

       QJsonObject r;
       QJsonArray ar;
       QJsonArray br;
       for( auto iit = _current_rates.begin(); iit != _current_rates.end(); iit++ ) {
           if ( assets->isMarketActive( iit.key() ) ) {
               r[ QString::number( iit.key() ) ] = QString::number(iit.value(), 'f', 8);
           }
       }
       const QString rr( QJsonDocument( r ).toJson( QJsonDocument::Compact ) );
       return rr;
    });
    session->provide( RPC_CURRENT_RATES_MSG, [=]( const QVariantList&v, const QVariantMap& m ) {
       Q_UNUSED(m)
       Q_UNUSED(v)
       QTimer::singleShot( 750, this, &OrderBooksProxy::sendRates );
       return "OK";
    });



    // Coins convertor
    session->provide( RPC_RATE_CONVERT, [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        const quint32 from_coin = v.at(0).toUInt();
        const quint32 to_coin = v.at(1).toUInt();
        const double amount = v.at(2).toDouble();

        qInfo() << "Converting " << from_coin << to_coin << amount;

        if ( from_coin == to_coin ) {
            return amount;
        } else if ( from_coin == 0 || to_coin == 0 ) {
            return 0.0;
        } else {
            const quint32 arb_pair = assets->getArbitragePairByCoins( from_coin, to_coin );
            if ( arb_pair == 0 ) {
                const quint32 arb_pair2 = assets->getArbitragePairByCoins( to_coin, from_coin );
                if ( arb_pair2 == 0 ) {
                    return 0.0;
                } else {
                    QList<quint32> _pairs( assets->getArbitragePairs().value( arb_pair ) );
                    if ( _pairs.isEmpty() ) {
                        return 0.0;
                    } else {
                        for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
                            if ( _current_rates.contains( *it ) ) {
                                const double ret = amount/_current_rates.value( *it );
                                return ret;
                            }
                        }
                    }
                }
            } else {
                QList<quint32> _pairs( assets->getArbitragePairs().value( arb_pair ) );
                if ( _pairs.isEmpty() ) {
                    return 0.0;
                } else {
                    for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
                        if ( _current_rates.contains( *it ) ) {
                            const double ret = _current_rates.value( *it ) * amount;
                            return ret;
                        }
                    }
                }
            }

        }
        return 0.0;
    });

    // Get last cached orderbooks for market_id
    session->provide( RPC_ORDERBOOKS_CACHE, [&]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        if ( v.isEmpty() ) {
            return QString("{}");
        }

        const quint32 pid = v.at(0).toUInt();

        QJsonObject r;
        QJsonArray ar;
        QJsonArray br;

        const QMap<double,double> _a( _asks.value( pid ) );
        for( auto iit = _a.begin(); iit != _a.end(); iit++ ) {
            ar.push_back( QJsonArray({ QString::number( pid ), QString::number( iit.key(), 'f', 8), QString::number( iit.value(), 'f', 8)}));
        }

        const QMap<double,double> _b( _bids.value( pid ) );
        for( auto iit = _b.begin(); iit != _b.end(); iit++ ) {
            br.push_back( QJsonArray({ QString::number( pid ), QString::number( iit.key(), 'f', 8), QString::number( iit.value(), 'f', 8)}));
        }
        r["asks"] = ar;
        r["bids"] = br;
        const QString rr( QJsonDocument( r ).toJson( QJsonDocument::Compact ) );
        return rr;
    });

    const QString str_exchs = session->call( RPC_MODULES_LIST_COMMAND ).toString();
    const QJsonArray j_exchs( QJsonDocument::fromJson( str_exchs.toUtf8() ).array() );
    for( auto it = j_exchs.begin(); it != j_exchs.end(); it++ ) {
        _exchanges.push_back( it->toObject().value("name").toString() );
    }

    QTimer::singleShot( 30000, this, &OrderBooksProxy::sendRates );
}

void OrderBooksProxy::mergeOrderbooks(const QString& topic,const QJsonObject &j_books) {
    Q_UNUSED(topic)
    QMutexLocker lock( &m );
    QHash<quint32, QMap<double,double>> _new_asks, _new_bids;

    const QJsonArray temo_asks( j_books.value("asks").toArray() );
    for( auto it = temo_asks.begin(); it != temo_asks.end(); it++ ) {

        // const quint32 pair_id( SwiftLib::getAssets()->ge);
        //if ( _available_pairs.contains( it->toArray().first().toArray().first().toString().toUInt() )) {
            const QJsonArray itm( it->toArray() );
            if ( !itm.isEmpty() ) {
                for( auto it1 = itm.begin(); it1 != itm.end(); it1++ ) {
                    _new_asks[ it1->toArray().at(0).toString().toInt() ].insert( it1->toArray().at(1).toString().toDouble(), it1->toArray().at(2).toString().toDouble() );
                    _last_updates[ it1->toArray().at(0).toString().toInt() ] = QDateTime::currentSecsSinceEpoch();
                }
            }
        //}
    }
    const QJsonArray temo_bids( j_books.value("bids").toArray() );
    for( auto it = temo_bids.begin(); it != temo_bids.end(); it++ ) {
       // if ( _available_pairs.contains( it->toArray().first().toArray().first().toString().toUInt() )) {
            const QJsonArray itm( it->toArray() );
            if ( !itm.isEmpty() ) {
                for( auto it1 = itm.begin(); it1 != itm.end(); it1++ ) {
                    _new_bids[ it1->toArray().at(0).toString().toInt() ].insert( it1->toArray().at(1).toString().toDouble(), it1->toArray().at(2).toString().toDouble() );
                    _last_updates[ it1->toArray().at(0).toString().toInt() ] = QDateTime::currentSecsSinceEpoch();
                }
            }
   //     }
    }
    _asks = _new_asks;
    _bids = _new_bids;

    for ( auto it = _bids.begin(); it != _bids.end(); it++ ) {
        if ( _asks.contains( it.key() ) &&  !_asks[it.key()].isEmpty() ) {
            const double as = _asks.value( it.key() ).lastKey();
            const double bi = it.value().firstKey();
            const double rate( as > bi ? as - ( ( as - bi ) / 2 ) : bi - ( ( bi - as ) /2 ) );
            _current_rates[ it.key() ] = rate;
            _rates_history_data[it.key()][ QDateTime::currentSecsSinceEpoch() ] = rate;
        }
    }


}

void OrderBooksProxy::checkOutDated() {
    return;
    const QMap<quint32, QDateTime> _timers( _last_update_times );
    for( auto it = _timers.begin(); it != _timers.end(); it++ ) {
        if ( (quint64)QDateTime::currentMSecsSinceEpoch() - (quint64)it.value().toMSecsSinceEpoch() >= valid_interval ) {
            _last_update_times[ it.key() ] = QDateTime::currentDateTime();
            _asks.remove( it.key() );
            _bids.remove( it.key() );
            _outdated_counters[ it.key() ]++;
            if ( _outdated_counters[ it.key() ] > 1000 ) {
                qWarning() << it.key() << "Outdated counter warning: " << _outdated_counters[ it.key()];
            }
        }
    }
    QTimer::singleShot( 5650, this, &OrderBooksProxy::checkOutDated );
}

void OrderBooksProxy::reloadTargets() {
    return;
    if ( isDebug() ) {
        addDebug("Reloading targets for orderbooks");
    }
    _pairs.clear();
    _pairs_errs_counter.clear();
    _exchs_errs_counter.clear();
    _errors_counters.clear();
    QHash<quint32,QList<quint32>> _arbpairs( SwiftCore::getAssets(true)->getArbitragePairs() );
    for( auto it = _arbpairs.begin(); it != _arbpairs.end(); it++ ) {
        _pairs.append( it.value() );
    }
    QTimer::singleShot( 125000, this, &OrderBooksProxy::reloadTargets );
}

void OrderBooksProxy::sendRates() {
    if ( session != nullptr && session->isJoined() ) {
        const QString message( getReportFormattedMessage() );
        QVariant ret = session->call(RCP_TELEGRAM_NOTIFY, QVariantList({message}));
        Q_UNUSED( ret );
    }
}

void OrderBooksProxy::logRates() {
    const QList<quint32> _pairs( _current_rates.keys() );
    for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
        const quint32 pairid( *it );
        const double currate( _current_rates.value( pairid ) );
        if ( currate > 0 ) {

            if (  !_rates_history[ pairid ].isEmpty() ) {
                const double diff = ( currate - _rates_history[ pairid ].first() ) / _rates_history[ pairid ].first() * 100;
                if ( diff > 10 ) {

                    const QString arbname( assets->getArbitragePairName( assets->getMarketArbitragePairId( pairid ) ));
                    const QString exchname( assets->getMarketExchangeName( pairid ) );
                    const double rate = currate;
                    const QString message( "<b>" + arbname + ":</b> "+exchname+" \nRate changed to <i>"+QString::number( rate, 'f', 2 )+"</i> ("+QString::number( diff, 'f', 2 )+"%)\n" );
                    QVariant ret = session->call(RCP_TELEGRAM_NOTIFY, QVariantList({message}));
                    Q_UNUSED( ret );

                    // Notify send alert!
                    qWarning() << diff;
                }
                _rates_history[ pairid ].push_back( currate );
            }
        }

    }
}

double OrderBooksProxy::getRateDiffHour(const quint32 &pid) {
    if ( _rates_history_data.contains( pid ) && _current_rates.contains( pid ) ) {
        const quint64 fromtimestamp = QDateTime::currentSecsSinceEpoch() - 3600;
        QList<double> _valids;
        QMap<quint64,double> _left_values;
        QMap<quint64, double> _rates( _rates_history_data.value( pid ) );
        for( auto it = _rates.begin(); it != _rates.end(); it++ ) {
            if ( it.key() >= fromtimestamp ) {
                _valids.push_back( it.value() );
                _left_values.insert( it.key(), it.value() );
            }
        }
        _rates_history_data[ pid ] = _left_values;
        if ( !_valids.isEmpty() ) {
            return ( _valids.first() - _current_rates.value( pid ) ) / _current_rates.value( pid ) * 100;
        }


    }
    return 0;
}

QString OrderBooksProxy::getReportFormattedMessage() {
    QString msg("<u><b>Current rates</b></u>\n");

    SwiftBot::ArbitragePair::eachPair([&](SwiftBot::ArbitragePair pair ) {
        msg += "\n--\n";
        msg += QString("<b>"+pair.name+"</b>\n");
        pair.eachMarkets([&](SwiftBot::Market market) {
            if ( market.is_enabled ) {
                quint64 diff = QDateTime::currentSecsSinceEpoch() - _last_updates.value( market.id );
                double ratechange = getRateDiffHour( market.id );
                QString rstrtch = QString(ratechange >= 0.0 ? "+" : "")+QString::number( getRateDiffHour( market.id ), 'f', 2 )+"%";
                msg += QString("<u>"+QString::number( _current_rates.value( market.id ), 'f', 2)+"</u> - "
                               + market.exchange().name + " ("+rstrtch+") "
                               + QString(diff > 2 ? QString("(Updated : "+QString::number( QDateTime::currentSecsSinceEpoch() - _last_updates.value( market.id ))+" sec)") : "")+ "\n");
            }
        });
    });

    return msg;
}

double OrderBooksProxy::getArbitragePairRate(const quint32 &arbpair) {
    QList<quint32> _markets( assets->getArbitragePairs().value( arbpair ) );
    if ( _markets.isEmpty() ) {
        return 0.0;
    } else {
        QList<double> _act;
        for( auto it = _markets.begin(); it != _markets.end(); it++ ) {
            if ( _current_rates.contains( *it ) ) {
                _act.push_back( _current_rates.value( *it ) );
            }
        }
        if ( _act.isEmpty() ) {
            return 0.0;
        } else {
            return std::accumulate( _act.begin(), _act.end(), 0.0 ) / _act.count();
        }
    }
}

QJsonObject OrderBooksProxy::getSnapshot() const {
    QJsonObject j_r;
    QJsonArray j_a;
    const QHash<quint32, QMap<double,double>> _a( _asks );
    for( auto it = _a.begin(); it != _a.end(); it++ ) {
        const quint32 pid = it.key();
        const QMap<double,double> _itms( it.value() );
        for( auto iit = _itms.begin(); iit != _itms.end(); iit++ ) {
            j_a.push_back( QJsonArray({ QString::number( pid ), QString::number( iit.key(), 'f', 8), QString::number( iit.value(), 'f', 8)}));
        }
    }

    const QHash<quint32, QMap<double,double>> _b( _bids );
    QJsonArray j_b;
    for( auto it = _b.begin(); it != _b.end(); it++ ) {
        const quint32 pid = it.key();
        const QMap<double,double> _itms( it.value() );
        for( auto iit = _itms.begin(); iit != _itms.end(); iit++ ) {
            j_b.push_back( QJsonArray({ QString::number( pid ), QString::number( iit.key(), 'f', 8), QString::number( iit.value(), 'f', 8)}));
        }
    }

    j_r["asks"] = j_a;
    j_r["bids"] = j_b;
    return j_r;
}
