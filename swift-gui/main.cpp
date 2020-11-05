#include <QApplication>
#include <QDirIterator>

#include "mainwindow.h"

#include "module_definitions.h"
#include <swiftbot.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName( "swift-gui" );
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
#endif

    return a.exec();
}
