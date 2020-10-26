#ifndef MODULEWORKER_H
#define MODULEWORKER_H

#include <QObject>

#include <wampclient.h>
#include <swiftbot.h>

typedef QMap<quint32, QMap<double,double>> OrderBookQuotes;

// ASKS, BIDS - Always this direction
typedef QPair<OrderBookQuotes,OrderBookQuotes> OrderBooks;

class ModuleWorker : public QObject
{
    Q_OBJECT
public:
    explicit ModuleWorker(QObject *parent = nullptr);

    SwiftBot::AssetsData prepareAssets( const QVariantList & v );
    OrderBooks prepareOrderbooks( const QVariantList & v );
signals:
    void callRpc( const QString& path, const QVariantList & args = QVariantList() );
    void publishFeed( const QString& path, const QVariantList & args );
    void subscribeFeed( const QString& path, const QString& name );
public slots:
    virtual void onRpcResult( const QVariant& )=0;
    virtual void onFeedMessage( const QString&, const QVariantList & )=0;
    virtual void onClientStarted()=0;

};

#endif // MODULEWORKER_H
