#include "swiftapiclientbinance.h"


QString SwiftApiClientBinance::getExchangeName() const
{
    return "binance";
}

void SwiftApiClientBinance::getCurrencies(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QNetworkRequest request( QUrl("https://api.binance.com/api/v3/exchangeInfo") );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::getMarkets(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QNetworkRequest request( QUrl("https://api.binance.com/api/v3/exchangeInfo") );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);

}

void SwiftApiClientBinance::getOrderbooks(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/api/v3/depth");
    QUrlQuery query;
    query.addQueryItem("symbol", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ) );
//    query.addQueryItem("limit", "50" );
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::orderPlace(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/api/v3/order");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("symbol", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ) );
    query.addQueryItem("timeInForce","GTC");
    query.addQueryItem("side", j_params.value("type").toString().toUpper() );
    query.addQueryItem("type", "LIMIT" );
    query.addQueryItem("quantity", QString::number( j_params.value("amount").toString().toDouble(), 'f', 6 ));
    query.addQueryItem("price", QString::number( j_params.value("rate").toString().toDouble(),'f', 6 ));
   // query.addQueryItem("newClientOrderId", j_params.value("local_id").toString() );
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    // url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->post( request, query.toString().toUtf8() );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::orderCancel(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/api/v3/order");
    QUrlQuery query;
    query.addQueryItem("symbol", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ) );
    query.addQueryItem("orderId", j_params.value("remote_id").toString() );
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->deleteResource( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::orderGet(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/api/v3/order");
    QUrlQuery query;
    query.addQueryItem("symbol", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ) );
    query.addQueryItem("orderId", j_params.value("remote_id").toString() );
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::withdrawGetLimits(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientBinance::withdrawHistory(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/wapi/v3/withdrawHistory.html");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}


void SwiftApiClientBinance::withdrawCreate(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/wapi/v3/withdraw.html");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());

    query.addQueryItem("asset", SwiftCore::getAssets()->getCurrencyName( j_params.value("currency_id").toString().toUInt() ) );
    if ( j_params.contains("chain") ) {
        query.addQueryItem("network", j_params.value("chain").toString() );
    }
    query.addQueryItem("address", j_params.value("address").toString() );
    if ( j_params.contains("tag") ) {
        query.addQueryItem("addressTag", j_params.value("tag").toString() );
    }
    query.addQueryItem("amount", j_params.value("amount").toString() );
    query.addQueryItem("timestamp", QString::number( ts ) );

    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    // url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->post( request, query.toString().toUtf8() );
    reply->setProperty("uuid",async_uuid);

}

void SwiftApiClientBinance::withdrawInner(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientBinance::withdrawGetFee(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/sapi/v1/capital/config/getall");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::getBalances(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/api/v3/account");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::getDeposits(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/wapi/v3/depositHistory.html");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);

}

void SwiftApiClientBinance::getDepositAddress(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientBinance::tradeHistory(const QJsonObject &j_params, const quint64 &async_uuid)
{
    QUrl url("https://api.binance.com/api/v3/allOrders");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("symbol", SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() ));
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::tradeOpenOrders(const QJsonObject &j_params, const quint64 &async_uuid)
{    
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/api/v3/openOrders");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    //request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientBinance::tradeGetFee(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QUrl url("https://api.binance.com/wapi/v3/tradeFee.html");
    QUrlQuery query;
    const quint64 ts( getInnerNoncer());
    query.addQueryItem("timestamp", QString::number( ts ));
    const QByteArray signature = QMessageAuthenticationCode::hash( query.toString().toUtf8(), getExchangeApiSecret().toUtf8(), QCryptographicHash::Sha256 ).toHex();
    query.addQueryItem("signature", signature );
    url.setQuery( query );
    QNetworkRequest request( url );
    request.setRawHeader("X-MBX-APIKEY", getExchangeApiKey().toUtf8() );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

quint64 SwiftApiClientBinance::getInnerNoncer()
{
    QMutexLocker lock( &innermutex );
    quint64 res = QDateTime::currentMSecsSinceEpoch() - timeDiff;
    return res;
}
