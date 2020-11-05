#include "wampauth.h"
#include "swiftcore.h"
#include <swiftbot.h>
#include <QTimer>
#include <QMessageAuthenticationCode>
WampAuth::WampAuth(const QString &realmname, const QString &servername, const quint16 &serverport, bool _debug, QObject *parent):
    QObject(parent),
    debug(_debug),
    started_at(QDateTime::currentDateTime()),
    p_realmname(realmname),
    p_servername(servername), p_serverport(serverport),session(nullptr), reconnect_limit(0),is_connected(false)
{
    settings = SwiftBot::moduleSettings();
    connect(&m_webSocket, &QTcpSocket::disconnected, this, &WampAuth::closeConnections );
    connect(&m_webSocket, &QTcpSocket::aboutToClose, this, &WampAuth::closeConnections );


}

WampAuth::WampAuth(const QString &realmname) : QObject(nullptr),debug(0), p_realmname(realmname),  p_servername(""),p_serverport(0) ,is_connected(false) {

}

WampAuth::WampAuth(QObject *parent) : QObject(parent), debug(0), p_realmname(""),  p_servername("localhost"),p_serverport(8081) , session(nullptr), is_connected(false) {
    connect(&m_webSocket, &QTcpSocket::disconnected, this, &WampAuth::closeConnections );
    connect(&m_webSocket, &QTcpSocket::aboutToClose, this, &WampAuth::closeConnections );
}

Wamp::Session *WampAuth::getSession() const {
    return session;
}

void WampAuth::connectRealm(const QString &realmname) {
    p_realmname = realmname;
    if ( session != nullptr && session->isJoined() ) {
        closeConnections();
    } else {
        startClient();
    }
    emit systemText( "Connecting to realm: " + realmname );
}


void WampAuth::publishMessage(const QString &topic, const QJsonObject &j) {
    if ( session != nullptr && session->isJoined() ) {
        const QString res( QJsonDocument( j ).toJson( QJsonDocument::Compact ) );
        session->publish( topic, QVariantList({res}));
    }
}


void WampAuth::startClient() {
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
        connect(&m_socket, &QLocalSocket::connected, this, &WampAuth::onConnected);
        connect(&m_socket, &QLocalSocket::disconnected, this, &WampAuth::closeConnections);
        connect( session, &Wamp::Session::left, this, &WampAuth::onDisconnected );
        if (!is_connected) {
            m_socket.connectToServer("/opt/swiftbot/socket1",QLocalSocket::ReadWrite);
        }
    } else {

        if ( m_webSocket.isOpen() ) {
            m_webSocket.close();

        }
        if ( session != nullptr ) {
            disconnect(&m_webSocket, &QTcpSocket::connected, this, &WampAuth::onConnected);
            disconnect( this, &WampAuth::clientJoined, this, &WampAuth::onJoined );
            //disconnect( session, &Wamp::Session::left, this, &WampAuth::onDisconnected );
        }

        session = new Wamp::Session( m_webSocket, Wamp::Session::MessageFormat::Msgpack, false);
        connect(&m_webSocket, &QTcpSocket::connected, this, &WampAuth::onConnected);
        connect( this, &WampAuth::clientJoined, this, &WampAuth::onJoined );
        connect( session, &Wamp::Session::left, this, &WampAuth::onDisconnected );
        if (!is_connected) {
            m_webSocket.connectToHost( p_servername , p_serverport);
        }
    }
}

void WampAuth::onJoined() {
    // const QString async_topic( SwiftLib::getTopic( SwiftLib::ApiRpcAsyncResults ) );
}

void WampAuth::onConnected() {
    is_connected=true;
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

        emit clientJoined();
        emit clientConnected( session );
     });

    QObject::connect(session, &Wamp::Session::challenge,[&](const QString &realm, const QString &authid, const Wamp::Challenge &credentials ){
        Q_UNUSED(credentials)
        session->authenticate(  "secret123" );
        qWarning() << realm << authid;
    });

    // При начале сессии
    connect(session, &Wamp::Session::started, [&]() {
        if (debug ) {
            qWarning() << "Session started on realm" << p_realmname;
        }
        session->join(p_realmname, "authenticator1", QStringList({"wampcra"}));
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

void WampAuth::onDisconnected( const QString& realm ) {
    if ( is_connected ) {
        is_connected=false;
        qWarning() << "WAMP client disconnected." << "Realm/Reason" << realm;
        const quint32 waitsecs = 2;
        if ( 20 >= reconnect_limit ) {
            reconnect_limit++;
            qWarning() << "Reconnecting after" << waitsecs << "seconds";
            if ( !is_connected ) {
                QTimer::singleShot( waitsecs * 1000, this, &WampAuth::startClient );
            }
        } else {
            qWarning() << "Maxmum connections attemps limit";
            emit isExited();
        }
    }

}

void WampAuth::closeConnections() {
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
