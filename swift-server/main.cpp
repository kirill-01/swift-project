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
#define WAMP_START_DELAY 3500

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

using namespace SwiftBot;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-server");
    QCoreApplication::setApplicationVersion("1.0.479");

    // Allow only one instance per host
    QLockFile lockFile( QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock( 100 ) ) {
       qWarning() << "Another instance is already running";
       return 1;
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
    static Modules modules( SwiftBot::Module::all() );
    //static QList<ModuleInfo> modules( ModulesManager::getAvailableModules( app_dir ) );

    HostInfoCollector * hinfo = new HostInfoCollector();
    QTimer::singleShot( SwiftCore::getSettings()->value("hostinfo_interval", 5000 ).toUInt(), hinfo, &HostInfoCollector::collectInfo );

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
        qApp->exit(1);
    }

    applySqlMigrations();

    if ( SwiftBot::appParam( SETTINGS_NAME_SECURE_START, true ).toBool() ) {
        qInfo() << "Secured start processing";
        QProcess proc;
        proc.setProgram("/opt/swift-bot/bin/swift-wampcfg" );
        proc.start();
        if ( !proc.waitForFinished(60000) ) {
            qWarning() << "Error while recreating security rules!";
            qApp->exit(1);
        }
        qInfo() << "WAMP configs is updated";
    }

    static QProcess * crossbar_process = new QProcess();

    if ( !QFile::exists( SwiftBot::appParam("crossbar_binary", "/usr/local/bin/crossbar").toString() )) {
        qWarning() << "Crossbar.io binary is not found at path :("+SwiftBot::appParam("crossbar_binary", "/usr/local/bin/crossbar").toString()+").\n\nPLease install it or change settings value";
        qApp->exit(1);
    }

    crossbar_process->setProgram( SwiftBot::appParam("crossbar_binary", "/usr/local/bin/crossbar").toString() );
    crossbar_process->setArguments({"start","--cbdir=/opt/swift-bot/crossbar","--logtofile","--logdir=/opt/swift-bot/crossbar"});

    QObject::connect( crossbar_process, &QProcess::stateChanged, [&]( QProcess::ProcessState state ) {
       if ( state == QProcess::NotRunning ) {
           qWarning() << "Crossbar router subprocess is going down!";
           qInfo() << "You may need to check, that crossbar is installed and worked:";
           qInfo() << "crossbar start --cbdir=/opt/swift-bot/crossbar";
           qApp->exit(1);
       } else if ( state == QProcess::Running ) {
           qInfo() << "Crossbar starting. Connecting after "+QString::number( quint32(WAMP_START_DELAY) / 1000, 'f', 2 )+" sec...";
           QTimer::singleShot( WAMP_START_DELAY, wamp_client.data(), &WampClient::startClient );
       }
    });

    QObject::connect( &a, &QCoreApplication::aboutToQuit, crossbar_process, &QProcess::kill );

    Logger logger;


    // Controling modules
    SwiftBot::provide( RPC_MODULE_COMMAND, [&a](const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        const QString module( v.at(0).toString() );
        const QString cmd( v.at(1).toString() );
        if ( cmd == "start" ) {
            if ( _running_modules.contains( module ) ) {
                _running_modules[ module ].data()->stop();
            }
            _running_modules[ module].reset( new ProcWrapper( modules[ module ].binary , {} ));
            QObject::connect( &a, &QCoreApplication::aboutToQuit, _running_modules[module].data(), &ProcWrapper::stop );

        } else if ( cmd == "stop" ) {
            if ( _running_modules.contains( module ) ) {
                _running_modules[ module ].data()->stop();
            }
        } else if ( cmd == "restart" ) {
            if ( _running_modules.contains( module ) ) {
                _running_modules[ module ].data()->stop();
            }
            _running_modules[ module].reset( new ProcWrapper( modules[ module ].binary , {} ));
            QObject::connect( &a, &QCoreApplication::aboutToQuit, _running_modules[module].data(), &ProcWrapper::stop );
        } else if ( cmd == "enable" ) {
            SwiftBot::Module::enable( module );
        } else if ( cmd == "disable" ) {
            SwiftBot::Module::disable( module );
        } else {
            return "ERROR: Available commands is [start,stop,restart,enable,disable]";
        }
        return "OK";
    });

    // Execute shell command on host
    SwiftBot::provide( RPC_SERVER_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        QProcess::startDetached( v.at(0).toString(), {} );
        return "OK";
    });


    // Update API methods stats
    SwiftBot::provide( RPC_API_METHODS_STATE, [](const QVariantList& v, const QVariantMap&m) {
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
    SwiftBot::provide( RPC_API_METHODS_STATUS, [](const QVariantList& v, const QVariantMap&m) {
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
    SwiftBot::provide( RPC_MODULES_LIST_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        QJsonArray j_ret;
        SwiftBot::Module::eachModule([&j_ret]( SwiftBot::Module module ){
            j_ret.push_back( module.toJson() );
        });
        const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
        return r;
    });

    SwiftBot::provide( RPC_HOST_STATUS, [hinfo](const QVariantList&v, const QVariantMap &m ) {
        QVariantList r;
        Q_UNUSED(m)
        Q_UNUSED(v)
        r.push_back( hinfo->getLastCpu() );
        r.push_back( hinfo->getLastRam() );
        return r;
    });

    // Get available exchanges modules
    SwiftBot::provide( RPC_EXCHANGES_LIST_COMMAND, [](const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        const QString r( exchange_modules.join(",") );
        return r;
    });

    // Get assets
    SwiftBot::provide( RPC_ASSETS_GET, [](const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        const QJsonObject j_assets( SwiftCore::getAssets()->toJson() );
        const QString str_assets( QJsonDocument( j_assets ).toJson( QJsonDocument::Compact ) );
        return str_assets;
    });

    wamp_client->subscribe( FEED_EVENTS_ARBITRAGE, [](const QVariantList& v, const QVariantMap&m) {
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
    SwiftBot::provide( RPC_APIMODULES_REPORTS,[]( const QVariantList& v, const QVariantMap&m) {
        Q_UNUSED(m)
        const QString module_name( v.at(0).toString() );
        const QString module_status_string( v.at(1).toString() );
        _exchanges_statuses[ module_name ] = module_status_string;
        return 1;
    });

    static QMap<QString, QDateTime> _watchdog;

    wamp_client->subscribe( FEED_WATCHDOG, [](const QVariantList& v, const QVariantMap&m){
        Q_UNUSED(m)
       _watchdog[ v.at(0).toString() ] = QDateTime::currentDateTime();
    });

    if ( SwiftBot::appParam("watchdog_enabled", false ).toBool() ) {
        QTimer * check_watchdog_timer = new QTimer();
        check_watchdog_timer->setInterval( SwiftBot::appParam("whatchdog_interval", 60 ).toUInt() * 1000 );

        QObject::connect( check_watchdog_timer, &QTimer::timeout,[&a](){
           for( auto it = _watchdog.begin(); it != _watchdog.end(); it++ ) {
               if ( QDateTime::currentSecsSinceEpoch() - it.value().toSecsSinceEpoch() >= SwiftBot::appParam("whatchdog_alive_time", 50 ).toUInt() ) {
                   if ( _running_modules.contains( it.key() ) ) {
                        _running_modules[ it.key() ].reset( new ProcWrapper( modules[ it.key() ].binary , {} ));
                        QObject::connect( &a, &QCoreApplication::aboutToQuit, _running_modules[it.key()].data(), &ProcWrapper::stop );
                   }
               }
           }

        });
        check_watchdog_timer->start();
    }
    wamp_client->onClientConnected([&a, &logger]( Wamp::Session * session ) {
        Q_UNUSED(session)
        logger.init();
        SwiftBot::addLog( "Total modules: " + QString::number( modules.count() ) );
        for( auto it = modules.begin(); it != modules.end(); it++ ) {
            Module module = it.value();
            if ( module.isEnabled() && !module.isExchange() ) {
                _running_modules[module.name].reset( new ProcWrapper(module.binary, {} ));
                SwiftBot::addLog( "Starting system module: " + module.name );
            }
        }
        qInfo().noquote() << "---- Exchanges clients ----";
        QThread::msleep(SwiftBot::appParam("before_exchanges_delay", 5000).toUInt() );
        // Start enabled modules
        for( auto it = modules.begin(); it != modules.end(); it++ ) {
            Module module = it.value();
            if ( module.isEnabled() && module.isExchange() ) {
                _running_modules[module.name].reset( new ProcWrapper(module.binary, {} ));
                exchange_modules.push_back( module.name );
                SwiftBot::addLog( "Starting exchange module: " + module.name );
            }
            QThread::msleep(100);
        }

        for ( auto it = _running_modules.begin(); it != _running_modules.end(); it++ ) {
            QObject::connect( &a, &QCoreApplication::aboutToQuit, it.value().data(), &ProcWrapper::stop );
        }
    });

    crossbar_process->start();
    return a.exec();
}
