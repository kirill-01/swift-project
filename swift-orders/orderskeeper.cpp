#include "orderskeeper.h"

#include <QList>

class OrdersKeeperData : public QSharedData
{
public:
    SwiftBot::Orders orders;
};

OrdersKeeper::OrdersKeeper(QObject *parent) : QObject(parent), is_pause(false),is_debug( SwiftBot::moduleParam("is_debug", false ).toBool() ),
    update_history_timer( new QTimer(this) ),
    update_active_timer( new QTimer(this) ),

    data(new OrdersKeeperData),

    session(nullptr)

{
    connect( this, &OrdersKeeper::activeEvent, this, &OrdersKeeper::onActiveEvent );
    connect( this, &OrdersKeeper::orderEvent, this, &OrdersKeeper::onOrderEvent );
    connect( this, &OrdersKeeper::historyEvent, this, &OrdersKeeper::onHistoryEvent );

    connect( this, &OrdersKeeper::callTelegtamStats, this, &OrdersKeeper::sendStatsMsg );
    connect( this, &OrdersKeeper::callTelegtamTrades, this, &OrdersKeeper::sendTradesMsg );
    connect( this, &OrdersKeeper::callTelegtamBalances, this, &OrdersKeeper::sendBalancesMsg );

    connect( this, &OrdersKeeper::closeAll, this, &OrdersKeeper::onCloseAll );

    QSettings * settings( SwiftBot::moduleSettings() );
    const quint32 history_interval = settings->value("history_update_interval", 360000).toUInt();
    if ( history_interval > 0 ) {
        update_history_timer->setInterval( history_interval );
        QObject::connect( update_history_timer, &QTimer::timeout, this, &OrdersKeeper::requestHistory );

    }
    const quint32 active_interval = settings->value("active_update_interval", 30000).toUInt();
    if ( active_interval > 0 ) {
        update_active_timer->setInterval( active_interval );
        QObject::connect( update_active_timer, &QTimer::timeout, this, &OrdersKeeper::requestActive );
    }

    QTimer::singleShot( 25000, this, &OrdersKeeper::requestHistory );

    QSqlQuery q("SELECT * FROM orders");
    if ( q.exec() ) {
        while ( q.next() ) {
            data->orders.insert( q.value("id").toUInt(), q.record() );
        }
    }
}

OrdersKeeper::OrdersKeeper(const OrdersKeeper &rhs) : QObject(nullptr), data(rhs.data)
{

}

OrdersKeeper &OrdersKeeper::operator=(const OrdersKeeper &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

OrdersKeeper::~OrdersKeeper()
{

}

void OrdersKeeper::onActiveEvent(const QJsonObject &j_data)
{
    if ( is_debug ) {
        qWarning() << "Orders active event: " << j_data;
    }
    const QJsonArray items( j_data.value("items").toArray() );
    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            const quint64 locuid( it->toObject().value("local_id").toString().toUInt() );
            if ( locuid == 0 ) {
                SwiftBot::Order order( it->toObject( ) );
                order.update();
                data->orders.insert( order.local_id, order );
            } else {
                if ( !data->orders.contains( locuid ) ){
                    data->orders.insert( it->toObject().value("local_id").toString().toUInt(),
                        SwiftBot::Order( it->toObject().value("local_id").toString().toUInt() )
                        );
                }
                data->orders[ locuid ].update( it->toObject() );
            }
        }
    }
}

void OrdersKeeper::onHistoryEvent(const QJsonObject &j_data)
{
    if ( is_debug ) {
        qWarning() << "Orders history event: " << j_data;
    }
    const QJsonArray items( j_data.value("items").toArray() );

    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            const QJsonObject j_order_params( it->toObject() );
            if ( !j_order_params.keys().contains("local_id") ) {
                SwiftBot::Order order( j_order_params );
                order.update();
                data->orders.insert( order.local_id, order );
            }
            const quint64 locuid( it->toObject().value("local_id").toString().toUInt() );
            if ( locuid == 0 ) {
                SwiftBot::Order order( it->toObject( ) );
                order.update();
                data->orders.insert( order.local_id, order );
            } else {
                if ( !data->orders.contains( locuid ) ){
                    data->orders.insert( it->toObject().value("local_id").toString().toUInt(),
                        SwiftBot::Order( it->toObject().value("local_id").toString().toUInt() )
                        );
                }
                data->orders[ locuid ].update( it->toObject() );
            }
        }
    }
}

void OrdersKeeper::onOrderEvent(const QString& event_name, const QJsonObject &j_data )
{
    if ( is_debug ) {
        qWarning() << "Order event: " << j_data;
    }
    const QJsonObject j_order_object( j_data );
    const quint64 locuid( j_order_object.value("local_id").toString().toUInt() );
    // Events actions here ?
    if ( event_name == EVENTS_NAME_ORDER_PLACED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_COMPLETED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_UPDATED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_ERROR ) {

    }
    data->orders[ locuid ].update( j_order_object );
}

void OrdersKeeper::onCloseAll() {
    SwiftBot::Orders _orders = data->orders;
    if ( is_debug ) {
        qWarning() << "Canceling all available orders";
    }
    for( auto it = _orders.begin(); it != _orders.end(); it++ ) {
        if ( it->status <= 1 ) {
            it->cancel( session );
        }
    }
}

void OrdersKeeper::requestHistory() {
    if ( is_pause ) {
        return;
    }
    if ( _active_clients.isEmpty() ) {
        getConnectedExchanges();
    } else {
        for( auto it = _active_clients.begin(); it != _active_clients.end(); it++ ) {
            SwiftBot::Exchange exchange( *it );
            if ( exchange.isRequstsSeparated() ) {
                exchange.forEachMarket([=](SwiftBot::Market m){
                    QJsonObject j_params({{"market_id",QString::number( m.id ) }});
                    const QString str( QJsonDocument(j_params).toJson( QJsonDocument::Compact ) );
                    const QString methodpath( "swift.api.trade.history."+exchange.name );
                    quint64 async_uid = session->call( methodpath, { str } ).toULongLong();
                    if ( async_uid <= 10 ) {
                        qWarning() << "Error requesting orders history" << j_params << methodpath;
                    }
                });
            } else {
                const QString methodpath( "swift.api.trade.history."+exchange.name );
                quint64 async_uid = session->call( methodpath, {}).toULongLong();
                if ( async_uid <= 10 ) {
                    qWarning() << "Error requesting orders history" << methodpath;
                }
            }
        }
    }
}

void OrdersKeeper::requestActive() {
    if ( is_pause ) {
        return;
    }
    if ( _active_clients.isEmpty() ) {
        getConnectedExchanges();
    } else {
        for( auto it = _active_clients.begin(); it != _active_clients.end(); it++ ) {
            SwiftBot::Exchange exchange( *it );
            if ( exchange.isRequstsSeparated() ) {
                exchange.forEachMarket([=](SwiftBot::Market m){
                    QJsonObject j_params({{"market_id",QString::number( m.id ) }});
                    const QString str( QJsonDocument(j_params).toJson( QJsonDocument::Compact ) );
                    const QString methodpath( "swift.api.trade.active."+exchange.name );
                    quint64 async_uid = session->call( methodpath, {str} ).toULongLong();
                    if ( async_uid <= 10 ) {
                        qWarning() << "Error requesting active orders" << async_uid << j_params << methodpath;
                    }
                });
            } else {
                const QString methodpath( "swift.api.trade.active."+exchange.name );
                quint64 async_uid = session->call( methodpath, {}).toULongLong();
                if ( async_uid <= 10 ) {
                    qWarning() << "Error requesting active orders" << async_uid << methodpath;
                }
            }
        }
    }
}

void OrdersKeeper::recheck_clients() {

    getConnectedExchanges();

    if ( _active_clients.isEmpty() ) {
        QTimer::singleShot( 120000, this, &OrdersKeeper::recheck_clients );
    } else {
        update_active_timer->start();
        update_history_timer->start();
        is_pause = false;
    }
}

void OrdersKeeper::onWampSession(Wamp::Session *sess) {
    session = sess;

    // Events listener
    session->subscribe( FEED_EVENTS_ORDERS, [=]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m)
        const QString event_name( v.at(0).toString() );
        const QJsonObject j_data( QJsonDocument::fromJson( v.at(1).toString().toUtf8() ).object() );
        if ( event_name == EVENTS_NAME_ORDERS_ACTIVE ) {
            emit activeEvent( j_data );
        } else if ( event_name == EVENTS_NAME_ORDERS_HISTORY ) {
            emit historyEvent ( j_data );
        } else {
            emit orderEvent( event_name, j_data );
        }
    });

    session->provide( "swift.orders.active", [=]( const QVariantList&v, const QVariantMap& m ) {
       Q_UNUSED(m)
       Q_UNUSED(v)
       QJsonArray j_ret;
       SwiftBot::Orders _orders = data->orders;
       for( auto it = _orders.begin(); it != _orders.end(); it++ ) {
           if ( it->status < 2 ) {
               j_ret.push_back( it->toJson() );
           }
       }
       const QString str_ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
       return str_ret;
    });

    session->provide( "swift.orders.history", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        QJsonArray j_ret;
        SwiftBot::Orders _orders = data->orders;
        for( auto it = _orders.begin(); it != _orders.end(); it++ ) {
            if ( it->status >= 2 ) {
                j_ret.push_back( it->toJson() );
            }
        }
        const QString str_ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
        return str_ret;
    });

    session->provide( "swift.orders.closeall", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        emit closeAll();
        return 1;
    });

    session->provide( "swift.orders.today", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        QJsonArray j_ret;
        const QString str_ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
        return str_ret;
    });

    session->provide( "swift.telegram.cmd.balances", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        emit callTelegtamBalances();
        return 1;
    });

    session->provide( "swift.telegram.cmd.trades", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        emit callTelegtamTrades();
        return 1;
    });

    session->provide( "swift.telegram.cmd.stats", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        emit callTelegtamStats();
        return 1;
    });

    session->provide( "swift.telegram.callback", [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        const QJsonObject j_v( v.at(0).toJsonObject() );
        const QJsonObject j_msg( j_v.value("message").toObject() );
        const QString cmd = j_msg.value("text").toString();
        if ( cmd == "/bals" ) {
            emit callTelegtamBalances();
        } else if ( cmd == "getAppBalances" ) {
            const QString bals = session->call(RPC_BALANCE_ALL).toString();
            return bals;
        } else if ( cmd == "getAppOrders" ) {
            QJsonObject j_r;
            QJsonArray j_act;
            for ( auto it = data->orders.begin(); it != data->orders.end(); it++ ) {
                j_act.push_back( it->toJson() );
            }
            j_r["items"] = j_act;
            const QString str_ret( QJsonDocument( j_r ).toJson( QJsonDocument::Compact ) );
            return str_ret;
        } else if ( cmd == "getAppArbitrage" ) {
            QJsonObject j_r;
            QJsonArray j_a_p;
            QSqlQuery q("SELECT arbitrage_pair_id, count(*) as cnt, sell_pair_id, buy_pair_id, AVG(min_amount) as min_am, AVG(max_amount) as max_am, AVG(min_sell_rate) as min_sr, AVG(max_sell_rate) as max_sr, AVG(min_buy_rate) as min_br, AVG(max_buy_rate) as max_br FROM arbitrage_events WHERE ts >= date_sub(NOW(), interval 48 hour) GROUP by arbitrage_pair_id, sell_pair_id, buy_pair_id ORDER by cnt desc");
            if ( q.exec() ) {
                while ( q.next() ) {
                    QJsonObject ji;
                    ji["apid"] = QString::number( q.value("arbitrage_pair_id").toUInt() );
                    ji["apiname"] = SwiftCore::getAssets()->getArbitragePairName( q.value("arbitrage_pair_id").toUInt() );
                    ji["spid"] = QString::number( q.value("sell_pair_id").toUInt() );
                    ji["bpid"] = QString::number( q.value("buy_pair_id").toUInt() );
                    ji["cnt"] = QString::number( q.value("cnt").toUInt() );
                    ji["sell_exch"] = SwiftCore::getAssets()->getMarketExchangeName( q.value("sell_pair_id").toUInt() );
                    ji["buy_exch"] = SwiftCore::getAssets()->getMarketExchangeName( q.value("buy_pair_id").toUInt() );
                    ji["market_name"] = SwiftCore::getAssets()->getMarketName( q.value("sell_pair_id").toUInt() );
                    ji["min_am"] = QString::number( q.value("min_am").toDouble(), 'f', 8 );
                    ji["max_am"] = QString::number( q.value("max_am").toDouble(), 'f', 8 );
                    ji["min_sr"] = QString::number( q.value("min_sr").toDouble(), 'f', 8 );
                    ji["max_sr"] = QString::number( q.value("max_sr").toDouble(), 'f', 8 );
                    ji["min_br"] = QString::number( q.value("min_br").toDouble(), 'f', 8 );
                    ji["max_br"] = QString::number( q.value("max_br").toDouble(), 'f', 8 );
                    j_a_p.push_back( ji );
                }
            }
            j_r["items"] = j_a_p;
            const QString str_ret( QJsonDocument( j_r ).toJson( QJsonDocument::Compact ) );
            return str_ret;
        } else if ( cmd == "getAppUserSummary" ) {
            QJsonObject j_r;
            QJsonArray bals_history;
            QJsonObject trade_fees;
            QSqlQuery q("SELECT DATE(ts) as dt, AVG(total) as ttl, AVG(profit) as profit FROM balance_history GROUP BY DATE(ts) ORDER BY DATE(ts) ASC");
            if ( q.exec() ) {
                while( q.next() ) {
                    bals_history.push_back( QJsonArray({ q.value("dt").toDate().toString(), QString::number( q.value("ttl").toDouble(), 'f', 8 ), QString::number( q.value("profit").toDouble(), 'f', 8 ) }));
                }
                j_r["bh"] = bals_history;
            }
            if ( q.exec("SELECT * FROM exchanges WHERE is_enabled=1") ) {
                while( q.next() ) {
                    trade_fees[ q.value("name").toString() ] = QString::number( q.value("trade_fee").toDouble(), 'f', 4 );
                }
                j_r["tfees"] = trade_fees;
            }
            const QString str_ret( QJsonDocument( j_r ).toJson( QJsonDocument::Compact ) );
            return str_ret;
        }
        return QString("{}");
    });


    update_history_timer->start();
    update_active_timer->start();



}

void OrdersKeeper::sendStatsMsg() {
    QString msg("\n<u>Statistics</u>\n\n");
    if ( session != nullptr && session->isJoined() ) {
        session->call( "swift.telegram.msg", {msg} );
    }
}

void OrdersKeeper::sendBalancesMsg() {
    if ( session != nullptr && session->isJoined() ) {
        session->call( "swift.balance.sendinfo" );
    }
}

void OrdersKeeper::sendTradesMsg() {
    QString msg("\n<u>Trades</u>\n\n");
    if ( session != nullptr && session->isJoined() ) {
        session->call( "swift.telegram.msg", {msg} );
    }
}

void OrdersKeeper::pauseModule() {
    if ( !is_pause ) {
        update_history_timer->stop();
        update_active_timer->stop();
        if ( session != nullptr && session->isJoined() ) {
            QString msg("<u><b>No accounts data</b></u>\nInformation about orders unavailable now.\n<u>Suspending orders module.</u>");
            session->call( RCP_TELEGRAM_NOTIFY, {msg} );
        }
        is_pause = true;
        qWarning() << "ORDERS: Pausing module!";
        QTimer::singleShot( 120000, this, &OrdersKeeper::recheck_clients );
    }
}

void OrdersKeeper::getConnectedExchanges()
{
    QMutexLocker lock( &m );
    static quint64 last_check_clients = 0;
    if ( ( !is_pause && _active_clients.isEmpty() ) || QDateTime::currentSecsSinceEpoch() - last_check_clients >= 180 ) {
        if ( session != nullptr && session->isJoined() ) {

            last_check_clients = QDateTime::currentSecsSinceEpoch();

            const QString _targetsstr = session->call( RPC_EXCHANGES_LIST_COMMAND ).toString();
            const QStringList exchsList( _targetsstr.split(",") );
            _active_clients.clear();
            if ( !exchsList.isEmpty() ) {
                for( auto it = exchsList.begin(); it!= exchsList.end(); it++ ) {
                    const QString path_( "swift.api.status."+*it );
                    QVariant res = session->call( path_ );
                    const QString res_str( res.toString() );
                    if ( !res_str.isEmpty() ) {
                        const QJsonObject j_api_state( QJsonDocument::fromJson( res_str.toUtf8() ).object() );
                        if ( j_api_state.value("public_methods").toBool( false ) &&
                             j_api_state.value("private_methods").toBool( false ) ) {
                            _active_clients.push_back( *it );
                        }
                    }
                }
            }
            if ( !is_pause && _active_clients.isEmpty() ) {
                pauseModule();
            }
        }
    }
}
