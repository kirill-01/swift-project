#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "../swift-corelib/wampclient.h"
#include "../swift-corelib/swiftcore.h"

#include <QLockFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>

#include "telegramapi.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-telegram");
    QCoreApplication::setApplicationVersion("1.0.275");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    if ( QFile::exists( "/opt/swift-bot/modules/telegram/telegram.dist" ) ) {
        QSettings dist_settings( "/opt/swift-bot/modules/telegram/telegram.dist", QSettings::IniFormat );
        QSettings current_settings( "/opt/swift-bot/modules/telegram/telegram.ini", QSettings::IniFormat );
        const QStringList dist_keys( dist_settings.allKeys() );
        for( auto it = dist_keys.begin(); it != dist_keys.end(); it++ ) {
            if ( !current_settings.contains( *it ) ) {
                current_settings.setValue( *it, dist_settings.value( *it ) );
            }
        }
        current_settings.sync();
        QFile::remove( "/opt/swift-bot/modules/telegram/telegram.dist" );
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("SwiftBot telegram module");
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

    TelegramApi * telegram_keeper = new TelegramApi();

    QObject::connect( &a, &QCoreApplication::aboutToQuit, telegram_keeper, &TelegramApi::beforeShutdown );
    QObject::connect( wamp_client, &WampClient::clientConnected, telegram_keeper, &TelegramApi::onWampSession );
    QObject::connect( wamp_client, &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    wamp_client->startClient();

    return a.exec();
}
