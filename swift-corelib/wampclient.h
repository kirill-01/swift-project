#ifndef WAMPCLIENT_H
#define WAMPCLIENT_H
#include "wamp.h"
#include <QMutex>
#include <QTimer>
#include <QLocalSocket>
#include <QSettings>
#include <QQueue>

typedef std::function<void( Wamp::Session * )> onWampConnected;
typedef std::function<QVariant( const QVariantList&v, const QVariantMap&m )> wampFunction;
typedef std::function<void(const QVariantList&, const QVariantMap&)> wampFunction2;

class WampClient : public QObject
{
    Q_OBJECT
public:
    WampClient(const QString & realmname, const QString & servername, const quint16& serverport, bool debug=false, QObject *parent = nullptr);
    WampClient( const QString& realmname );
    explicit WampClient( QObject * parent = nullptr );
    bool hasSession() const;
    Wamp::Session * getSession() const;
    void setOrderbooksSubscriber();
    void subscribeSignl( const QString& topic );
    bool isConnected() const;
    bool isStarting() const;
    void onClientConnected(onWampConnected func );
    QQueue<QPair<QString, Wamp::Endpoint::Function>> _provided_queue;
    void provide( const QString& method,  Wamp::Endpoint::Function func ) {
        if ( getSession() == nullptr || !getSession()->isJoined() ) {
            _provided_queue.enqueue( QPair<QString,  Wamp::Endpoint::Function>({method, func }) );
        } else {
            getSession()->provide( method, func );
        }
    }
    QQueue<QPair<QString, wampFunction2>> _subsd_queue;
    void subscribe( const QString& method, wampFunction2 func ) {
        if ( getSession() == nullptr || !getSession()->isJoined() ) {
            _subsd_queue.enqueue( QPair<QString, wampFunction2>({method, func }) );
        } else {
            getSession()->subscribe( method, func );
        }
    }
    QQueue<QPair<QString, QVariantList>> _published_queue;
    void publish( const QString& method, QVariantList func ) {
        if ( getSession() == nullptr || !getSession()->isJoined() ) {
            _published_queue.enqueue( QPair<QString, QVariantList>({method, func }) );
        } else {
            getSession()->publish( method, func );
        }
    }

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
    void subscribeFeed( const QString& path, const QString& name );
    void callRpc( const QString& path, const QVariantList & args = QVariantList() );
    void publishFeed( const QString& path, const QVariantList & args );

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
    bool is_starting;
    QMutex mutex;
    QSettings * settings;
};

#endif // WAMPCLIENT_H
