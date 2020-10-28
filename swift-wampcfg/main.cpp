#include <QCoreApplication>
#ifdef SWIFT_ASYNC_WORKER
#include <QThread>
#include <templateworker.h>
#endif

#include <iostream>

#include "module_definitions.h"
#include <swiftbot.h>
#include <QDirIterator>

#include "crossbarconfig.h"
#include <QCryptographicHash>

QStringList arrayToStringList( const QJsonArray & src ) {
    QStringList ret;
    for ( auto it = src.begin(); it != src.end();it++ ) {
        ret.push_back( it->toString() );
    }
    return ret;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName( SWIFT_MODULE_NAME );
    QCoreApplication::setApplicationVersion( SWIFT_MODULE_VERSION );
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

    CrossbarConfig config;
    config.loadConfig( "/opt/swift-bot/crossbar/config.json" );
    QStringList _modules_files;
    QDirIterator it("/opt/swift-bot/modules", QStringList() << "wamp.json", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        _modules_files << it.next();
    }
    QString _docs_api_index;
    for( auto it = _modules_files.begin(); it != _modules_files.end(); it++ ) {
        QFile _module_wamp_file( *it );
        if ( _module_wamp_file.open( QFile::ReadOnly ) ) {
            const QString fname( *it );
            QStringList fparts( fname.split("/") );
            const QString modulename( fparts.at( fparts.count()-2 ));
            qWarning() << "Processing module: " << modulename;
            QByteArray filedata( _module_wamp_file.readAll().constData()  );
            QJsonObject j_config( QJsonDocument::fromJson( filedata ).object() );

            QString _api_description("# API description for module "+modulename+"\n");
            _docs_api_index+="["+modulename+"]("+modulename+".md)";
            QJsonArray j_provided_feeds( j_config.value("provide").toObject().value("feeds").toArray() );
            if ( !j_provided_feeds.isEmpty() ) {
                _api_description += "## Feeds\n";
                for( auto it = j_provided_feeds.begin(); it != j_provided_feeds.end(); it++ ) {
                    QJsonObject j_feed( it->toObject() );
                    _api_description += "### "+ j_feed.value("uri").toString() + "\n";
                    _api_description += j_feed.value("description").toString() + "\n";
                    _api_description += "#### Message \n";
                    _api_description += "```json\n"+QJsonDocument( j_feed.value("message").toArray()).toJson( QJsonDocument::Indented)+"\n```";
                }
            }

            QJsonArray j_provided_methods( j_config.value("provide").toObject().value("methods").toArray() );

            if ( !j_provided_methods.isEmpty() ) {
                _api_description += "## Methods\n";
                for( auto it = j_provided_methods.begin(); it != j_provided_methods.end(); it++ ) {
                    QJsonObject j_rpc( it->toObject() );
                    _api_description += "### "+ j_rpc.value("uri").toString() + "\n";
                    _api_description += j_rpc.value("description").toString() + "\n";
                    _api_description += "#### Arguments \n";
                    _api_description += "```json\n"+QJsonDocument( j_rpc.value("arguments").toArray()).toJson( QJsonDocument::Indented)+"\n```";
                    _api_description += "#### Response \n";
                    _api_description += "```json\n"+QJsonDocument( j_rpc.value("response").toArray()).toJson( QJsonDocument::Indented)+"\n```";
                }
            }

            QFile docs_file("/opt/swift-bot/docs/"+modulename+".md");
            if ( docs_file.open( QFile::ReadWrite ) ) {
                docs_file.resize(0);
                docs_file.write( _api_description.toUtf8() );
                docs_file.close();
            }

            _module_wamp_file.close();

            const QString authid( modulename );
            const QString authpass( QCryptographicHash::hash( QString(authid + QString::number( QDateTime::currentSecsSinceEpoch() )).toUtf8(), QCryptographicHash::Sha1 ).toHex().toUpper().left(12) );
            const QString modulerole( "m"+modulename );

            QString modulesettingsfile( "/opt/swift-bot/modules/"+modulename+"/"+modulename+".ini" );
            if ( !QFile::exists( modulesettingsfile ) ) {
                  QFile sf( modulesettingsfile );
                  sf.open( QFile::ReadWrite );
                  sf.resize(0);
                  sf.close();
            }

            QSettings modulesettings( modulesettingsfile, QSettings::IniFormat );
            modulesettings.setValue(SETTINGS_NAME_WAMP_USER, authid );
            modulesettings.setValue(SETTINGS_NAME_WAMP_PASS, authpass );
            modulesettings.setValue(SETTINGS_NAME_WAMP_ROLE, modulerole );

            QJsonArray permissions_default(
                        CrossbarConfig::getPermissions(
                            arrayToStringList( j_config.value("default").toObject().value("permissions").toObject().value("call").toArray()),
                        arrayToStringList( j_config.value("default").toObject().value("permissions").toObject().value("register").toArray()),
                        arrayToStringList( j_config.value("default").toObject().value("permissions").toObject().value("publish").toArray()),
                        arrayToStringList( j_config.value("default").toObject().value("permissions").toObject().value("subscribe").toArray())
                        ));

            QJsonArray _calls;
            QStringList exchanges( QString("bittrex bitfinex binance hitbtc zb kucoin huobi kraken idcm livecoin").split(" ") );
            QJsonArray call_enabled( j_config.value("module").toObject().value("call").toArray() );
            for( auto it = call_enabled.begin(); it != call_enabled.end(); it++ ) {
                QString uri = it->toString();
                if ( uri.contains("*") ) {
                    for( auto ie = exchanges.begin(); ie != exchanges.end(); ie++ ) {
                        QString newuri( uri );
                        _calls.push_back( newuri.replace("*", *ie ));
                    }
                } else {
                    _calls.push_back( uri );
                }

            }

            QJsonArray permissions_module(
                        CrossbarConfig::getPermissions(
                            arrayToStringList( _calls ),
                        arrayToStringList( j_config.value("module").toObject().value("register").toArray()  ),
                        arrayToStringList( j_config.value("module").toObject().value("publish").toArray()  ),
                        arrayToStringList( j_config.value("module").toObject().value("subscribe").toArray() )
                        ));

            QJsonArray permissions_result( permissions_default );
            for( auto it = permissions_module.begin(); it != permissions_module.end();it++ ) {
                permissions_result.push_back( it->toObject() );
            }
            config.addUser( authid, QJsonObject({{"ticket",authpass},{"role",modulerole}}) );


            config.addRole( QJsonObject({{"name",modulerole},{"permissions", permissions_result }}) );

        }
        QFile docs_file("/opt/swift-bot/docs/api.md");
        if ( docs_file.open( QFile::ReadWrite ) ) {
            docs_file.resize(0);
            docs_file.write( _docs_api_index.toUtf8() );
            docs_file.close();
        }
    }
    config.addUser( "pamm_caller", QJsonObject({{"ticket","pamm_caller"},{"role","pamm_ext"}}) );
    QJsonArray j_perms;
    QStringList calls({"swift.pamm.node","swift.pamm.api.user","swift.pamm.api.invest"});
    for( auto it = calls.begin(); it != calls.end(); it++ ) {
        QJsonObject j_rule( CrossbarConfig::getRuleObj( *it ) );
        QJsonObject j_allow( j_rule.value("allow").toObject() );
        j_allow["call"] = true;
        j_rule["allow"] = j_allow;
        j_perms.push_back( j_rule );
    }
    config.addRole( QJsonObject({{"name","pamm_ext"},{"permissions", j_perms }}) );
    config.saveConfig( "/opt/swift-bot/crossbar/config.json" );
    qWarning() << "Config generated";
    a.exit(0);
}
