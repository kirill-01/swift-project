#include <QCoreApplication>
#include "swiftapiclientzb.h"
#include "swiftapiparserzb.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QThread>
#include <QLockFile>
#include <QDir>
#include <QSettings>
#include <swiftbot.h>


#define APP_DIR "/opt/swift-bot"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-zb");
    QCoreApplication::setApplicationVersion("1.0.395");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

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

    SwiftApiClientZB * api_client = new SwiftApiClientZB(nullptr);
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, api_client, &SwiftApiClient::onWampSession);
    SwiftApiParserZB * api_parser = new SwiftApiParserZB();


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
