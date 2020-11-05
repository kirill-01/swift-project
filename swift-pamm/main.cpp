#include <QCoreApplication>
#ifdef SWIFT_ASYNC_WORKER
#include <QThread>
#include "templateworker.h"
#endif

#include "module_definitions.h"
#include <swiftbot.h>
#include "swift_ext.h"
#include "node.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName( SWIFT_MODULE_NAME );
    QCoreApplication::setApplicationVersion( SWIFT_MODULE_VERSION );
    SwiftBot::initWampClient();
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
    wamp_client->provide("swift.pamm.node", [](const QVariantList&v, const QVariantMap&m ){
        Q_UNUSED(m)
        Q_UNUSED(v);
        Node node;
        // Status
        QJsonObject node_info = node.toJson();
        const QString j( QJsonDocument( node_info ).toJson( QJsonDocument::Compact ) );
        return j;
    });

    wamp_client->provide("swift.pamm.api.user", [](const QVariantList&v, const QVariantMap&m ){

        // register,username,password
        const QString cmd( v.at(0).toString() );

        if ( cmd == "register") {
            SwiftBot::User::create( v.at(1).toString(), v.at(2).toString() );
            SwiftBot::User user( v.at(1).toString() );
            if ( user.id > 0 ) {
                QTimer::singleShot( 100, [](){
                    QSqlQuery q;
                    const QString address_btc = SwiftBot::method("swift.address.generate", {1} ).toString();
                    const QString address_eth = SwiftBot::method("swift.address.generate", {2} ).toString();
                });
                return QString( "{\"success\":true, \"id\":\""+QString::number( user.id )+"\"}" );
            } else {
                return QString( "{\"success\":false}" );
            }
        }
        // login,username,password
        else if ( cmd == "login") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        }
        // info,username,password
        else if ( cmd == "info") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    QJsonObject j_ret;
                    j_ret["success"] = true;
                    QJsonArray j_addrs;
                    QSqlQuery q("SELECT a.*, b.address as address FROM pamm_addresses a LEFT JOIN cypher_addresses b ON a.cypher_id=b.id WHERE a.user_id="+QString::number(user.id ) );
                    if ( q.exec() ) {
                        while( q.next() ) {
                            QJsonObject j_address;
                            j_address["id"] = QString::number( q.value("cypher_id").toUInt() );
                            j_address["coin_id"] = QString::number( q.value("coin_id").toUInt() );
                            j_address["address"] = q.value("address").toString();
                            j_addrs.push_back( j_address );
                        }
                    }
                    j_ret["addresses"] = j_addrs;
                    j_ret["total_balance"] = QString::number(0,'f',8);
                    const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
                    return r;
                }
            }
            return QString( "{\"success\":false}" );
        } else if ( cmd == "invests") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    QJsonObject j_ret;
                    j_ret["success"] = true;
                    j_ret["items"] = QJsonArray();
                    const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
                    return r;
                }
            }
            return QString( "{\"success\":false}" );
        } else if ( cmd == "active") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    QJsonObject j_ret;
                    j_ret["success"] = true;
                    j_ret["items"] = QJsonArray();
                    const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
                    return r;

                }
            }
            return QString( "{\"success\":false}" );
        }
        // deposit,wounderer,dedicated,1
        else if ( cmd == "deposit") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    const quint32 coin_id = v.at(3).toUInt();
                    QSqlQuery q("SELECT * FROM pamm_addresses where coin_id="+QString::number(coin_id)+" AND user_id="+QString::number( user.id ) );
                    if ( q.exec() ) {
                        if ( q.next() ) {

                        } else {
                            if ( q.exec("SELECT * FROM cypher_addresses WHERE id NOT IN (SELECT cypher_id FROM pamm_addresses WHERE coin_id="+QString::number(coin_id)+")") ){
                                if ( q.next() ) {
                                    const QString address = q.value("address").toString();
                                    const quint32 id = q.value("id").toUInt();
                                    QSqlQuery qq("INSERT INTO pamm_addresses (user_id, coin_id, cypher_id) VALUES ("+QString::number(user.id)+","+QString::number(coin_id)+","+QString::number(id)+")");
                                    if ( !qq.exec() ) {
                                        qWarning() << qq.lastError().text();
                                    }
                                    QJsonObject j_ret;
                                    j_ret["success"] = true;
                                    j_ret["address"] = address;
                                    const QString r( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
                                    return r;
                                }
                            }
                        }
                    }


                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        } else if ( cmd == "withdraw") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        }

        Q_UNUSED(m)
        return QString( "{\"success\":false}" );
    });

    wamp_client->provide("swift.pamm.api.invest", [](const QVariantList&v, const QVariantMap&m ){
        // Start
        // Info
        // Cancel
        const QString cmd( v.at(0).toString() );
        if ( cmd == "start") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        } else if ( cmd == "info") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        } else if ( cmd == "cancel") {
            SwiftBot::User user( v.at(1).toString() );
            if( user.id > 0 ) {
                if ( user.auth( v.at(2).toString() ) ) {
                    return QString( "{\"success\":true}" );
                }
            }
            return QString( "{\"success\":false}" );
        }
        Q_UNUSED(m)
         return QString( "{\"success\":false}" );
    });
    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        wamp_client->publish( FEED_WATCHDOG, { QCoreApplication::applicationName().replace("swift-","")});
    });
    watchdog->start();

    wamp_client->startClient();

    return a.exec();
}
