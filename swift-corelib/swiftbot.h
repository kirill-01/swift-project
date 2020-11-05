#ifndef SWIFTBOT_H
#define SWIFTBOT_H

#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QMap>

#ifdef SWIFT_USE_WAMP_AUTH
#include <wampauth.h>
#endif

#include <wampclient.h>


#ifndef VARS_DEFINED
#define VARS_DEFINED
#define APP_DIR "/opt/swift-bot"
#define RPC_MODULE_COMMAND "swift.module.cmd"
#define RPC_SERVER_COMMAND "swift.server.cmd"
#define RPC_SERVER_LOGGER_LOGS "swift.logger.logs"
#define RPC_SERVER_LOGGER_ERRORS "swift.logger.errors"

#define RPC_APIMODULES_REPORTS "swift.apimodule.report"

#define RPC_MODULES_LIST_COMMAND "swift.module.list"
#define RPC_EXCHANGES_LIST_COMMAND "swift.module.exchanges"
#define RPC_ASSETS_GET "swift.assets.get"
#define RPC_API_METHODS_STATE "swift.methodstate"
#define RPC_API_METHODS_STATUS "swift.methodstate.summary"
#define RPC_RESERVE_ASYNC_UUID "swift.asyncuid"
#define RPC_ORDERBOOKS_CACHE "swift.orderbooks.cache"
#define RPC_CURRENT_RATES "swift.rates"
#define RPC_CURRENT_RATES_MSG "swift.rates.msg"

// Convert from one coin to another: swift.rate.convert 1,2,100
#define RPC_RATE_CONVERT "swift.rate.convert"

#define RPC_BALANCE_GET "swift.balance"
#define RPC_BALANCE_GET_TOTAL "swift.balance.total"
#define RPC_BALANCE_ALL "swift.balance.all"

#define WAMP_AUTH_COMPONENT "swift.system.authenticate"

#define RPC_BALANCE_DEPOSITS "swift.deposits"
#define RPC_BALANCE_WITHDRAWS "swift.withdraws"

#define RCP_TELEGRAM_NOTIFY "swift.telegram.msg"

#define RPC_HOST_STATUS "swift.host.status"

#define RPC_SETTINGS_GET_PARAM "swift.settings.get"
#define RPC_SETTINGS_SET_PARAM "swift.settings.set"

#define FEED_EVENTS_ORDERS "swift.orders.events"
#define FEED_EVENTS_BALANCES "swift.balance.events"
#define FEED_ASYNC_RESULTS "swift.api.asyncresults"
#define FEED_EVENTS_SYSTEM "swift.system.events"

// Arbitrage events
#define FEED_EVENTS_ARBITRAGE "swift.arbitrage.windows"

#define FEED_SETTINGS_SYSTEM "swift.system.settings"
#define FEED_WATCHDOG "swift.watchdog"
#define FEED_LOGS "swift.system.feed.logs"
#define FEED_ERRORS "swift.system.feed.errors"

#define FEED_ORDERBOOKS_SNAPSHOT "swift.orderbooks"

#define LOGS_DIRNAME "/opt/swift-bot/logs/"
#define ERRS_DIRNAME "/opt/swift-bot/logs/errors/"


#define EVENTS_NAME_WITHDRAWS_HISTORY "WITHDRAWS"
#define EVENTS_NAME_DEPOSITS_HISTORY "DEPOSITS"
#define EVENTS_NAME_WITHDRAWS_CREATED "NEWWITHDRAW"
#define EVENTS_NAME_BALANCES_UPDATE "UPDATE"

#define EVENTS_NAME_ORDERS_HISTORY "HISTORY"
#define EVENTS_NAME_ORDERS_ACTIVE "ACTIVE"

#define EVENTS_NAME_ORDER_UPDATED "UPDATED"
#define EVENTS_NAME_ORDER_PLACED "PLACED"
#define EVENTS_NAME_ORDER_CANCELED "CANCELED"
#define EVENTS_NAME_ORDER_ERROR "ERROR"
#define EVENTS_NAME_ORDER_COMPLETED "COMPLETED"

#define SETTINGS_NAME_MODULES_DEBUG "modules_debug"
#define SETTINGS_NAME_API_DEBUG "api_debug"

#define SETTINGS_NAME_DEFAULT_COINS "default_currencies"
#define SETTINGS_NAME_SKIP_PAIRS "exclude_pairs"

#define SETTINGS_NAME_WAMP_REALM "wamp_realm"
#define SETTINGS_NAME_WAMP_HOME "wamp_home"
#define SETTINGS_NAME_WAMP_PORT "wamp_port"
#define SETTINGS_NAME_WAMP_DEBUG "wamp_debug"
#define SETTINGS_NAME_WAMP_USER "wamp_user"
#define SETTINGS_NAME_WAMP_PASS "wamp_password"
#define SETTINGS_NAME_WAMP_ROLE "wamp_role"


#define SETTINGS_NAME_MYSQL_HOST "mysql_host"
#define SETTINGS_NAME_MYSQL_PORT "mysql_port"
#define SETTINGS_NAME_MYSQL_USER "mysql_user"
#define SETTINGS_NAME_MYSQL_PASSWORD "mysql_password"
#define SETTINGS_NAME_MYSQL_DBNAME "mysql_db"

#define SETTINGS_NAME_ORDERBOOKS_VALID_TIMER "orderbooks_valid_time"
#define SETTINGS_NAME_ORDERBOOKS_ADOPT_INTERVAL "adopt_orderbooks_interval"
#define SETTINGS_NAME_SECURE_START "startup_secured"


#define SETTINGS_NAME_EXCHANGE_CACHE_ORDERBOOKS "cache_orderbooks"
#define SETTINGS_NAME_EXCHANGE_CACHE_ORDERBOOKS_TIME "cache_orderbooks_time"

#define SETTINGS_NAME_EXCHANGE_DELAY_REQUESTS "requests_delay"
#define SETTINGS_NAME_EXCHANGE_DELAY_SAME_REQUESTS "requests_repeat_delay"

#endif

#ifndef SWIFT_MODULE_NAME
#define SWIFT_MODULE_NAME "template"
#define SWIFT_MODULE_VERSION "1.0.0"
#define SWIFT_MODULE_DESCRIPTION "SwiftBot simple module template"
#endif


#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QLockFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


#ifdef SWIFT_USE_MYSQL_DB
static QSqlDatabase db;
#endif

#ifdef SWIFT_USE_WAMP_AUTH
static QSharedPointer<WampAuth> wamp_client;
#else
static QSharedPointer<WampClient> wamp_client = nullptr;
#endif


namespace SwiftBot {

typedef std::function<QVariant(const QVariantList &v, const QVariantMap &m)> WampProcedure;
typedef std::function<void(const QVariantList &v, const QVariantMap &m)> WampSubscribe;

struct AssetsStruct {


    AssetsStruct & operator=( QJsonObject & j ) {
        const QStringList k( j.keys() );
        for( auto ki = k.begin(); ki != k.end(); ki++ ) {
            _data[ *ki ] = j.value( *ki ).toVariant();
        }
        return *this;
    }

    QVariant get( const QString& name, const QVariant & default_value = QVariant() ) const {
        return _data.contains( name ) ? _data.value( name ) : default_value;
    }
    QJsonObject toJson() const {
        QJsonObject j;
        for( auto it = _data.begin(); it != _data.end(); it++ ) {
            j[it.key()] = it.value().toString();
        }
        return j;
    }
    QHash<QString,QVariant> _data;
};

class Exchange;

#ifndef SWIFT_CURRENCY_ATTRIBUTES
#define SWIFT_CURRENCY_ATTRIBUTES "id,exchange_id,coin_id,is_enabled,name,withdraw_fee,min_withdraw"
#endif

class Currency;

class Coin;

struct Balance  : AssetsStruct {
    Balance( const QJsonObject & j ) {
        currency_id = j.value("currency_id").toString().toUInt();
        available = j.value("available").toString().toDouble();
        total = j.value("total").toString().toDouble();
        requested = j.value("requested").toString().toDouble();
    }
    Balance & operator=( const QJsonObject& j) {
        currency_id = j.value("currency_id").toString().toUInt();
        available = j.value("available").toString().toDouble();
        total = j.value("total").toString().toDouble();
        requested = j.value("requested").toString().toDouble();
        return *this;
    }
    quint32 currency_id;
    double available;
    double total;
    double requested;
};

typedef QMap<quint32,Balance> Balances;

struct Deposit : AssetsStruct {
    Deposit( const QJsonObject & j ) {
        currency_id = j.value("currency_id").toString().toUInt();
        exchange_id = j.value("exchange_id").toString().toUInt();
        amount =j.value("amount").toString().toDouble();
        ts =j.value("ts").toString().toULongLong();
        status =j.value("status").toString();
        from_address =j.value("from_address").toString();
        transaction_hash = j.value("transaction_hash").toString();
        remote_id = j.value("remote_id").toString();
    }
    Deposit & operator=( const QJsonObject& j) {
        currency_id = j.value("currency_id").toString().toUInt();
        exchange_id = j.value("exchange_id").toString().toUInt();
        amount =j.value("amount").toString().toDouble();
        ts =j.value("ts").toString().toULongLong();
        status =j.value("status").toString();
        from_address =j.value("from_address").toString();
        transaction_hash = j.value("transaction_hash").toString();
        remote_id = j.value("remote_id").toString();
        return *this;
    }
    quint32 id;
    quint32 currency_id;
    quint32 exchange_id;
    double amount;
    quint64 ts;
    QString status;
    QString from_address;
    QString transaction_hash;
    QString remote_id;
};

typedef QMap<quint32,Deposit> Deposits;


struct Withdraw : AssetsStruct{
    Withdraw( const QJsonObject & j ) {
        currency_id = j.value("currency_id").toString().toUInt();
        exchange_id = j.value("exchange_id").toString().toUInt();
        amount =j.value("amount").toString().toDouble();
        ts =j.value("ts").toString().toULongLong();
        status =j.value("status").toString();
        from_address =j.value("from_address").toString();
        transaction_hash = j.value("transaction_hash").toString();
        remote_id = j.value("remote_id").toString();
    }
    Withdraw & operator=( const QJsonObject& j) {
        currency_id = j.value("currency_id").toString().toUInt();
        exchange_id = j.value("exchange_id").toString().toUInt();
        amount =j.value("amount").toString().toDouble();
        ts =j.value("ts").toString().toULongLong();
        status =j.value("status").toString();
        from_address =j.value("from_address").toString();
        transaction_hash = j.value("transaction_hash").toString();
        remote_id = j.value("remote_id").toString();
        return *this;
    }
    quint32 id;
    quint32 currency_id;
    quint32 exchange_id;
    double amount;
    quint64 ts;
    QString status;
    QString from_address;
    QString transaction_hash;
    QString remote_id;
};

typedef QMap<quint32,Withdraw> Withdraws;

class Market;
class ArbitragePair;
class Order;

typedef QMap<quint32, Currency> Currencies;
typedef QMap<quint32, Coin> Coins;
typedef QMap<quint32, Market> Markets;
typedef QMap<quint32, Order> Orders;
typedef QMap<quint32, Exchange> Exchanges;
typedef QMap<quint32, ArbitragePair> ArbitragePairs;

static QSettings * moduleSettings(  const QString& m = QString(QCoreApplication::applicationName()).replace("swift-","")   );
static QSettings * appSettings();

static QVariant appParam( const QString& name, const QVariant & default_value = QVariant("") ) {
    return appSettings()->value( name, default_value );
}


static QVariant moduleParam( const QString& name, const QVariant & default_value = QVariant("") ) {
    return moduleSettings()->value( name, default_value );
}

static void initWampClient() {
    if ( !wamp_client || wamp_client == nullptr ) {
        wamp_client = QSharedPointer<WampClient>( new WampClient(SwiftBot::appParam(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
             SwiftBot::appParam(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
             SwiftBot::appParam(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
             SwiftBot::appParam(SETTINGS_NAME_WAMP_DEBUG, false).toBool())
        );
    }

}

static QVariant method( const QString& method_name, const QVariantList & arguments ) {

    if ( !wamp_client || wamp_client == nullptr ) {
        return 0;
    } else {
        if ( wamp_client->isConnected() ) {
            return wamp_client->getSession()->call( method_name, arguments );
        }
    }
    return 0;

}

static void provide( const QString& method_name, WampProcedure p ) {

    if ( !wamp_client || wamp_client == nullptr ) {
        initWampClient();
    }
    wamp_client->provide( method_name, p );
}

static void publish( const QString& method_name, const QVariantList& v ) {
    if ( !wamp_client || wamp_client == nullptr ) {
        initWampClient();
    }
    wamp_client->publish( method_name, v );
}

static void subscribe( const QString& method_name, WampSubscribe p ) {
    if ( !wamp_client || wamp_client == nullptr ) {
        initWampClient();
    }
    wamp_client->subscribe( method_name, p );
    if ( false ) {
        method( method_name, {} );
        provide( method_name, []( const QVariantList&, const QVariantMap& ){ return 1; });
        publish( method_name, QVariantList({}) );
    }
}


#ifdef SWIFT_USE_WAMP_CLIENT
static void addLog( const QString & message, const QString & log_group = "INFO" ) {
    if ( wamp_client->isConnected() ) {
        SwiftBot::method( RPC_SERVER_LOGGER_LOGS, { SWIFT_MODULE_NAME, log_group, message } );
        return;
    }

    qInfo().noquote() << log_group << message;
}

static void addError( const QString & message, const QString & log_group = "DANGER" ) {
    if ( wamp_client->isConnected() ) {
        SwiftBot::method( RPC_SERVER_LOGGER_ERRORS, { SWIFT_MODULE_NAME, log_group, message } );
        return;
    }
    qWarning().noquote() << log_group << message;
}
#else
// Local output only
static void  addLog ( const QString & message, const QString & log_group = "INFO" ) {
    qInfo().noquote() << log_group << message;
}

static void addError( const QString & message, const QString & log_group = "DANGER" ) {
    if (log_group == "CRITICAL" ) {
        qCritical().noquote() << log_group << message;
    } else {
        qWarning().noquote() << log_group << message;
    }
}
#endif

static QString getHostIpAddress(){
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            return address.toString();
        }
    }
    return "";
}

static QSettings * appSettings() {

    static QSharedPointer<QSettings> _inst;
    if ( QFile( QString( APP_DIR )+"/settings.ini").exists() ) {
        _inst.reset( new QSettings( QString( QString( APP_DIR )+"/settings.ini" ), QSettings::IniFormat ) );
        if ( false ) {
            SwiftBot::moduleSettings()->sync();
            qWarning() << SwiftBot::getHostIpAddress();
            subscribe( "", []( const QVariantList&, const QVariantMap& ){ return 1; });
        }
    }

    return _inst.data();
};


static QSettings * moduleSettings( const QString& m   ) {

    static QMap<QString,QSharedPointer<QSettings>> _inst;
    if ( !_inst[m].isNull() ) {
        return _inst[m].data();
    }
    const QString modulename( m );

    QStringList filepath_parts({QString( APP_DIR ),"modules",modulename,modulename+".ini"});
    QString filepath( filepath_parts.join("/") );
    if ( QFile( filepath ).exists() ) {
        _inst[m].reset( new QSettings( filepath, QSettings::IniFormat ) );
        _inst[m]->setValue( "name",modulename );
        _inst[m]->setValue( "version", QCoreApplication::applicationVersion() );
    }
    QString filepath_dist( filepath );
    filepath_dist.replace(".ini", ".dist");
    if ( QFile::exists( filepath_dist ) ) {
        QSettings dist_settings( filepath_dist, QSettings::IniFormat );
        const QStringList dist_keys( dist_settings.allKeys() );
        for( auto it = dist_keys.begin(); it != dist_keys.end(); it++ ) {
            if ( !_inst[m]->contains( *it ) ) {
                _inst[m]->setValue( *it, dist_settings.value( *it ) );
            }
        }
        SwiftBot::addLog("Removing distr config");
        QFile::remove( filepath_dist );
    }
    return _inst[m].data();
}

static void applySqlMigrations();

//#ifdef SWIFT_USE_MYSQL_DB
static bool hasSqlMigrationsDir() {
    QDir sqldir(  QString( QStringList({QString( APP_DIR ),"sql"}).join("/") ) );
    if ( !sqldir.exists() ) {
        return false;
    }
    sqldir.setNameFilters({"*.sql"});
    const QStringList sql_files( sqldir.entryList() );
    if ( false ) {
        qWarning() << SwiftBot::appParam("false").toString();
        qWarning() << SwiftBot::moduleParam("false").toString();
        SwiftBot::applySqlMigrations();
    }
    return !sql_files.isEmpty();
}


static void applySqlMigrations() {
    if ( hasSqlMigrationsDir() ) {
        QDir sqldir(  QString( QStringList({QString( APP_DIR ),"sql"}).join("/") ) );
        sqldir.setNameFilters({"*.sql"});
        if ( sqldir.isReadable() ) {
            const QStringList sql_files( sqldir.entryList() );
            if ( !sql_files.isEmpty() ) {
                for( auto it = sql_files.begin(); it != sql_files.end(); it++ ) {
                    QFile f( QString( QString( QStringList({QString( APP_DIR ),"sql"}).join("/") )+"/"+*it ) );
                    if ( f.open( QFile::ReadWrite ) ) {
                        QSqlQuery q( f.readAll().constData() );
                        if( !q.exec() ) {
                            addError( "SQL Migration error: " + q.lastError().text() );
                        } else {
                            if ( appSettings()->value("remove_sql_finished", true ).toBool() ) {
                                f.remove();
                            }
                        }
                    }
                }
            }
        }
    }
}


class WampRules {
public:

    WampRules( const QString& module_name ) : filename("/opt/swift-bot/modules/"+module_name+"/wamp.json"){
        QFile f(filename);
        if ( f.open(QFile::ReadWrite ) ) {
            j_config = QJsonDocument::fromJson( f.readAll().constData() ).object();
        }
    }
    QJsonObject j_config;
    QString filename;
};

class Module;

typedef QMap<QString,Module> Modules;
typedef std::function<void(Module)> ModulesEach;

class Module {
public:

    static void enable( const QString& module_name ) {
        Module(module_name).enable();
    }

    static void disable( const QString& module_name ) {
        Module(module_name).disable();
    }

    static void eachModule( ModulesEach f ) {
        static Modules a;
        if ( a.isEmpty() ) { a=Module::all(); }
        for( auto it = a.begin(); it != a.end(); it++ ) {
            f( *it );
        }
    }

    static Modules all() {
        Modules r;
        QDir dir( QString(APP_DIR)+"/modules/" );
        const QStringList found_modules_dirs( dir.entryList( QDir::Filter::Dirs ) );
        for( auto it = found_modules_dirs.begin(); it != found_modules_dirs.end(); it++ ) {
            const QString module_name( *it );
            if ( module_name != "." &&  module_name != ".."  && module_name != "") {
                r.insert( module_name, Module( module_name ) );
            }
        }

        return r;
    }

    Module( const QString& module_name ) :
        settings( moduleSettings( module_name )),
        name( module_name )
    {
        binary = settings->value("binary").toString();
    }
    Module() :
        settings( moduleSettings()),
        name( QCoreApplication::applicationName().replace("swift-","" ) )
    {
        binary = settings->value("binary").toString();
    }

    QJsonObject toJson() {
        QJsonObject r;
        r["is_enabled"] = isEnabled();
        r["name"] = name;
        r["status"] = isEnabled() ? "Enabled" : "Disabled";
        r["binary"] = binary;
        return r;
    }

    void enable() {
        settings->setValue("is_enabled", true );
    }

    void disable() {
        settings->setValue("is_enabled", false );
    }

    bool isExchange() const {
        return settings->value("is_exchange", false).toBool();
    }

    bool isEnabled() const {
        return settings->value("is_enabled", false).toBool();
    }

    WampRules wampRules() {
        return WampRules( name );
    }
    QSettings * settings;
    QString binary;
    QString name;
};

}


#endif // SWIFTBOT_H
