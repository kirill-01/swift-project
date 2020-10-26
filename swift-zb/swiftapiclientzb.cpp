#include "swiftapiclientzb.h"

SwiftApiClientZB::SwiftApiClientZB(QObject* parent) : SwiftApiClient(parent)
{}


QString SwiftApiClientZB::getExchangeName() const{
    return "zb";
}

QString SwiftApiClientZB::getApiVersionString(){
    return "1.0.0";
}

void SwiftApiClientZB::getCurrencies(const QJsonObject &j_params, const quint64 &async_uuid){
    unrealizedMethod(j_params, async_uuid);
}

void SwiftApiClientZB::getMarkets(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QString url( QStringList({publicPrefix, "markets"}).join("/") );
    QNetworkRequest request( url );
    QNetworkReply * reply = getManager()->get( request );
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientZB::getOrderbooks(const QJsonObject &j_params, const quint64 &async_uuid){
    QString url("http://api.zb.live/data/v1/depth");
    QNetworkRequest request(QUrl(url + "?" + "market=" + SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() )));
    QNetworkReply* reply = getManager()->get(request);
    reply->setProperty("uuid", async_uuid);
}

void SwiftApiClientZB::orderPlace(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    seq["method"] = "order";
    seq["accesskey"] = api_key;
    seq["amount"] = j_params.value("amount").toString();
    seq["currency"] = SwiftCore::getAssets()->getMarketName(j_params.value("market_id").toString().toUInt());
    seq["customerOrderId"] = j_params.value("local_id").toString();
    seq["orderType"] = "2";
    seq["price"] = j_params.value("rate").toString();
    seq["tradeType"] = j_params.value("type").toString() == "sell" ? "0" : "1";
    QString url(QStringList({privatePrefix, "order"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::orderCancel(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    if(isParamExist({"market_id", "remote_id"}, j_params, errorMessage)){
        if(isUintValid(j_params.value("market_id").toString().toUInt(), errorMessage)){

            seq["method"] = "cancelOrder";
            seq["accesskey"] = api_key;
            seq["currency"] = SwiftCore::getAssets()->getMarketName(j_params.value("market_id").toString().toUInt());
            seq["id"] = j_params.value("remote_id").toString();
            QString url(QStringList({privatePrefix, "cancelOrder"}).join("/"));
            buildAuthRequest(async_uuid, url, seq);
            return;
        }
    }
    addError("[orderCancel] " + errorMessage);
}

void SwiftApiClientZB::orderGet(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    if(isParamExist({"market_id", "remote_id"}, j_params, errorMessage)){
        if(isUintValid(j_params.value("market_id").toString().toUInt(), errorMessage)){

            seq["method"] = "cancelOrder";
            seq["accesskey"] = api_key;
            seq["currency"] = SwiftCore::getAssets()->getMarketName(j_params.value("market_id").toString().toUInt());
            seq["id"] = j_params.value("remote_id").toString();
            QString url(QStringList({privatePrefix, "getOrder"}).join("/"));
            buildAuthRequest(async_uuid, url, seq);
            return;
        }
    }
    addError("[orderGet] " + errorMessage);
}

void SwiftApiClientZB::withdrawGetLimits(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QMap<QString, QString> seq;
    seq["method"] = "getSubUserList";
    seq["accesskey"] = api_key;
    QString url(QStringList({privatePrefix, "getSubUserList"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::withdrawHistory(const QJsonObject &j_params, const quint64 &async_uuid){
    QMap<QString, QString> seq;
    seq["method"] = "getWithdrawRecord";
    seq["accesskey"] = api_key;
    seq["currency"] = SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt() );
    seq["pageIndex"] = "1";
    seq["pageSize"] = "50";
    QString url(QStringList({privatePrefix, "getWithdrawRecord"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::withdrawCreate(const QJsonObject &j_params, const quint64 &async_uuid){
    QMap<QString, QString> seq;
    double fee = SwiftCore::getAssets()->getCurrencyWithdrawFee( j_params.value("currency_id").toString().toUInt() );
    const double amount = j_params.value("amount").toString().toDouble();
    seq["accesskey"] = api_key;
    seq["amount"] = QString::number( amount, 'f', 4 );
    seq["currency"] = SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt() );
    seq["fees"] = QString::number( fee, 'f', 4 );
    seq["itransfer"] = QString::number( 0 );
    seq["method"] = "withdraw";
    seq["receiveAddr"] = j_params.value("address").toString();
    QString url(QStringList({privatePrefix, "withdraw"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::withdrawInner(const QJsonObject &j_params, const quint64 &async_uuid){
    unrealizedMethod(j_params, async_uuid);
}

void SwiftApiClientZB::withdrawGetFee(const QJsonObject &j_params, const quint64 &async_uuid){
    unrealizedMethod(j_params, async_uuid);
}

void SwiftApiClientZB::getBalances(const QJsonObject &j_params, const quint64 &async_uuid){
    Q_UNUSED(j_params)
    QMap<QString, QString> seq;
    seq["method"] = "getAccountInfo";
    seq["accesskey"] = api_key;
    QString url(QStringList({privatePrefix, "getAccountInfo"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);

}

void SwiftApiClientZB::getDeposits(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    if(isParamExist({"currency_id"}, j_params, errorMessage)){
            seq["method"] = "getChargeRecord";
            seq["accesskey"] = api_key;
            seq["currency"] = SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt());
            seq["pageIndex"] = "1";
            seq["pageSize"] = "50";
            QString url(QStringList({privatePrefix, "getChargeRecord"}).join("/"));
            buildAuthRequest(async_uuid, url, seq);
            return;
    }
}

void SwiftApiClientZB::getDepositAddress(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    if(isParamExist({"currency_id"}, j_params, errorMessage)){
        if(isUintValid(j_params.value("currency_id").toString().toUInt(), errorMessage, "currency_id")){

            seq["method"] = "getPayinAddress";
            seq["accesskey"] = api_key;
            seq["currency"] = SwiftCore::getAssets()->getCurrencyName(j_params.value("currency_id").toString().toUInt());
            QString url(QStringList({privatePrefix, "getPayinAddress"}).join("/"));
            buildAuthRequest(async_uuid, url, seq);
            return;
        }
    }
    addError("[getDepositAddress] " + errorMessage);
}

void SwiftApiClientZB::tradeHistory(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    seq["method"] = "getOrdersIgnoreTradeType";
    seq["accesskey"] = api_key;
    seq["currency"] = SwiftCore::getAssets()->getMarketName(j_params.value("market_id").toString().toUInt());
    seq["pageIndex"] = "1";
    seq["pageSize"] = "90";
    QString url(QStringList({privatePrefix, "getOrdersIgnoreTradeType"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::tradeOpenOrders(const QJsonObject &j_params, const quint64 &async_uuid){
    QString errorMessage;
    QMap<QString, QString> seq;
    seq["method"] = "getUnfinishedOrdersIgnoreTradeType";
    seq["accesskey"] = api_key;
    seq["currency"] = SwiftCore::getAssets()->getMarketName( j_params.value("market_id").toString().toUInt() );
    seq["pageIndex"] = "1";
    seq["pageSize"] = "90";
    QString url(QStringList({privatePrefix, "getUnfinishedOrdersIgnoreTradeType"}).join("/"));
    buildAuthRequest(async_uuid, url, seq);
}

void SwiftApiClientZB::tradeGetFee(const QJsonObject &j_params, const quint64 &async_uuid){
    unrealizedMethod(j_params, async_uuid);
}

QString SwiftApiClientZB::buildSignature(const QString& params) const {
    QByteArray key_sha1 = QCryptographicHash::hash(api_secret.toUtf8(), QCryptographicHash::Sha1).toHex();

    // mda5
    QByteArray k_ipad(64, 54), k_opad(64, 92),
            keyb, value;
    keyb = key_sha1;
    value = params.toUtf8();
    for(auto i = 0; i < keyb.size(); ++i){
        k_ipad[i] = keyb[i] ^ 0x36;
        k_opad[i] = keyb[i] ^ 0x5c;
    }
    value = QCryptographicHash::hash(k_ipad + value, QCryptographicHash::Md5);
    return QString(QCryptographicHash::hash(k_opad + value, QCryptographicHash::Md5).toHex());
}

void SwiftApiClientZB::buildAuthRequest(const quint64 &uuid, const QString& method_url, const QMap<QString, QString>& params)
{
    QUrl url;
    QString params_str;
    for(auto it = params.cbegin(); it != params.cend(); ++it){
        if(it != params.cbegin())
            params_str += "&";
        params_str += it.key() + "=" + it.value();
    }
    params_str += "&sign=" + buildSignature(params_str) + "&reqTime=" + QString::number(QDateTime::currentMSecsSinceEpoch());

    QNetworkRequest request(QUrl(method_url + "?" + params_str));
    QNetworkReply* reply = getManager()->get(request);
    reply->setProperty("uuid", uuid);
}


bool SwiftApiClientZB::isUintValid(quint32 id, QString& error, const QString& id_name) {
    if(id > 0)
        return true;
    error = "Invalid parameter - " + id_name;
    return false;
}
bool SwiftApiClientZB::isPriceValid(double price, QString& error) {
    if(price > 0)
        return true;
    error = "Invalid parameter - rate";
    return false;
}
bool SwiftApiClientZB::isAmountValid(double amount, QString& error) {
    if(amount > 0)
        return true;
    error = "Invalid parameter - amount";
    return false;
}
bool SwiftApiClientZB::isSideValid(const QString& side, QString& error) {
    if(side == "sell" || side == "buy")
        return true;
    error = "Invalid parameter - type";
    return false;
}

bool SwiftApiClientZB::isParamExist(const QStringList& params, const QJsonObject& j_params, QString& error) {
    QStringList absentParams;
    for(auto& p : params){
        if(!j_params.contains(p))
            absentParams.append(p);
    }
    if(absentParams.isEmpty()){
        error = "";
        return true;
    }
    else {
        error = "Absent parameters: <" + absentParams.join(",") + ">";
        return false;
    }
}

