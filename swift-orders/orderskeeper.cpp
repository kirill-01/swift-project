#include "orderskeeper.h"

#include <QList>

class OrdersKeeperData : public QSharedData
{
public:
    SwiftBot::Orders orders;
};

OrdersKeeper::OrdersKeeper(QObject *parent) : QObject(parent), is_pause(false),
    update_history_timer( new QTimer(this) ),
    update_active_timer( new QTimer(this) ),

    data(new OrdersKeeperData),

    session(nullptr)

{
    connect( this, &OrdersKeeper::activeEvent, this, &OrdersKeeper::onActiveEvent );
    connect( this, &OrdersKeeper::orderEvent, this, &OrdersKeeper::onOrderEvent );
    connect( this, &OrdersKeeper::historyEvent, this, &OrdersKeeper::onHistoryEvent );

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
    const QJsonArray items( j_data.value("orders").toArray() );
    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            const quint64 locuid( it->toObject().value("local_id").toString().toUInt() );
            if ( locuid == 0 ) {
                SwiftBot::Order order( SwiftBot::Order::create(
                                           it->toObject().value("market_id").toString().toUInt(),
                                           it->toObject().value("amount").toString().toDouble(),
                                           it->toObject().value("rate").toString().toDouble(),
                                           it->toObject().value("type").toString() == "sell" ? 0 : 1
                                           ));
                order.update( it->toObject( ) );
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
    const QJsonArray items( j_data.value("orders").toArray() );

    if ( !items.isEmpty() ) {
        for( auto it = items.begin(); it != items.end(); it++ ) {
            const quint64 locuid( it->toObject().value("local_id").toString().toUInt() );
            if ( locuid == 0 ) {
                SwiftBot::Order order( SwiftBot::Order::create(
                                           it->toObject().value("market_id").toString().toUInt(),
                                           it->toObject().value("amount").toString().toDouble(),
                                           it->toObject().value("rate").toString().toDouble(),
                                           it->toObject().value("type").toString() == "sell" ? 0 : 1
                                           ));
                order.update( it->toObject( ) );
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
    const QJsonObject j_order_object( j_data );
    const quint64 locuid( j_order_object.value("local_id").toString().toUInt() );
    if ( event_name == EVENTS_NAME_ORDER_PLACED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_COMPLETED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_UPDATED ) {

    } else if ( event_name == EVENTS_NAME_ORDER_ERROR ) {

    }
    data->orders[ locuid ].update( j_order_object );
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
                exchange.forEachMarket([&exchange](SwiftBot::Market m){
                    QJsonObject j_params({{"market_id",QString::number( m.id ) }});
                    const QString str( QJsonDocument(j_params).toJson( QJsonDocument::Compact ) );
                    const QString methodpath( "swift.api.trade.history."+exchange.name );
                    quint64 async_uid = SwiftBot::method( methodpath, { str } ).toULongLong();
                    if ( async_uid <= 10 ) {
                        qWarning() << "Error requesting orders history" << j_params << methodpath;
                    }
                });
            } else {
                const QString methodpath( "swift.api.trade.history."+exchange.name );
                quint64 async_uid = SwiftBot::method( methodpath, {}).toULongLong();
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
                exchange.forEachMarket([&exchange](SwiftBot::Market m){
                    QJsonObject j_params({{"market_id",QString::number( m.id ) }});
                    const QString str( QJsonDocument(j_params).toJson( QJsonDocument::Compact ) );
                    const QString methodpath( "swift.api.trade.active."+exchange.name );
                    quint64 async_uid = SwiftBot::method( methodpath, { str } ).toULongLong();
                    if ( async_uid <= 10 ) {
                        qWarning() << "Error requesting active orders" << async_uid << j_params << methodpath;
                    }
                });
            } else {
                const QString methodpath( "swift.api.trade.active."+exchange.name );
                quint64 async_uid = SwiftBot::method( methodpath, {}).toULongLong();
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

    update_history_timer->start();
    update_active_timer->start();

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

            const QString _targetsstr = SwiftBot::method( RPC_EXCHANGES_LIST_COMMAND, {} ).toString();
            const QStringList exchsList( _targetsstr.split(",") );

            _active_clients.clear();

            if ( !exchsList.isEmpty() ) {
                for( auto it = exchsList.begin(); it!= exchsList.end(); it++ ) {
                    QVariant res = session->call("swift.api.status."+*it);
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
