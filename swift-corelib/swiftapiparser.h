#ifndef SWIFTAPIPARSER_H
#define SWIFTAPIPARSER_H

#include <QObject>
#include "swiftapiclient.h"
#include "swiftcore.h"

class SwiftApiParser : public QObject
{
    Q_OBJECT
public:
    explicit SwiftApiParser(QObject *parent = nullptr);
    quint32 getExchangeId() const;
    virtual QString getExchangeName() const=0;
    bool isApiDebug() const;
    void sendResponse( const quint64&uuid, const QJsonObject& j_result );
    QSettings * getSettings() {
        return SwiftCore::getModuleSettings( getExchangeName() );
    }
signals:
    void resultParsed( const quint64& uuid, const QJsonObject& j_result );

public slots:
    void registerResponse( const quint64& uuid, const SwiftApiClient::AsyncMethods& method, const QByteArray& data );
    virtual void parseResponse(  const quint64& uuid, const SwiftApiClient::AsyncMethods& method, const QByteArray& data )=0;
private:
    QMap<quint64,QByteArray> _rawdatas;

};

#endif // SWIFTAPIPARSER_H
