#include "templateworker.h"

#include <swiftbot.h>

class TemplateStorageData : public QSharedData
{
public:
    OrderBooks _actual_orderbooks;

};

TemplateWorker::TemplateWorker(QObject *parent) : ModuleWorker(parent), data(new TemplateStorageData)
{

}

TemplateWorker::TemplateWorker(const TemplateWorker &rhs) : ModuleWorker( rhs.parent() ), data(rhs.data)
{

}

TemplateWorker &TemplateWorker::operator=(const TemplateWorker &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

TemplateWorker::~TemplateWorker()
{

}

TemplateWorker::TemplateWorker() {

}


void TemplateWorker::onRpcResult(const QVariant & res )
{
    qInfo() << "RPC Call result: " << res;
}

void TemplateWorker::onFeedMessage(const QString &feed, const QVariantList &data)
{
    if ( feed == FEED_ORDERBOOKS_SNAPSHOT ) {
        // last_orderbooks = prepareOrderbooks( data );
    } else if ( FEED_EVENTS_BALANCES ) {

    } else if ( FEED_EVENTS_ORDERS ) {
        qInfo() << "Orders event received" << data;
    } else {
        qWarning() << "Data received to unknown channel or alias";
    }
}

void TemplateWorker::onClientStarted()
{
#ifdef SWIFT_LISTEN_ORDERBOOKS
    emit subscribeFeed( QString(FEED_ORDERBOOKS_SNAPSHOT), QString(FEED_ORDERBOOKS_SNAPSHOT) );
#endif

#ifdef  SWIFT_LISTEN_BALANCE
    emit subscribeFeed( QString(FEED_EVENTS_BALANCES), QString(FEED_EVENTS_BALANCES) );
#endif

#ifdef SWIFT_LISTEN_ORDERS
    emit subscribeFeed( QString(FEED_EVENTS_ORDERS), QString(FEED_EVENTS_ORDERS) );
#endif
    qDebug() << "Example RPC call sending";
    emit callRpc( RPC_ASSETS_GET );
}
