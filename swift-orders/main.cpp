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

#include "orderskeeper.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-orders");
    QCoreApplication::setApplicationVersion("1.0.493");

    // Allow only one instance per host
    QLockFile lockFile( QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }
    std::cout << "SwiftBot  v." << QCoreApplication::applicationVersion().toStdString() << std::endl;
    std::cout << "Running on host: " << SwiftBot::getHostIpAddress().toStdString() << std::endl;


    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("SwiftBot orders module");
    parser.addHelpOption();
    parser.addVersionOption();

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);


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

    SwiftBot::initWampClient();
    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        wamp_client->publish( FEED_WATCHDOG, { QCoreApplication::applicationName().replace("swift-","")});
    });
    watchdog->start();

    OrdersKeeper * orders_keeper = new OrdersKeeper();
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, orders_keeper, &OrdersKeeper::onWampSession );
    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    wamp_client->startClient();

    return a.exec();
}
