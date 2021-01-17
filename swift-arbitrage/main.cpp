#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <swiftcore.h>

#include <QLockFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include "marketsfilter.h"
#include "statsnotifier.h"
#include <QThread>

QString prepareCurrencyName( const QString& api_name )  {
    QString lname( api_name.toLower() );
    lname.replace( "xbt", "btc" );
    lname.replace( "xbtc", "btc");
    lname.replace( "xeth", "eth");
    lname.replace( "usdt20", "usdt" );
    return lname.toUpper();
}

QStringList currencyNameVariants( const QString& coinname ) {
    QStringList names;
    names << QString( coinname + "20" ).toLower() << QString( "x" + coinname).toLower()
             << QString( coinname.toLower().replace("xbt","btc")).toLower();
    return names;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("swift-arbitrage");
    QCoreApplication::setApplicationVersion("1.0.479");

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

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("SwiftBot arbitrage trading module");
    parser.addHelpOption();
    parser.addVersionOption();

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    SwiftBot::initWampClient();


    if ( SwiftBot::moduleParam("assets_initiated", false ).toBool() && QDateTime::currentSecsSinceEpoch() - SwiftBot::moduleParam("assets_ts", 0).toUInt() >= 3600 ) {
        SwiftBot::moduleSettings()->setValue("assets_initiated",false);
    }

    if ( !SwiftBot::moduleParam("assets_initiated", false ).toBool() ) {

        // Get available coins
        QStringList default_enabled_names( SwiftCore::getSettings()->value(SETTINGS_NAME_DEFAULT_COINS,"").toString().split(",") );
        QSqlQuery q("SELECT DISTINCT( LOWER(name) ) as nm FROM currencies");
        QStringList avcoins;
        if ( q.exec() ) {
            while( q.next() ) {
                avcoins.push_back( q.value("nm").toString() );
            }
        }
        if( !avcoins.isEmpty() ) {
            for( auto it = avcoins.begin(); it != avcoins.end(); it++ ) {
                const QString prepared_name( prepareCurrencyName(*it) );
                const quint32 is_enabled = default_enabled_names.contains( prepared_name.toLower() ) ? 1 : 0;
                if ( is_enabled == 1 ) {
                    // Later add option to store all coins in db
                    if ( !q.exec("INSERT INTO coins (`name`, `is_enabled`) VALUES ('"+prepared_name+"', "+QString::number(is_enabled)+") ON DUPLICATE KEY UPDATE is_enabled="+QString::number( is_enabled )) ){
                        qWarning() << q.lastError().text();
                    }
                }
            }
        }


        // Assign coin_id to currencies
        const QList<quint32> _coins( SwiftCore::getAssets( true )->getCoins() );
        if ( !_coins.isEmpty() ) {
            for( auto it = _coins.begin(); it != _coins.end(); it++ ) {
                const quint32 coid = *it;
                const QString cname = SwiftCore::getAssets()->getCoinName( coid );
                if ( !q.exec("UPDATE currencies SET coin_id="+QString::number( coid )+" WHERE LOWER(name) IN ('"+currencyNameVariants( cname ).join("','")+"')") ) {
                    qWarning() << q.lastError().text();
                }
            }
        }

        // Create arbitrage pairs
        QStringList exclude_pairs( SwiftCore::getSettings()->value(SETTINGS_NAME_SKIP_PAIRS,"").toString().split(","));
        if ( q.exec("SELECT DISTINCT c.coin_id as bcid, c2.coin_id as mcid FROM pairs p left join currencies c on c.id=p.base_currency_id left join currencies c2 on c2.id=p.market_currency_id" ) ) {
            while ( q.next() ) {
                const quint32 bcid = q.value("bcid").toUInt();
                const quint32 mcid = q.value("mcid").toUInt();
                if ( bcid > 0 && mcid > 0 ) {
                    if ( SwiftCore::getAssets()->getArbitragePairByCoins( bcid, mcid ) == 0 ) {
                        const QString apname( SwiftCore::getAssets()->getCoinName( bcid )+"-"+SwiftCore::getAssets()->getCoinName( mcid ));
                        if ( !exclude_pairs.contains( apname ) ) {
                            QSqlQuery qq("INSERT INTO arbitrage_pairs (`base_coin_id`,`price_coin_id`,`name`,`is_enabled`) VALUES ("+QString::number( bcid )+","+QString::number( mcid )+", '"+apname+"', TRUE) ON DUPLICATE KEY UPDATE is_enabled=VALUES(is_enabled)");
                            if ( !qq.exec() ) {
                                qWarning() << qq.lastError().text();
                            }
                        }
                    }
                }
            }
        }

        // Update pairs to currenct arbitrage
        QList<quint32> _pairs( SwiftCore::getAssets()->getActiveMarkets() );
        for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
            const quint32 bcoid = SwiftCore::getAssets()->getCurrencyCoin( SwiftCore::getAssets()->getMarketBaseCurrency( *it ) );
            const quint32 mcoid = SwiftCore::getAssets()->getCurrencyCoin( SwiftCore::getAssets()->getMarketPriceCurrency( *it ) );
            const quint32 apaid = SwiftCore::getAssets()->getArbitragePairByCoins( bcoid, mcoid );
            if ( !q.exec("UPDATE pairs SET arbitrage_pair_id="+QString::number( apaid )+" WHERE id="+QString::number( *it ) ) ) {
                qWarning() << q.lastError().text();
            }
        }

        q.finish();
        SwiftBot::moduleSettings()->setValue("assets_ts", QDateTime::currentSecsSinceEpoch() );
        SwiftBot::moduleSettings()->setValue("assets_initiated",true);
    }
    // Create arbitrage windows


    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        const QString mname( QCoreApplication::applicationName().replace("swift-","") );
        wamp_client->publish( FEED_WATCHDOG, { mname });
    });
    watchdog->start();

    MarketsFilter * markets_filter = new MarketsFilter();

    QObject::connect( wamp_client.data(), &WampClient::clientConnected, markets_filter, &MarketsFilter::onWampSession );

    StatsNotifier * notifier = new StatsNotifier();

    QObject::connect( notifier, &StatsNotifier::updatedStats,[]( const QJsonObject& stats ){
        wamp_client->publishMessage( FEED_EVENTS_ARBITRAGE, stats );
    });

    QObject::connect( wamp_client.data(), &WampClient::clientConnected, notifier, &StatsNotifier::onWampSession );

    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    wamp_client->startClient();

    return a.exec();
}
