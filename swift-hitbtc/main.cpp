#include <QCoreApplication>
#include "swiftapiclienthitbtc.h"
#include "swiftapiparserhitbtc.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "../swift-corelib/wampclient.h"
#include <QLockFile>
#include <QDir>
#include <QSettings>


#define APP_DIR "/opt/swift-bot"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-hitbtc");
    QCoreApplication::setApplicationVersion("1.0.275");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("Hitbtc API client module for swift bot system");
    parser.addHelpOption();
    parser.addVersionOption();

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    static QString app_dir( parser.value( targetDirectoryOption ) );

    // Get stored settings
    QSettings settings(app_dir+"/settings.ini", QSettings::IniFormat );


    // MySQL db
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName( settings.value(SETTINGS_NAME_MYSQL_HOST ).toString() );
    db.setPort( settings.value(SETTINGS_NAME_MYSQL_PORT ).toInt() );
    db.setUserName( settings.value(SETTINGS_NAME_MYSQL_USER).toString() );
    db.setPassword( settings.value(SETTINGS_NAME_MYSQL_PASSWORD ).toString() );
    db.setDatabaseName( settings.value(SETTINGS_NAME_MYSQL_DBNAME ).toString() );

    if ( !db.open() ) {
        qWarning() << "MySQL database error: ";
        qWarning() << db.lastError().text();
        return 1;
    }

    // Wamp client
    WampClient * wamp_client = new WampClient(
                   settings.value(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
                   settings.value(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
                   settings.value(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
                   settings.value(SETTINGS_NAME_WAMP_DEBUG, false).toBool() );

    QObject::connect( wamp_client, &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    SwiftApiClientHitbtc * api_client = new SwiftApiClientHitbtc(nullptr);
    QObject::connect( wamp_client, &WampClient::clientConnected, api_client, &SwiftApiClient::onWampSession);
    SwiftApiParserHitbtc * api_parser = new SwiftApiParserHitbtc();


    QObject::connect( api_client, &SwiftApiClient::parseApiResponse, api_parser, &SwiftApiParser::registerResponse);
    QObject::connect( api_parser,  &SwiftApiParser::resultParsed, api_client, &SwiftApiClient::onApiResponseParsed);

    QObject::connect( wamp_client, &WampClient::clientdiconnected,[&a](){
        qWarning() << "Exiting";
        a.quit();
    } );
    wamp_client->startClient();

    return a.exec();
}
