#include <QApplication>
#include <QDirIterator>

#include "mainwindow.h"


#ifdef SWIFT_ASYNC_WORKER
#include <QThread>
#include <templateworker.h>
#endif

#include "module_definitions.h"
#include <swiftbot.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName( SWIFT_MODULE_NAME );
    QApplication::setApplicationVersion( SWIFT_MODULE_VERSION );
    MainWindow window;

    QStringList _modules_files;
    QDirIterator it("/opt/swift-bot/modules", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString ne = it.next();
        if ( ne != "settings_user.ini" ) {
            _modules_files << ne;
        }
    }
    QJsonArray _modules;

    if ( !_modules_files.isEmpty() ) {
        for( auto it = _modules_files.begin(); it != _modules_files.end(); it++ ) {
            QSettings group_settings( *it, QSettings::IniFormat );
            const QString modulename( it->split("/").last().replace(".ini","") );
            if ( modulename != "settings" || modulename != "" || modulename != "server" ) {
            _modules.push_back(QJsonObject({
                                   {"name",modulename},
                                   {"description",group_settings.value("description").toString()},
                                   {"binary",group_settings.value("binary").toString()},
                                   {"is_enabled",group_settings.value("is_enabled").toBool()},
                               }));
            }
        }
    }
    window.setModules( _modules );
    window.show();


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

#ifdef SWIFT_USE_WAMP_CLIENT
    // Wamp client. Allowing communications inside system between modules
    wamp_client.reset( new WampClient(
       SwiftBot::appSettings()->value(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
       SwiftBot::appSettings()->value(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
       SwiftBot::appSettings()->value(SETTINGS_NAME_WAMP_PORT,8081).toInt(),
       SwiftBot::appSettings()->value(SETTINGS_NAME_WAMP_DEBUG,false).toBool()
    ));

    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        SwiftBot::addLog( "WAMP client disconnected. Exiting." );
        a.quit();
    });

#ifdef SWIFT_ASYNC_WORKER
    QThread worker_thread;
    worker_thread.setObjectName("TemplateWorkerThread");
    TemplateWorker * worker = new TemplateWorker();
    QObject::connect( wamp_client.data(), &WampClient::clientJoined, worker, &TemplateWorker::onClientStarted, Qt::QueuedConnection );
    QObject::connect( worker, &TemplateWorker::publishFeed, wamp_client.data(), &WampClient::publishFeed, Qt::QueuedConnection );
    QObject::connect( worker, &TemplateWorker::callRpc, wamp_client.data(), &WampClient::callRpc, Qt::QueuedConnection );
    QObject::connect( worker, &TemplateWorker::subscribeFeed, wamp_client.data(), &WampClient::subscribeFeed, Qt::QueuedConnection );
    QObject::connect( wamp_client.data(), &WampClient::feedMessage, worker, &TemplateWorker::onFeedMessage, Qt::QueuedConnection );
    QObject::connect( wamp_client.data(), &WampClient::rpcResult, worker, &TemplateWorker::onRpcResult, Qt::QueuedConnection );
    //QObject::connect( &worker_thread, &QThread::started, wamp_client.data(), &WampClient::startClient );
    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, &worker_thread, &QThread::quit );

    worker->moveToThread( &worker_thread );
    worker_thread.start();
#else

    QObject::connect( wamp_client.data(), &WampClient::clientConnected, []( Wamp::Session * session ) {
        Q_UNUSED( session );
        SwiftBot::addLog( "WAMP Client connected!" );
    });
#endif
#endif
#ifdef SWIFT_USE_WAMP_CLIENT
#ifndef SWIFT_ASYNC_WORKER
    //wamp_client->startClient();
#endif
#endif


    return a.exec();
}
