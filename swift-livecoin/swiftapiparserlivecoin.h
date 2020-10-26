#ifndef SWIFTAPIPARSERLIVECOIN_H
#define SWIFTAPIPARSERLIVECOIN_H

#include <QObject>
#include "../swift-corelib/swiftapiparser.h"
#include "../swift-corelib/swiftcore.h"
class SwiftApiParserLivecoin : public SwiftApiParser
{
    Q_OBJECT
public:
    SwiftApiParserLivecoin(QObject *parent = nullptr);

signals:


    // SwiftApiParser interface
public:

    QString getExchangeName() const override;

public slots:
    void parseResponse(const quint64 &uuid, const SwiftApiClient::AsyncMethods &method, const QByteArray &data) override;
};

#endif // SWIFTAPIPARSERLIVECOIN_H
