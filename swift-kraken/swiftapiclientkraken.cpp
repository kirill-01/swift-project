#include "swiftapiclientkraken.h"


SwiftApiClientKraken::SwiftApiClientKraken(QObject* parent) : SwiftApiClient(parent)
{
}

int SwiftApiClientKraken::getCounter() const {
    return counter;
}


QString SwiftApiClientKraken::getExchangeName() const{
    return "kraken";
}

QString SwiftApiClientKraken::getApiVersionString(){
    return "1.0.0";
}

void SwiftApiClientKraken::getCurrencies(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    // all GET params - defaults
    QUrl url(url_public + "Assets");
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientKraken::getMarkets(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    // all GET params - defaults
    QUrl url(url_public + "AssetPairs");
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientKraken::getOrderbooks(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrl url(url_public + "Depth");
    QUrlQuery args;
    args.addQueryItem("pair", SwiftCore::getAssets()->getMarketName(j_params.value("market_id").toString().toUInt() ));
    url.setQuery(args);
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}

void SwiftApiClientKraken::orderPlace(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body;
    const quint32 pid = j_params.value("market_id").toString().toUInt();
    body.addQueryItem("pair", SwiftCore::getAssets()->getMarketName( pid ) );
    body.addQueryItem("type", j_params.value("type").toString());
    body.addQueryItem("ordertype", "limit");
    body.addQueryItem("userref", j_params.value("local_id").toString() );
    body.addQueryItem("price", QString::number( j_params.value("rate").toString().toDouble(),'f', SwiftCore::getAssets()->getMarketPricePrecision( pid ) ) );
    body.addQueryItem("volume",QString::number( j_params.value("amount").toString().toDouble(),'f', SwiftCore::getAssets()->getMarketAmountPrecision( pid ) ) );
    QNetworkReply * reply = authPostRequest(QUrl(url_private + "AddOrder"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::orderCancel(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body( {{"txid", j_params.value("remote_id").toString()}} );
    auto reply = authPostRequest(QUrl(url_private + "CancelOrder"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::orderGet(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body( {{"txid", j_params.value("remote_id").toString()}} );
    auto reply = authPostRequest(url_private + "QueryOrders", body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::withdrawGetLimits(const QJsonObject &j_params, const quint64 &async_uuid){
    // ???
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)

    // WithdrawInfo???
}

void SwiftApiClientKraken::withdrawHistory(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body({
                   {"asset", SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt()).toLower()}
                   });
    auto reply = authPostRequest(QUrl(url_private + "WithdrawStatus"), body);
    reply->setProperty("uuid", async_uuid);
}


void SwiftApiClientKraken::withdrawCreate(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body({
                       {"asset", SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt()).toLower() },
                       { "key", j_params.value("address").toString().left(24) },
                       { "amount", j_params.value("amount").toString() }
                   });
    auto reply = authPostRequest(QUrl(url_private + "Withdraw"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::withdrawInner(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body({
                       {"asset", SwiftCore::getAssets()->getMarketName(j_params.value("currency_id").toString().toUInt())},
                       { "to", j_params.value("to").toString() },
                       { "from", j_params.value("from").toString() },
                       { "amount", j_params.value("amount").toString() }
                   });
    auto reply = authPostRequest(QUrl(url_private + "WalletTransfer"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::withdrawGetFee(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

void SwiftApiClientKraken::getBalances(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QUrlQuery body;
    auto reply = authPostRequest(QUrl(url_private + "Balance"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::getDeposits(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body({
                       {"asset", SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt()).toLower() }
                   });
    auto reply = authPostRequest(QUrl(url_private + "DepositStatus"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::getDepositAddress(const QJsonObject &j_params, const quint64 &async_uuid){
    QUrlQuery body({
                       {"asset", SwiftCore::getAssets()->getMarketName(j_params.value("currency_id").toString().toUInt())},
                       { "method", j_params.value("method").toString() }
                   });
    auto reply = authPostRequest(QUrl(url_private + "DepositAddresses"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::tradeHistory(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QUrlQuery body;
    auto reply = authPostRequest(QUrl(url_private + "ClosedOrders"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::tradeOpenOrders(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QUrlQuery body;
    auto reply = authPostRequest(QUrl(url_private + "OpenOrders"), body);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientKraken::tradeGetFee(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    // all GET params - defaults
    QUrl url(url_public + "AssetPairs");
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid",async_uuid);
}


QNetworkReply* SwiftApiClientKraken::authPostRequest(const QUrl &url_path, QUrlQuery& body) {
    QString nonce = QString::number(quint64(QDateTime::currentMSecsSinceEpoch())* 1000);
    body.addQueryItem("nonce", nonce);
    const auto signature = QMessageAuthenticationCode::hash(
                url_path.path().toUtf8() + QCryptographicHash::hash((nonce + body.toString()).toUtf8(), QCryptographicHash::Sha256),
                QByteArray::fromBase64(api_secret.toUtf8()),
                QCryptographicHash::Sha512
                ).toBase64();

    QNetworkRequest request(url_path);
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("API-Key", api_key.toUtf8());
    request.setRawHeader("API-Sign", signature);
    return getManager()->post(request, body.toString().toUtf8());
}

inline QString SwiftApiClientKraken::sideToStr(int side) const {
    switch (side) {
        case 0: return "sell";
        case 1: return "buy";
    }
    return "undefined";
}

inline bool SwiftApiClientKraken::isRequreSecondPrice(const QString& ordertype) const {
    if(ordertype == "stop-loss-profit")
        return true;
    else if(ordertype == "stop-loss-profit")
        return true;
    else if(ordertype == "stop-loss-profit-limit")
        return true;
    else if(ordertype == "stop-loss-limit")
        return true;
    else if(ordertype == "trailing-stop-limit")
        return true;
    else if(ordertype == "stop-loss-and-limit")
        return true;
    else return false;
}
