#include "swiftapiparser.h"

SwiftApiParser::SwiftApiParser(QObject *parent) : QObject(parent)
{
 qRegisterMetaType<SwiftApiClient::AsyncMethods>("SwiftApiClient::AsyncMethods");
}

quint32 SwiftApiParser::getExchangeId() const {
    static quint32 _id = 0;
    if ( _id == 0 ) {
        _id = SwiftCore::getAssets()->getExchangeId( getExchangeName() );
    }
    return _id;
}

bool SwiftApiParser::isApiDebug() const {
    return SwiftCore::getSettings()->value( SETTINGS_NAME_API_DEBUG, false ).toBool();
}

void SwiftApiParser::sendResponse(const quint64 &uuid, const QJsonObject &j_result) {
    QJsonObject resp( j_result );
    if ( isApiDebug() ) {
        QJsonObject jdebug;
        //jdebug["raw"] = QString( _rawdatas.take( uuid ) );
        jdebug["ts"] = QString::number( QDateTime::currentMSecsSinceEpoch() );
        jdebug["async_uuid"] = QString::number( uuid );
        resp["debug"] = jdebug;
    }
    resp["async_uuid"] = QString::number( uuid );
    resp["exchange_id"] = QString::number( getExchangeId() );
    emit resultParsed( uuid, resp );
}

void SwiftApiParser::registerResponse(const quint64 &uuid, const SwiftApiClient::AsyncMethods &method, const QByteArray &data) {
    if ( isApiDebug() ) {
        _rawdatas.insert( uuid, data );
    }
    parseResponse( uuid, method, data );
}
