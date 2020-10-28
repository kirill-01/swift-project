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
#include <swiftbot.h>
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
    void init() {
        loadFromFile();
        QTimer::singleShot( 30000, this, &Logger::saveToFile );
        QTimer::singleShot( 500, this, &Logger::processErrs );
        QTimer::singleShot( 500, this, &Logger::processLogs );

        SwiftBot::provide(RPC_SERVER_LOGGER_LOGS, [&](const QVariantList&v, const QVariantMap&m) {
            //[SENDER,GROUP,MESSAGE]
            if ( v.size() != 3 ) {
                return true;
            }
            const QString sender( v.at(0).toString() );
            const QString group( v.at(1).toString() );
            const QString msg( v.at(2).toString() );
            const qint64 ts = QDateTime::currentSecsSinceEpoch();
            if ( group != "DEBUG" ) {
                _logs[group].push_back( Log(msg,sender,group,ts) );
            }
            // [SENDER,GROUP,TS,MESSAGE]
            _logs_queue.enqueue( Log(msg,sender,group,ts) );
            Q_UNUSED(m);
            return true;
        });
        SwiftBot::provide(RPC_SERVER_LOGGER_ERRORS, [&](const QVariantList&v, const QVariantMap&m) {
            //[SENDER,GROUP,MESSAGE]
            if ( v.size() != 3 ) {
                return true;
            }
            const QString sender( v.at(0).toString() );
            const QString group( v.at(1).toString() );
            const QString msg( v.at(2).toString() );
            const qint64 ts = QDateTime::currentSecsSinceEpoch();
            _errors[group].push_back( Log(msg,sender,group,ts));
            // [SENDER,GROUP,TS,MESSAGE]
            _errs_queue.enqueue( Log(msg,sender,group,ts) );
            Q_UNUSED(m);
            return true;
        });

    }
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
