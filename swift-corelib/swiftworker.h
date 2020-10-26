#ifndef SWIFTWORKER_H
#define SWIFTWORKER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QMutex>
#include "wampclient.h"
#include <QNetworkInterface>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlError>
#include "swiftcore.h"

#ifndef APP_NAME
#define APP_NAME "Swift worker"
#endif
#ifndef APP_VERSION
#define APP_VERSION "2.0.2016"
#endif
class SwiftWorker : public QObject
{
    Q_OBJECT
public:
    QString getConfigValue( const QString& param );
    void insertLog( const QString& group, const QString& message );
    void insertError( const QString& group, const QString& message );
    explicit SwiftWorker(QObject *parent = nullptr);

    virtual QStringList listenParams();
    virtual void onParamChanged( const QString& param, const QString& value );
    virtual void initWorker( Wamp::Session * sess )=0;
    virtual QString getWorkerName() const=0;
    void setStatus( const quint32& stat );
    quint32 getStatus() const;
    QSqlDatabase _ch;
    Wamp::Session * getSession();

    Wamp::Session * session;
    void setIsInited();
signals:
    void restartRequested();
public slots:
    void watchdog();
    void onRestart();
    void onWampSession( Wamp::Session * sess );
private:
    quint32 status;
    QString hostip;
    QTimer * wdtimer;


};

#endif // SWIFTWORKER_H
