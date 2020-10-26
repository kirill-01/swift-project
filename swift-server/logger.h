#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QQueue>
#include <QFile>
#include <QTimer>
#include <wampclient.h>
#include "../swift-corelib/swiftbot.h"
#include <QDir>


struct Log {
    Log( const QString&m,const QString&s,const QString&g, const qint64&d);
    Log( const QJsonObject j_obj ) {
        msg = j_obj.value("m").toString();
        sender = j_obj.value("s").toString();
        group = j_obj.value("g").toString();
        ts = j_obj.value("ts").toString().toUInt();
    }
    QString msg;
    QString sender;
    QString group;
    qint64 ts;
    QJsonObject toJson();
};



class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    void loadFromFile();

signals:
private:
    QMap<QString, QVector<Log>> _logs;
    QMap<QString, QVector<Log>> _errors;

    QQueue<Log> _logs_queue;
    QQueue<Log> _errs_queue;
public slots:
    void saveToFile();
    void processLogs();
    void processErrs();
    void onWampSession( Wamp::Session * sess );
private:
    Wamp::Session * session;

};



#endif // LOGGER_H
