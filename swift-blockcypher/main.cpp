#include <QCoreApplication>

#include "module_definitions.h"
#include <swiftbot.h>
#include "blockcypherapi.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName( SWIFT_MODULE_NAME );
    QCoreApplication::setApplicationVersion( SWIFT_MODULE_VERSION );

    SwiftBot::initWampClient();
#ifdef SWIFT_SINGLE_INSTANCE
    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       SwiftBot::addError( "Another instance is already running" );
       return 1;
    }
#endif


    SwiftBot::addLog( "Starting module : " + QString( SWIFT_MODULE_NAME ) + QString( SWIFT_MODULE_VERSION ) );

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription( SWIFT_MODULE_DESCRIPTION );
    parser.addHelpOption();
    parser.addVersionOption();

#ifdef SWIFT_USE_MYSQL_DB
    // MySQL default DB connection. Required for system assets.
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName( SwiftBot::appSettings()->value(SETTINGS_NAME_MYSQL_HOST ).toString() );
    db.setPort( SwiftBot::appSettings()->value(SETTINGS_NAME_MYSQL_PORT ).toInt() );
    db.setUserName( SwiftBot::appSettings()->value(SETTINGS_NAME_MYSQL_USER).toString() );
    db.setPassword( SwiftBot::appSettings()->value(SETTINGS_NAME_MYSQL_PASSWORD ).toString() );
    db.setDatabaseName( SwiftBot::appSettings()->value(SETTINGS_NAME_MYSQL_DBNAME ).toString() );
    if ( !db.open() ) {
        SwiftBot::addError( "MySQL database error: " +  db.lastError().text() );
        return 1;
    }


#ifdef SWIFT_MODULE_INITIAL_SQL
    if ( SwiftBot::hasSqlMigrationsDir() ) {
        SwiftBot::addLog( "Trying to apply SQL migrations" );
        SwiftBot::applySqlMigrations();
    }
#endif

#endif

    // "swift.address.generate","swift.address.balance","swift.address.withdraw"
    BlockCypherApi block_api;

    SwiftBot::provide("swift.address.generate", [&block_api]( const QVariantList&v, const QVariantMap&m){
        Q_UNUSED(m)
        qWarning() << "call";
        const QString r = block_api.createAddress( v.at(0).toUInt() );
        qWarning() << r << "response";
        return r;
    });

    SwiftBot::provide("swift.address.balance", [&block_api]( const QVariantList&v, const QVariantMap&m){
        Q_UNUSED(m)
        const double r = block_api.getBalance( v.at(0).toUInt() );
        return r;
    });

    SwiftBot::provide("swift.address.withdraw", [&block_api]( const QVariantList&v, const QVariantMap&m){
        Q_UNUSED(m)
        const quint32 from_id( v.at(0).toUInt() );
        const QString toAddress( v.at(1).toString() );
        const double amount( v.at(2).toDouble() );
        const double r = block_api.withdraw( from_id, toAddress, amount );
        return r;
    });
    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        wamp_client->publish( FEED_WATCHDOG, { QCoreApplication::applicationName().replace("swift-","")});
    });
    watchdog->start();

    wamp_client->startClient();
    return a.exec();
}
