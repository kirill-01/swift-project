#include <QCoreApplication>
#include "swiftapiclientkraken.h"
#include "swiftapiparserkraken.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QLockFile>
#include <QDir>
#include <QSettings>
#include <swiftbot.h>
#include <QThread>

#define APP_DIR "/opt/swift-bot"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-kraken");
    QCoreApplication::setApplicationVersion("1.0.451");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("Kraken API client module for swift bot system");
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
    // Wamp client
    wamp_client.reset(new WampClient(
                   SwiftBot::appParam(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
                   SwiftBot::appParam(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
                   SwiftBot::appParam(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
                   SwiftBot::appParam(SETTINGS_NAME_WAMP_DEBUG, false).toBool() ));

    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    SwiftApiClientKraken * api_client = new SwiftApiClientKraken(nullptr);
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, api_client, &SwiftApiClient::onWampSession);
    SwiftApiParserKraken * api_parser = new SwiftApiParserKraken();


    QObject::connect( api_client, &SwiftApiClient::parseApiResponse, api_parser, &SwiftApiParser::registerResponse);
    QObject::connect( api_parser,  &SwiftApiParser::resultParsed, api_client, &SwiftApiClient::onApiResponseParsed);

    QThread parserThread;
    parserThread.setObjectName("parserThread");
    api_parser->moveToThread( & parserThread );

    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected,[&a](){
        qWarning() << "Exiting";
        a.quit();
    } );
    QObject::connect( &parserThread, &QThread::started, wamp_client.data(), &WampClient::startClient );
    parserThread.start();


    return a.exec();
}
