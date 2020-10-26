#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "../swift-corelib/wampclient.h"
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QLockFile>
#include <QDir>
#include "consolereader.h"
#include "resultwaiter.h"

#include <iostream>

#define APP_DIR "/opt/swift-bot"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-console");
    QCoreApplication::setApplicationVersion("1.0.255");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options

    QCommandLineParser parser;
    parser.setApplicationDescription("Swift console");
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

    static QString current_command("");

    // Wamp client
    WampClient * wamp_client = new WampClient(
                   settings.value(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
                   settings.value(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
                   settings.value(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
                   settings.value(SETTINGS_NAME_WAMP_DEBUG, false).toBool() );
    static ResultWaiter * waiter = new ResultWaiter();
    QObject::connect( wamp_client, &WampClient::clientConnected, waiter, &ResultWaiter::onWampSession );
    QObject::connect( waiter, &ResultWaiter::resultReceived, []( const QString& res) {
       qWarning() << "Response: \n";
       const QJsonDocument jdoc( QJsonDocument::fromJson( res.toUtf8() ) );
       if ( !jdoc.isEmpty() && !jdoc.isNull() ) {
           std::cout << jdoc.toJson( QJsonDocument::Indented ).toStdString() << std::endl;
           std::cout << "> " << std::flush;
       } else {
           std::cout << res.toStdString() << std::endl;
           std::cout << "> " << std::flush;
       }
    });
    static ConsoleReader *consoleReader = new ConsoleReader();

    QObject::connect( wamp_client, &WampClient::clientConnected,[]( Wamp::Session * session ) {
        Q_UNUSED(session)
        qWarning() << "Connected. Enter the command:";
        std::cout << "> ";
        QObject::connect(consoleReader, &ConsoleReader::KeyPressed, waiter, &ResultWaiter::onKeyPress );
        consoleReader->start();

    });

    wamp_client->startClient();

    return a.exec();
}
