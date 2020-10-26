#include "swiftapiclientlivecoin.h"

SwiftApiClientLivecoin::SwiftApiClientLivecoin(QObject *parent) : SwiftApiClient(parent)
{
}

QString SwiftApiClientLivecoin::getExchangeName() const
{
    return "livecoin";
}

QString SwiftApiClientLivecoin::getApiVersionString()
{
    return "1.0.0";
}

void SwiftApiClientLivecoin::getCurrencies(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.livecoin.net/info/coinInfo" );
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::getMarkets(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.livecoin.net/exchange/ticker" );
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::getOrderbooks(const QJsonObject &j_params, const quint64 &async_uuid)
{
    const bool cachingorderbooks = getSettings()->value( SETTINGS_NAME_EXCHANGE_CACHE_ORDERBOOKS, false ).toBool();
    if ( cachingorderbooks ) {
        const quint32 cachingorderbooks_time = getSettings()->value( SETTINGS_NAME_EXCHANGE_CACHE_ORDERBOOKS_TIME, 1500 ).toUInt();
        const quint32 pid = j_params.value("market_id").toString().toUInt();

        const quint64 curtime = QDateTime::currentMSecsSinceEpoch();
        const quint64 timdiff = curtime - _cached_orderbooks_timers.value(pid, 0);
        if ( timdiff < cachingorderbooks_time && _cached_orderbooks.contains(  pid ) ) {
            if ( isApiDebug() ) {
                qWarning() << "Livecoin - reusing orderbooks cache";
            }
            QJsonObject result;
            result["async_uuid"] = QString::number( async_uuid );
            result["success"] = true;
            result["exchange_id"] = QString::number( getExchangeId() );
            result["asks"] = _cached_orderbooks.value( pid ).value("asks").toArray();
            result["bids"] = _cached_orderbooks.value( pid ).value("bids").toArray();
            onApiResponseParsed( async_uuid, result );
            return;
        }

        QUrl url("https://api.livecoin.net/exchange/all/order_book");
        QNetworkRequest request( url );
        QNetworkReply * reply = getManager()->get( request );
        reply->setProperty("uuid", async_uuid );
        reply->setProperty("req_caching", 1 );
        reply->setProperty("market_id", pid );
    } else {
        QUrl url("https://api.livecoin.net/exchange/order_book");
        QUrlQuery query;
        query.addQueryItem("currencyPair", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ) );
        query.addQueryItem("depth", "25");
        url.setQuery( query );
        QNetworkRequest request( url );
        QNetworkReply * reply = getManager()->get( request );
        reply->setProperty("uuid", async_uuid );
    }
}

void SwiftApiClientLivecoin::orderPlace(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrlQuery query;
    if ( !j_params.contains("market_name") ) {
        query.addQueryItem("currencyPair", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ).replace("/","%2F") );
    } else {
        query.addQueryItem("currencyPair", j_params.value("market_name").toString().replace("/","%2F") );
    }
    if ( j_params.value("market_id").toString().toUInt() == 33 ) {
        query.addQueryItem("price",  QString::number( j_params.value("rate").toString().toDouble(), 'f', 5 ) );
    } else {
        query.addQueryItem("price",  QString::number( j_params.value("rate").toString().toDouble(), 'f', 8 ) );
    }
    query.addQueryItem("quantity", QString::number( j_params.value("amount").toString().toDouble(), 'f', 8 ) );
    QNetworkRequest request( j_params.value("type").toString() == "sell" ? QUrl("https://api.livecoin.net/exchange/selllimit") : QUrl("https://api.livecoin.net/exchange/buylimit") );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded" );
    const QByteArray sign = QMessageAuthenticationCode::hash( query.query(QUrl::FullyEncoded).toUtf8() , api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->post( request, query.query(QUrl::FullyEncoded).toUtf8() );
    reply->setProperty("uuid", async_uuid );

}

void SwiftApiClientLivecoin::orderCancel(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrlQuery query;
    query.addQueryItem("currencyPair", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ).replace("/","%2F") );
    query.addQueryItem("orderId", j_params.value("remote_id").toString() );
    QUrl url("https://api.livecoin.net/exchange/cancellimit");
    QNetworkRequest request( url );
    QByteArray sign = QMessageAuthenticationCode::hash( query.query(QUrl::FullyEncoded).toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->post( request,  query.query(QUrl::FullyEncoded).toUtf8() );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::orderGet(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QNetworkRequest request( QUrl("https://api.livecoin.net/exchange/order") );
    QUrlQuery query;
    query.addQueryItem("orderId", j_params.value("remote_id").toString() );
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}


void SwiftApiClientLivecoin::withdrawGetLimits(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}


void SwiftApiClientLivecoin::withdrawHistory(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.livecoin.net/payment/history/transactions");
    QUrlQuery query;
    query.addQueryItem("end", QString::number( QDateTime::currentDateTime().toMSecsSinceEpoch() ) );
    query.addQueryItem("start", QString::number(  QDateTime(QDate::currentDate()).toMSecsSinceEpoch() - 2592000000 + 86400000) );
    query.addQueryItem("types", "WITHDRAWAL" );
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    url.setQuery( query );
    QNetworkRequest request( url );

    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::withdrawCreate(const QJsonObject &j_params, const quint64 &async_uuid)
{
    const quint32 currency_id = j_params.value("currency_id").toString().toUInt();
    const QString cname = SwiftCore::getAssets()->getCurrencyName( currency_id );
    QUrlQuery query;
    query.addQueryItem("amount", j_params.value("amount").toString() );
    QString address( j_params.value("address").toString() );
    if ( j_params.contains("tag") && j_params.value("tag").toString() != "") {
        address += QString( "::" + j_params.value("tag").toString() );
    }
    query.addQueryItem("currency", cname );
    query.addQueryItem("wallet", address );
    QUrl url("https://api.livecoin.net/payment/out/coin");
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded" );
    qWarning() << query.query(QUrl::FullyEncoded).toUtf8();
    const QByteArray sign = QMessageAuthenticationCode::hash( query.query(QUrl::FullyEncoded).toUtf8() , api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->post( request, query.query(QUrl::FullyEncoded).toUtf8() );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::withdrawInner(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientLivecoin::withdrawGetFee(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientLivecoin::getBalances(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    // Pqgpy74mYTt2TuUrKpxR97zSYJrtmntX
    QNetworkRequest request( QUrl("https://api.livecoin.net/payment/balances") );
    QUrlQuery query;
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );

}

void SwiftApiClientLivecoin::getDeposits(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.livecoin.net/payment/history/transactions");
    QUrlQuery query;
    query.addQueryItem("end", QString::number( QDateTime::currentDateTime().toMSecsSinceEpoch() ) );
    query.addQueryItem("start", QString::number(  QDateTime(QDate::currentDate()).toMSecsSinceEpoch() - 2592000000 + 86400000 ) );
    query.addQueryItem("types", "DEPOSIT" );
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::getDepositAddress(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.livecoin.net/payment/get/address");
    QUrlQuery query;
    if ( j_params.contains("currency") ) {
        query.addQueryItem("currency", j_params.value("currency").toString() );
    } else {
        query.addQueryItem("currency", SwiftCore::getAssets()->getCurrencyName( j_params.value("currency_id").toString().toUInt() ) );
    }

    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::tradeHistory(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QNetworkRequest request( QUrl("https://api.livecoin.net/exchange/client_orders") );
    QUrlQuery query;
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::tradeOpenOrders(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QNetworkRequest request( QUrl("https://api.livecoin.net/exchange/client_orders") );
    QUrlQuery query;
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}

void SwiftApiClientLivecoin::tradeGetFee(const QJsonObject &j_params, const quint64 &async_uuid)

{
    Q_UNUSED(j_params)
    QNetworkRequest request( QUrl("https://api.livecoin.net/exchange/commissionCommonInfo") );
    QUrlQuery query;
    QByteArray sign = QMessageAuthenticationCode::hash( query.toString().toUtf8(), api_secret.toUtf8(), QCryptographicHash::Sha256 ).toHex().toUpper();
    request.setRawHeader("Api-Key", api_key.toUtf8() );
    request.setRawHeader("Sign", sign );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid );
}
