#include "wampclient.h"
#include "swiftbot.h"
#include "swiftcore.h"

WampClient::WampClient(const QString &realmname, const QString &servername, const quint16 &serverport, bool _debug, QObject *parent):
    QObject(parent),
    debug(_debug),
    started_at(QDateTime::currentDateTime()),
    sysinf_timer(new QTimer), p_realmname(realmname),
    p_servername(servername), p_serverport(serverport),session(nullptr), reconnect_limit(0),is_connected(false),is_starting(false)
{

    connect(&m_webSocket, &QTcpSocket::disconnected, this, &WampClient::closeConnections );
    connect(&m_webSocket, &QTcpSocket::aboutToClose, this, &WampClient::closeConnections );
    settings = SwiftBot::moduleSettings();
}

WampClient::WampClient(const QString &realmname) : QObject(nullptr),debug(0), p_realmname(realmname),  p_servername(""),p_serverport(0) ,is_connected(false) {

}

WampClient::WampClient(QObject *parent) : QObject(parent), debug(0), p_realmname(""),  p_servername("localhost"),p_serverport(8081) , session(nullptr), is_connected(false) {
    connect(&m_webSocket, &QTcpSocket::disconnected, this, &WampClient::closeConnections );
    connect(&m_webSocket, &QTcpSocket::aboutToClose, this, &WampClient::closeConnections );
}

bool WampClient::hasSession() const {
    if ( is_connected ) {
        return session != nullptr && session->isJoined();
    } else {
        return false;
    }
}

Wamp::Session *WampClient::getSession() const {
    return session;
}

bool WampClient::isConnected() const {
    return (session != nullptr && session->isJoined());
}

bool WampClient::isStarting() const
{
    return is_starting;
}

void WampClient::onClientConnected(onWampConnected func) {
    connect( this, &WampClient::clientConnected, func );
}

void WampClient::subscribeFeed(const QString &path, const QString &name){
    if ( is_connected && session != nullptr && session->isJoined() ) {
        session->subscribe( path, [=]( const QVariantList & v, const QVariantMap & m ) {
            Q_UNUSED( m )
            emit feedMessage( name, v );
        });
    }
}

void WampClient::callRpc(const QString &path, const QVariantList &args){
    if ( is_connected && session != nullptr && session->isJoined() ) {
        const QVariant re = session->call( path, args );
        emit rpcResult( re );
    }
}

void WampClient::publishFeed(const QString &path, const QVariantList &args){
    if ( is_connected && session != nullptr && session->isJoined() ) {
        session->publish( path, args );
    }
}

void WampClient::connectRealm(const QString &realmname) {
    p_realmname = realmname;
    if ( session != nullptr && session->isJoined() ) {
        closeConnections();
    } else {
        startClient();
    }
    emit systemText( "Connecting to realm: " + realmname );
}


void WampClient::publishMessage(const QString &topic, const QJsonObject &j) {
    if ( session != nullptr && session->isJoined() ) {
        const QString res( QJsonDocument( j ).toJson( QJsonDocument::Compact ) );
        session->publish( topic, QVariantList({res}));
    }
}
/*
void WampClient::publishMessage(const QString &topic, const QJsonArray &j) {
    if ( session != nullptr && session->isJoined() ) {
        const QString res( QJsonDocument( j ).toJson( QJsonDocument::Compact ) );
        if ( topic == FEED_EVENTS_ARBITRAGE ) {
            session->publish( topic, QVariantList({"STATS", res}));
        } else {
            session->publish( topic, QVariantList({res}));
        }

    }
}*/

void WampClient::startClient() {
    if ( is_connected || is_starting ) {
        return;
    }
    is_starting = true;
    if (debug) {
        qInfo() << "Starting WAMP client node :" << p_realmname << p_servername << p_serverport;
    }

    if ( p_realmname == "orderbooks" && p_servername == "" ) {
        if ( m_socket.isOpen() ) {
            m_socket.close();
        }
        if ( session != nullptr ) {
            session = nullptr;
        }
        session = new Wamp::Session( m_socket, Wamp::Session::MessageFormat::Msgpack, false);
        connect(&m_socket, &QLocalSocket::connected, this, &WampClient::onConnected);
        connect(&m_socket, &QLocalSocket::disconnected, this, &WampClient::closeConnections);
        connect( session, &Wamp::Session::left, this, &WampClient::onDisconnected );
        if (!is_connected) {
            m_socket.connectToServer("/opt/swiftbot/socket1",QLocalSocket::ReadWrite);
        }
    } else {

        if ( m_webSocket.isOpen() ) {
            m_webSocket.close();

        }
        if ( session != nullptr ) {
            disconnect(&m_webSocket, &QTcpSocket::connected, this, &WampClient::onConnected);
            disconnect( this, &WampClient::clientJoined, this, &WampClient::onJoined );
            //disconnect( session, &Wamp::Session::left, this, &WampClient::onDisconnected );
        }

        session = new Wamp::Session( m_webSocket, Wamp::Session::MessageFormat::Msgpack, false);
        connect(&m_webSocket, &QTcpSocket::connected, this, &WampClient::onConnected);
        connect( this, &WampClient::clientJoined, this, &WampClient::onJoined );
        connect( session, &Wamp::Session::left, this, &WampClient::onDisconnected );
        if (!is_connected) {
            m_webSocket.connectToHost( p_servername , p_serverport);
        }
    }
}

void WampClient::onJoined() {
    // const QString async_topic( SwiftLib::getTopic( SwiftLib::ApiRpcAsyncResults ) );
}

void WampClient::onConnected() {
    is_connected=true;
    is_starting = false;
    reconnect_limit = 0;
    started_at = QDateTime::currentDateTime();

    if (debug ) {
        qDebug() << "WAMP connected to " << p_servername << p_serverport;
    }

    QObject::connect(session, &Wamp::Session::joined, [&](qint64 s) {

        if (debug ) {
            qWarning() << "Session joined to realm" << p_realmname << "with session ID " << s;
        }
        sessionname = QString::number( s );
        is_connected=true;

        if ( !_provided_queue.isEmpty() ) {
            while ( !_provided_queue.isEmpty() ) {
                QPair<QString,  Wamp::Endpoint::Function> next = _provided_queue.dequeue();
                session->provide( next.first, next.second );
            }
        }

        if ( !_subsd_queue.isEmpty() ) {
            while ( !_subsd_queue.isEmpty() ) {
                QPair<QString, wampFunction2> next = _subsd_queue.dequeue();
                session->subscribe( next.first, next.second );
            }
        }

        if ( !_published_queue.isEmpty() ) {
            while ( !_published_queue.isEmpty() ) {
                QPair<QString, QVariantList> next = _published_queue.dequeue();
                session->publish( next.first, next.second );
            }
        }

        emit clientJoined();
        emit clientConnected( session );
     });

    QObject::connect(session, &Wamp::Session::challenge,[&](const QString &realm, const QString &authid, const Wamp::Challenge &credentials ){
        Q_UNUSED(realm)
        Q_UNUSED(authid)
        Q_UNUSED(credentials)
        session->authenticate( settings->value( SETTINGS_NAME_WAMP_PASS ).toString() );
    });

    // При начале сессии
    connect(session, &Wamp::Session::started, [&]() {
        if (debug ) {
            qWarning() << "Session started on realm" << p_realmname;
        }

        session->join(p_realmname, settings->value( SETTINGS_NAME_WAMP_USER ).toString(), QStringList({"ticket"}));
    });

    // При завершении сессии инциируем отключение
    connect( session, &Wamp::Session::left, [&] ( const QString& reason ){
        if (debug ) {
            qWarning() << "Session left: " << reason ;
        }
        qWarning() << reason;
        emit clientdiconnected();
    });

    session->start();


}

void WampClient::onDisconnected( const QString& realm ) {
    is_starting = false;
    if ( is_connected ) {
        is_connected=false;
        qWarning() << "WAMP client disconnected." << "Realm/Reason" << realm;
        const quint32 waitsecs = 2;
        if ( 20 >= reconnect_limit ) {
            reconnect_limit++;
            qWarning() << "Reconnecting after" << waitsecs << "seconds";
            if ( !is_connected ) {
                QTimer::singleShot( waitsecs * 1000, this, &WampClient::startClient );
            }
        } else {
            qWarning() << "Maxmum connections attemps limit";
            emit isExited();
        }
    }

}

void WampClient::closeConnections() {
    is_starting = false;
    if ( is_connected ) {
        if ( session != nullptr ) {
            if ( session->isJoined() ) {
                session->leave();
            }
        }
        session->deleteLater();
        qWarning() << "WAMP CLOSED!";
        onDisconnected(p_realmname);
    }
}
