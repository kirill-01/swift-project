#include "swiftcore.h"
#include <QDebug>
#include <QCoreApplication>

SwiftCore::SwiftCore()
{
}

QSqlQuery SwiftCore::createQuery(const QString &dbname)
{
    static QHash<QString, QSqlDatabase> _dbs;
    if ( _dbs.contains( dbname ) ) {
         if ( _dbs[dbname].isOpen()  ) {
              return QSqlQuery( _dbs[dbname] );
         }
    } else {
        _dbs[dbname] = QSqlDatabase::addDatabase("QMYSQL", dbname );
    }

    QSqlDatabase db = _dbs[dbname];
    if( QSqlDatabase::contains( dbname ) )
    {
        db = QSqlDatabase::database( dbname );
        if ( !db.isOpen() ) {
            db.setHostName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_HOST,"localhost" ).toString() );
            db.setPort( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PORT, 3306 ).toInt() );
            db.setUserName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_USER,"swiftbot" ).toString() );
            db.setPassword( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PASSWORD,"swiftbot" ).toString() );
            db.setDatabaseName( SwiftCore::getSettings()->value("mysql_db","swiftbot" ).toString() );
            if ( !db.open() ) {
                qWarning() << db.lastError().text();
                qApp->exit( 1 );
            }
        }
    }
    else
    {
        db.setHostName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_HOST,"localhost" ).toString() );
        db.setPort( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PORT, 3306 ).toInt() );
        db.setUserName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_USER,"swiftbot" ).toString() );
        db.setPassword( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PASSWORD,"swiftbot" ).toString() );
        db.setDatabaseName( SwiftCore::getSettings()->value("mysql_db","swiftbot" ).toString() );
        if ( !db.open() ) {
            qWarning() << db.lastError().text();
            qApp->exit( 1 );
        }
    }
    _dbs[dbname] = db;
    return QSqlQuery( db );
}
