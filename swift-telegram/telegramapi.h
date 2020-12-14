#ifndef TELEGRAMAPI_H
#define TELEGRAMAPI_H

#include <QObject>


#include "../swift-corelib/swiftcore.h"
#include <wampclient.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

class TelegramApi : public QObject
{
    Q_OBJECT
public:
    explicit TelegramApi(QObject *parent = nullptr);
signals:
    void sendMessage( const QString& msg );
public slots:
    void beforeShutdown(){
        emit sendMessage( QString("Server going down..") );
    }
    void onReply( QNetworkReply * reply );
    void onSendMessage( const QString & msg );
    void onWampSession( Wamp::Session * sess );
private:
    QNetworkAccessManager * manager;
    Wamp::Session * session;

};

#endif // TELEGRAMAPI_H
