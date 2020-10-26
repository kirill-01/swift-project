#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H
#include "wamp.h"
#include <QMutex>
#include <QTimer>
#include <QLocalSocket>
#include <QSettings>


class WampClient : public QObject
{
    Q_OBJECT
public:
    WampClient(const QString & realmname, const QString & servername, const quint16& serverport, bool debug=false, QObject *parent = nullptr);
    WampClient( const QString& realmname );
    explicit WampClient( QObject * parent = nullptr );
    bool hasSession() const {
        if ( is_connected ) {
            return session != nullptr && session->isJoined();
        } else {
            return false;
        }
    }
    Wamp::Session * getSession() const;
    void setOrderbooksSubscriber();
    void subscribe( const QString& topic );
signals:
    void clientStarted();
    void clientConnected( Wamp::Session * session );
    void clientdiconnected();
    void clientJoined();
    void asyncResult( const QJsonObject& result );
    void isExited();
    void orderbooksDataReceived( const QJsonObject& jdata );
    void messageReceived( const QString& topic, const QString& message );
    void systemText( const QString& text );

    void rpcResult( const QVariant& vars );
    void feedMessage( const QString& feed, const QVariantList & args );

public slots:
    void subscribeFeed( const QString& path, const QString& name ){
        if ( is_connected && session != nullptr && session->isJoined() ) {
            session->subscribe( path, [=]( const QVariantList & v, const QVariantMap & m ) {
                Q_UNUSED( m )
                emit feedMessage( name, v );
            });
        }
    }
    void callRpc( const QString& path, const QVariantList & args = QVariantList() ){
        if ( is_connected && session != nullptr && session->isJoined() ) {
            const QVariant re = session->call( path, args );
            emit rpcResult( re );
        }
    }
    void publishFeed( const QString& path, const QVariantList & args ){
        if ( is_connected && session != nullptr && session->isJoined() ) {
            session->publish( path, args );
        }
    }

    void connectRealm( const QString& realmname );
    void publishMessage( const QString& topic, const QJsonObject& j );
    //void publishMessage( const QString& topic, const QJsonArray& j );
    void startClient();
    void onJoined();
    void onConnected();
    void onDisconnected( const QString& realm );

    void closeConnections();
private:
    bool debug;
    QDateTime started_at;
    QTimer * sysinf_timer;
    QString p_realmname;
    QString p_servername;

    quint16 p_serverport;
    QLocalSocket m_socket;
    QTcpSocket m_webSocket;
    Wamp::Session *session;
    quint16 reconnect_limit;
    QString sessionname;
    bool is_connected;
    QMutex mutex;
    QSettings * settings;
};

#endif // WAMPCLIENT_H
