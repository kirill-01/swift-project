#ifndef TEMPLATEWORKER_H
#define TEMPLATEWORKER_H

#include <QObject>
#include <QSharedDataPointer>
#include <moduleworker.h>

class TemplateStorageData;

class TemplateWorker : public ModuleWorker
{
public:
    TemplateWorker();
    // ModuleWorker interface
    TemplateWorker(QObject *parent);
    TemplateWorker(const TemplateWorker &rhs);
    TemplateWorker &operator =(const TemplateWorker &rhs);
    ~TemplateWorker();
public slots:
    void onRpcResult(const QVariant &) override;
    void onFeedMessage(const QString &feed, const QVariantList &) override;
    void onClientStarted() override;
private:
    SwiftBot::AssetsData assets_data;
    OrderBooks last_orderbooks;private:
    QSharedDataPointer<TemplateStorageData> data;
};

#endif // TEMPLATEWORKER_H
