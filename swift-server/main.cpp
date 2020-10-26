#include <QCoreApplication>
#include "modulesmanager.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSettings>
#include <QDebug>
#include <QList>
#include "procwrapper.h"
#include <QLockFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "logger.h"
#include <QQueue>
#include <QFile>
#include <QThread>

#include "hostinfocollector.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <swiftbot.h>
#include <QDirIterator>

struct ApiMethodStatus {
    quint64 success_count;
    quint64 failed_count;
    QDateTime last_result_at;
    void addSuccess() {
        success_count++;
        last_result_at = QDateTime::currentDateTime();
    }
    void addError() {
        failed_count++;
        last_result_at = QDateTime::currentDateTime();
    }
};


void collectUserConfig() {
    QStringList _modules_files;
    QDirIterator it("/opt/swift-bot", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString ne = it.next();
        if ( ne != "settings_user.ini" ) {
            _modules_files << it.next();
        }
    }
    QSettings summary_settings("/opt/swift-bot/settings_user.ini", QSettings::IniFormat );
    if ( !_modules_files.isEmpty() ) {
        for( auto it = _modules_files.begin(); it != _modules_files.end(); it++ ) {
            QSettings group_settings( *it, QSettings::IniFormat );
            const QString modulename( it->split("/").last().replace(".ini","") );
            summary_settings.beginGroup( modulename );
            QStringList keys( group_settings.allKeys() );
            for( auto ii = keys.begin(); ii != keys.end(); ii++ ) {
                summary_settings.setValue( *ii, group_settings.value( *ii ) );
            }
            summary_settings.endGroup();
        }
    }

    qWarning() << "Aggregated config available at path:  /opt/swift-bot/settings_user.ini\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-server");
    QCoreApplication::setApplicationVersion("1.0.275");

    collectUserConfig();

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    if ( QFile::exists( "/opt/swift-bot/settings.dist" ) ) {
        QSettings dist_settings( "/opt/swift-bot/settings.dist", QSettings::IniFormat );
        QSettings current_settings( "/opt/swift-bot/settings.ini", QSettings::IniFormat );
        const QStringList dist_keys( dist_settings.allKeys() );
        for( auto it = dist_keys.begin(); it != dist_keys.end(); it++ ) {
            if ( !current_settings.contains( *it ) ) {
                current_settings.setValue( *it, dist_settings.value( *it ) );
            }
        }
        current_settings.sync();
        QFile::remove( "/opt/swift-bot/settings.dist" );
    }

    // App commandline options

    std::cout << "SwiftBot  v." << QCoreApplication::applicationVersion().toStdString() << std::endl;
    std::cout << "Running on host: " << SwiftBot::getHostIpAddress().toStdString() << std::endl;
    QCommandLineParser parser;
    parser.setApplicationDescription("Swift bot server help");
    parser.addHelpOption();
    parser.addVersionOption();


    static QMap<QString, QSharedPointer<ProcWrapper>> _running_modules;
    static QMap<QString,QString> _exchanges_statuses;

    static QStringList exchange_modules;

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    static QString app_dir( parser.value( targetDirectoryOption ) );

    static QMap<QString, ApiMethodStatus> _api_methods_results;

    // Get list of installed modules
    static QList<ModuleInfo> modules( ModulesManager::getAvailableModules( app_dir ) );

    // Get stored settings
    QSettings settings(app_dir+"/settings.ini", QSettings::IniFormat );



    HostInfoCollector * hinfo = new HostInfoCollector();
    QTimer::singleShot( SwiftCore::getSettings()->value("hostinfo_interval", 5000 ).toUInt(), hinfo, &HostInfoCollector::collectInfo );

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


    // Apply MySQL migrations
    QStringList filters;
    filters << "*.sql";
    QDir sql_dir( app_dir+"/sql" );
    sql_dir.setNameFilters(filters);
    const QStringList sql_files( sql_dir.entryList() );
    if ( !sql_files.isEmpty() ) {
        for( auto it = sql_files.begin(); it != sql_files.end(); it++ ) {
            QFile f( QString(app_dir+"/sql"+"/"+*it ) );
            if ( f.open( QFile::ReadOnly ) ) {
                qWarning() << "Executing migration from file: "+*it;
                const QByteArray file_data( f.readAll().constData() );
                QSqlQuery q( file_data );
                if( !q.exec() ) {
                    qWarning() << "Migration error: " << q.lastError().text();
                }
            }
        }
    }


    if ( settings.value( SETTINGS_NAME_SECURE_START, true ).toBool() ) {
        QProcess proc;
        proc.setProgram("/opt/swift-bot/bin/swift-wampcfg" );
        proc.start();
        if ( !proc.waitForFinished() ) {
            qWarning() << "Error while recreating security rules!";
            return 1;
        }
    }


    // Wamp client
    WampClient * wamp_client = new WampClient(
                   settings.value(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
                   settings.value(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
                   settings.value(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
                    settings.value(SETTINGS_NAME_WAMP_DEBUG, false).toBool() );

    static QProcess * crossbar_process = new QProcess();

    if ( !QFile::exists( settings.value("crossbar_binary", "/home/kkuznetsov/.local/bin/crossbar").toString() )) {
        qWarning() << "Crossbar.io binary is not found at path :("+settings.value("crossbar_binary", "/home/kkuznetsov/.local/bin/crossbar").toString()+").\n\nPLease install it or change settings value";
        return 1;
    }

    crossbar_process->setProgram( settings.value("crossbar_binary", "/home/kkuznetsov/.local/bin/crossbar").toString() );
    crossbar_process->setArguments({"start","--cbdir=/opt/swift-bot/crossbar","--logtofile","--logdir=/opt/swift-bot/crossbar"});

    QObject::connect( crossbar_process, &QProcess::stateChanged, [&]( QProcess::ProcessState state ) {
       if ( state == QProcess::NotRunning ) {
           qWarning() << "Crossbar router subprocess is going down!";
          // qApp->exit(1);
       } else if ( state == QProcess::Running ) {
            QTimer::singleShot( 2500, wamp_client, &WampClient::startClient );
       }
    });

    QObject::connect( &a, &QCoreApplication::aboutToQuit, crossbar_process, &QProcess::kill );

    Logger * logger = new Logger();
    QObject::connect( wamp_client, &WampClient::clientConnected,logger, &Logger::onWampSession );

    QObject::connect( wamp_client, &WampClient::clientConnected,[&a]( Wamp::Session * session ) {
        // Controling modules
        session->provide( RPC_MODULE_COMMAND, [&a](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            const QString module( v.at(0).toString() );
            const QString cmd( v.at(1).toString() );
            if ( cmd == "start" ) {
                if ( _running_modules.contains( module ) ) {
                    _running_modules[ module ].data()->stop();
                }
                for( auto it = modules.begin(); it != modules.end(); it++ ) {
                    if ( it->name == module ) {
                        _running_modules[ module]= QSharedPointer<ProcWrapper>( new ProcWrapper(it->binary, {})) ;
                        QObject::connect( &a, &QCoreApplication::aboutToQuit, _running_modules.last().data(), &ProcWrapper::stop );
                    }
                }
            } else if ( cmd == "stop" ) {
                if ( _running_modules.contains( module ) ) {
                    _running_modules[ module ].data()->stop();
                }
            } else if ( cmd == "restart" ) {
                if ( _running_modules.contains( module ) ) {
                    _running_modules[ module ].data()->stop();
                }
                for( auto it = modules.begin(); it != modules.end(); it++ ) {
                    if ( it->name == module ) {
                        _running_modules[ module]= QSharedPointer<ProcWrapper>( new ProcWrapper(it->binary, {})) ;
                        QObject::connect( &a, &QCoreApplication::aboutToQuit, _running_modules.last().data(), &ProcWrapper::stop );
                    }
                }
            } else if ( cmd == "enable" ) {
                QSettings sett( QString( app_dir + "/modules/"+module+"/"+module+".ini" ), QSettings::IniFormat );
                sett.setValue("is_enabled", true );
            } else if ( cmd == "disable" ) {
                QSettings sett( QString( app_dir + "/modules/"+module+"/"+module+".ini"), QSettings::IniFormat );
                sett.setValue("is_enabled", false );
            } else {
                return "ERROR: Available commands is [start,stop,restart,enable,disable]";
            }
            return "OK";
        });


        // Execute shell command on host
        session->provide( RPC_SERVER_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            QProcess::startDetached( v.at(0).toString() );
            return "OK";
        });


        // Update API methods stats
        session->provide( RPC_API_METHODS_STATE, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            const QString methodname( v.at(0).toString() );
            const bool is_success( v.at(1).toBool() );
            if ( is_success ) {
                _api_methods_results[methodname].addSuccess();
            } else {
                _api_methods_results[methodname].addError();
            }
            return true;
        });

        // Get summary info of API methods stats
        session->provide( RPC_API_METHODS_STATUS, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m);
            Q_UNUSED(v);
            QJsonObject j_ret;
            for( auto it = _api_methods_results.begin(); it != _api_methods_results.end(); it++ ) {
                QJsonObject ji;
                ji["errors"] = QString::number( it.value().failed_count );
                ji["last_ts"] = QString::number( it.value().last_result_at.toSecsSinceEpoch() );
                ji["succees"] = QString::number( it.value().success_count );
                j_ret[ it.key() ] = ji;
            }
            const QString ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact) );
            return ret;
        });

        // List available modules
        session->provide( RPC_MODULES_LIST_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            Q_UNUSED(v)
            QJsonArray j_ret;
            const QList<ModuleInfo> modules( ModulesManager::getAvailableModules( app_dir ) );
            for( auto it = modules.begin(); it != modules.end(); it++ ) {
                QJsonObject j_obj( it->toJson() );
                j_obj["status"] = _running_modules.contains( it->name ) ? _running_modules[ it->name ]->getStatus() : "Disabled";
                j_ret.push_back( j_obj );
            }
            const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
            return r;
        });

        // Get available exchanges modules
        session->provide( RPC_EXCHANGES_LIST_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            Q_UNUSED(v)
            const QString r( exchange_modules.join(",") );
            return r;
        });

        // Get assets
        session->provide( RPC_ASSETS_GET, [](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            Q_UNUSED(v)
            const QJsonObject j_assets( SwiftCore::getAssets()->toJson() );
            const QString str_assets( QJsonDocument( j_assets ).toJson( QJsonDocument::Compact ) );
            return str_assets;
        });


        session->subscribe( FEED_EVENTS_ARBITRAGE, [=](const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            const QString event_name( v.at(0).toString() );
            if ( event_name == "WINDOW" ) {
                const QJsonObject j_window( QJsonDocument::fromJson( v.at(1).toString().toUtf8() ).object() );
                QSqlQuery q;

                q.prepare("INSERT IGNORE INTO arbitrage_events \
(`sell_pair_id`,`buy_pair_id`,`min_amount`,`min_profit`,`min_sell_rate`,\
`min_buy_rate`,`max_amount`,`max_profit`,`max_sell_rate`,`max_buy_rate`,`arbitrage_pair_id`,`roi`,`spread`) \
VALUES (:sell_pair_id,:buy_pair_id,:min_amount,:min_profit,:min_sell_rate,:min_buy_rate,:max_amount,:max_profit,:max_sell_rate,:max_buy_rate,:arbitrage_pair_id,:roi,:spread)");
                q.bindValue(":sell_pair_id", j_window.value("sell_market_id").toString().toUInt() );
                q.bindValue(":buy_pair_id",j_window.value("buy_market_id").toString().toUInt());
                q.bindValue(":min_amount",j_window.value("min_amount").toString().toDouble());
                q.bindValue(":min_profit",j_window.value("min_profit").toString().toDouble());
                q.bindValue(":min_sell_rate",j_window.value("min_sell_rate").toString().toDouble());
                q.bindValue(":min_buy_rate",j_window.value("min_buy_rate").toString().toDouble());
                q.bindValue(":max_amount",j_window.value("max_amount").toString().toDouble());
                q.bindValue(":max_profit",j_window.value("max_profit").toString().toDouble());
                q.bindValue(":max_sell_rate",j_window.value("max_sell_rate").toString().toDouble());
                q.bindValue(":max_buy_rate",j_window.value("max_buy_rate").toString().toDouble());
                q.bindValue(":arbitrage_pair_id",j_window.value("arbitrage_pair_id").toString().toUInt());
                q.bindValue(":roi",j_window.value("roi").toString().toDouble());
                q.bindValue(":spread",j_window.value("spread").toString().toDouble());
                if ( !q.exec() ) {
                    qWarning() << q.lastError().text();
                }
                q.finish();
            } else if ( event_name == "STATS") {
                // Arbitrage windows stats - ?
                // std::cout << QString( QJsonDocument::fromJson( v.at(1).toString().toUtf8() ).toJson( QJsonDocument::Indented ) ).toStdString();
            }
        });

        // Control api clients credentials
        session->provide( RPC_APIMODULES_REPORTS,[]( const QVariantList& v, const QVariantMap&m) {
            Q_UNUSED(m)
            const QString module_name( v.at(0).toString() );
            const QString module_status_string( v.at(1).toString() );
            _exchanges_statuses[ module_name ] = module_status_string;
            return 1;
        });

        qWarning() << "Total modules: " << modules.count();
        for( auto it = modules.begin(); it != modules.end(); it++ ) {
            if ( it->is_enabled && !it->is_exchange ) {
                _running_modules.insert( it->name, QSharedPointer<ProcWrapper>( new ProcWrapper(it->binary, {})) );
                qWarning() << "Starting system module: " << it->name;
            }
        }

        qWarning().noquote() << "----";
        QThread::msleep(4500);
        // Start enabled modules
        for( auto it = modules.begin(); it != modules.end(); it++ ) {
            if ( it->is_enabled && it->is_exchange ) {
                _running_modules.insert( it->name, QSharedPointer<ProcWrapper>( new ProcWrapper(it->binary, {})) );
                exchange_modules.push_back( it->name );
                qWarning() << "Starting exchange module: " << it->name;
            }
            QThread::msleep(150);
        }

        for ( auto it = _running_modules.begin(); it != _running_modules.end(); it++ ) {
            QObject::connect( &a, &QCoreApplication::aboutToQuit, it.value().data(), &ProcWrapper::stop );
        }
    });


    crossbar_process->start();
    return a.exec();
}
