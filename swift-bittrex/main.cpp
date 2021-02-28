#include <QCoreApplication>
#include "swiftapiclientbittrex.h"
#include "swiftapiparserbittrex.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <wampclient.h>
#include <QLockFile>
#include <QDir>
#include <QThread>


#define APP_DIR "/opt/swift-bot"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-bittrex");
    QCoreApplication::setApplicationVersion("1.0.493");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("Bittrex API client module for swift bot system");
    parser.addHelpOption();
    parser.addVersionOption();
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

    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    SwiftApiClientBittrex * api_client = new SwiftApiClientBittrex(nullptr);
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, api_client, &SwiftApiClient::onWampSession);
    SwiftApiParserBittrex * api_parser = new SwiftApiParserBittrex();


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
