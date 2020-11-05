#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <swiftbot.h>

#include <QLockFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <iostream>

#include "balanceskeeper.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-balance");
    QCoreApplication::setApplicationVersion("1.0.379");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("SwiftBot balances module");
    parser.addHelpOption();
    parser.addVersionOption();

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    static QString app_dir( parser.value( targetDirectoryOption ) );

    // MySQL db
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_HOST ).toString() );
    db.setPort( SwiftBot::appParam(SETTINGS_NAME_MYSQL_PORT ).toInt() );
    db.setUserName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_USER).toString() );
    db.setPassword( SwiftBot::appParam(SETTINGS_NAME_MYSQL_PASSWORD ).toString() );
    db.setDatabaseName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_DBNAME ).toString() );

    if ( !db.open() ) {
        qWarning() << "MySQL database error: ";
        qWarning() << db.lastError().text();
        return 1;
    }
    std::cout << "SwiftBot  v." << QCoreApplication::applicationVersion().toStdString() << std::endl;
    std::cout << "Running on host: " << SwiftBot::getHostIpAddress().toStdString() << std::endl;


    // Wamp client
    wamp_client.reset(new WampClient(
       SwiftBot::appParam(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
       SwiftBot::appParam(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
       SwiftBot::appParam(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
       SwiftBot::appParam(SETTINGS_NAME_WAMP_DEBUG, false).toBool() ));

    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        wamp_client->publish( FEED_WATCHDOG, { QCoreApplication::applicationName().replace("swift-","")});
    });
    watchdog->start();


    BalancesKeeper * bals_keeper = new BalancesKeeper();
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, bals_keeper, &BalancesKeeper::onWampSession );
    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    wamp_client->startClient();

    return a.exec();
}
