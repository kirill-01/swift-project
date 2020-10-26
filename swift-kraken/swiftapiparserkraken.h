#ifndef SWIFTAPIPARSERKRAKEN_H
#define SWIFTAPIPARSERKRAKEN_H
#include "../swift-corelib/swiftapiparser.h"
#include "../swift-corelib/swiftcore.h"


class SwiftApiParserKraken : public SwiftApiParser
{
    Q_OBJECT
public:
    explicit SwiftApiParserKraken(QObject *parent = nullptr);

    // SwiftApiParser interface

    QString getExchangeName() const override;

    //typedef
public slots:
    void parseResponse( const quint64& uuid, const SwiftApiClient::AsyncMethods &method, const QByteArray &data) override;

private:
    int GetMarketId(const QString& market) const;
    QString orderStatus(const QString& status) const;
    QString ioStatus(const QString& status) const;
};

#endif // SWIFTAPIPARSERKRAKEN_H
