#include "logger.h"
#include <swiftcore.h>

Logger::Logger(QObject *parent) : QObject(parent), session( nullptr )
{
    loadFromFile();
    QTimer::singleShot( 30000, this, &Logger::saveToFile );
    QTimer::singleShot( 500, this, &Logger::processErrs );
    QTimer::singleShot( 500, this, &Logger::processLogs );
}


void Logger::loadFromFile() {

    QDir logsdir( LOGS_DIRNAME );
    logsdir.setNameFilters(QStringList("*.log"));
    logsdir.setFilter( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
    QStringList logs_groups_files = logsdir.entryList();
    if ( !logs_groups_files.isEmpty() ) {
        for( auto it = logs_groups_files.begin(); it != logs_groups_files.end(); it ++ ) {
            QFile f( *it );
            if ( f.open( QFile::ReadOnly ) ) {
                QJsonArray _itms( QJsonDocument::fromJson( f.readAll().constData() ).array() );
                for( auto it = _itms.begin(); it != _itms.end(); it++ ) {
                    _logs[ it->toObject().value("g").toString()].push_back( Log( it->toObject() ) );
                }
            }
        }
    }

    QDir errsdir( ERRS_DIRNAME );
    errsdir.setNameFilters(QStringList("*.log"));
    errsdir.setFilter( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
    QStringList errs_groups_files = errsdir.entryList();
    if ( !errs_groups_files.isEmpty() ) {
        for( auto it = errs_groups_files.begin(); it != errs_groups_files.end(); it ++ ) {
            QFile f( *it );
            if ( f.open( QFile::ReadOnly ) ) {
                QJsonArray _itms( QJsonDocument::fromJson( f.readAll().constData() ).array() );
                for( auto it = _itms.begin(); it != _itms.end(); it++ ) {
                    _errors[ it->toObject().value("g").toString()].push_back( Log( it->toObject() ) );
                }
            }
        }
    }
}

void Logger::saveToFile() {


    QStringList logsgroups( _logs.keys() );
    for( auto it = logsgroups.begin(); it != logsgroups.end(); it++ ) {
        QFile groupfile( QString(LOGS_DIRNAME)+*it+".log" );
        if ( groupfile.open( QFile::ReadWrite ) ) {
            groupfile.resize(0);
            QJsonArray j;
            QVector<Log> _itms( _logs.value( *it ) );
            for( auto it = _itms.begin(); it != _itms.end(); it++ ) {
                j.push_back( it->toJson() );
            }
            groupfile.write( QJsonDocument( j ).toJson( QJsonDocument::Compact ) );
            groupfile.close();
        }
    }


    QStringList errsgroups( _errors.keys() );
    for( auto it = errsgroups.begin(); it != errsgroups.end(); it++ ) {
        QFile groupfile( QString(ERRS_DIRNAME)+*it+".log" );
        if ( groupfile.open( QFile::ReadWrite ) ) {
            groupfile.resize(0);
            QJsonArray j;
            QVector<Log> _itms( _errors.value( *it ) );
            for( auto it = _itms.begin(); it != _itms.end(); it++ ) {
                j.push_back( it->toJson() );
            }
            groupfile.write( QJsonDocument( j ).toJson( QJsonDocument::Compact ) );
            groupfile.close();
        }
    }
    QTimer::singleShot( 30000, this, &Logger::saveToFile );
}

void Logger::processLogs() {
    if ( !_logs_queue.isEmpty() ) {
        while ( !_logs_queue.isEmpty()) {
            Log next( _logs_queue.dequeue() );
            const QJsonObject msg({{"s",next.sender},{"g",next.group},{"m",next.msg},{"ts",QString::number( next.ts )}});
            const QString m( QJsonDocument ( msg ).toJson( QJsonDocument::Compact ) );

            session->publish(FEED_LOGS, {m});
            if ( SwiftCore::getSettings()->value("logs_output", false).toBool() && next.group == "INFO") {
                qWarning().noquote() << QDateTime::currentDateTime().time().toString() << next.sender << QString("["+next.group+"]") << next.msg;
            } else if (  SwiftCore::getSettings()->value("reports_outputs", false).toBool()  && next.group == "REPORTS"  ) {
                qWarning().noquote() << QDateTime::currentDateTime().time().toString() << next.sender << QString("["+next.group+"]") << next.msg;
            } else if (  SwiftCore::getSettings()->value("debugs_outputs", false).toBool()   && next.group == "DEBUG" ) {
                qWarning().noquote() << QDateTime::currentDateTime().time().toString() << next.sender << QString("["+next.group+"]") << next.msg;
            }
        }
    }
    QTimer::singleShot( 500, this, &Logger::processLogs );
}

void Logger::processErrs() {
    if ( !_errs_queue.isEmpty() ) {
        while ( !_errs_queue.isEmpty()) {
            Log next( _errs_queue.dequeue() );
            const QJsonObject msg({{"s",next.sender},{"g",next.group},{"m",next.msg},{"ts",QString::number( next.ts )}});
            const QString m( QJsonDocument ( msg ).toJson( QJsonDocument::Compact ) );
            session->publish(FEED_ERRORS, {m});
            if ( SwiftCore::getSettings()->value("errors_output", false).toBool()  && next.group == "WARNING") {
                qWarning().noquote() << QDateTime::currentDateTime().time().toString() << next.sender << QString("["+next.group+"]") << next.msg;
            }
        }
    }
    QTimer::singleShot( 500, this, &Logger::processErrs );
}

void Logger::onWampSession(Wamp::Session *sess) {
    session = sess;

    session->provide(RPC_SERVER_LOGGER_LOGS, [&](const QVariantList&v, const QVariantMap&m) {
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
    session->provide(RPC_SERVER_LOGGER_ERRORS, [&](const QVariantList&v, const QVariantMap&m) {
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

Log::Log(const QString &m, const QString &s, const QString &g, const qint64 &d)
    : msg(m),sender(s),group(g),ts(d) {
}

QJsonObject Log::toJson() {
    return QJsonObject({{"m",msg},{"s",sender},{"g",group},{"ts",QString::number(ts)}});
}

