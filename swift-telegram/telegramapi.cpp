#include  "telegramapi.h"
#include <QCoreApplication>
TelegramApi::TelegramApi(QObject *parent) : QObject(parent), manager( new QNetworkAccessManager )
{
    connect( manager, &QNetworkAccessManager::finished, this, &TelegramApi::onReply );
    connect( this, &TelegramApi::sendMessage, this, &TelegramApi::onSendMessage );
}

void TelegramApi::onReply(QNetworkReply *reply) {
    const QByteArray dat( reply->readAll().constData() );
    reply->deleteLater();
}

void TelegramApi::onSendMessage(const QString &msg) {
    QUrl url( "https://api.telegram.org/bot"+SwiftCore::getModuleSettings("telegram")->value("telegram_api").toString()+"/sendMessage" );
    QUrlQuery query;
    query.addQueryItem("chat_id", SwiftCore::getModuleSettings("telegram")->value("chat_id").toString() );
    query.addQueryItem("text", msg );
    query.addQueryItem("parse_mode", "HTML");
    url.setQuery( query );
    QNetworkRequest request( url );
    QNetworkReply * rep = manager->get( request );
    Q_UNUSED(rep);
}
void TelegramApi::onWampSession(Wamp::Session *sess)
{
    session = sess;

    session->provide(RCP_TELEGRAM_NOTIFY, [=]( const QVariantList &v, const QVariantMap&m){
        Q_UNUSED(m);
        if ( v.isEmpty() ) {
        } else {
            emit sendMessage( v.at(0).toString() );
        }
        return 1;
    });

    sendMessage( "<b>Swift BOT</b> "+QCoreApplication::applicationVersion()+" - server started." );

}
