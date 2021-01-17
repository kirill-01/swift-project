#include "swiftcore.h"
#include <QDebug>
#include <QCoreApplication>

SwiftCore::SwiftCore()
{
}

QSqlQuery SwiftCore::createQuery(const QString &dbname)
{
    static QHash<QString, QSqlDatabase> _dbs;
    if ( _dbs.contains( dbname ) ) {
         if ( _dbs[dbname].isOpen()  ) {
              return QSqlQuery( _dbs[dbname] );
         }
    } else {
        _dbs[dbname] = QSqlDatabase::addDatabase("QMYSQL", dbname );
    }

    QSqlDatabase db = _dbs[dbname];
    if( QSqlDatabase::contains( dbname ) )
    {
        db = QSqlDatabase::database( dbname );
        if ( !db.isOpen() ) {
            db.setHostName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_HOST,"localhost" ).toString() );
            db.setPort( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PORT, 3306 ).toInt() );
            db.setUserName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_USER,"swiftbot" ).toString() );
            db.setPassword( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PASSWORD,"swiftbot" ).toString() );
            db.setDatabaseName( SwiftCore::getSettings()->value("mysql_db","swiftbot" ).toString() );
            if ( !db.open() ) {
                qWarning() << db.lastError().text();
                qApp->exit( 1 );
            }
        }
    }
    else
    {
        db.setHostName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_HOST,"localhost" ).toString() );
        db.setPort( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PORT, 3306 ).toInt() );
        db.setUserName( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_USER,"swiftbot" ).toString() );
        db.setPassword( SwiftCore::getSettings()->value(SETTINGS_NAME_MYSQL_PASSWORD,"swiftbot" ).toString() );
        db.setDatabaseName( SwiftCore::getSettings()->value("mysql_db","swiftbot" ).toString() );
        if ( !db.open() ) {
            qWarning() << db.lastError().text();
            qApp->exit( 1 );
        }
    }
    _dbs[dbname] = db;
    return QSqlQuery( db );
}

double SwiftCore::getExchangeFee(const quint32 &exchange_id) {
    return getAssets()->getExchangeTradeFee( exchange_id );
}



QSettings *SwiftCore::getSettings(const QString &s) {
    Q_UNUSED(s)
    return SwiftBot::appSettings();
}

QSettings *SwiftCore::getModuleSettings(const QString &s) {
    Q_UNUSED(s)
    if ( s == "" ) {
        return SwiftBot::moduleSettings();
    } else {
        return SwiftBot::moduleSettings( s );
    }
}

double SwiftBot::Market::rate() {
    double r = 0;
    if ( wamp_client->isConnected() ) {
        QJsonObject j_ret = QJsonDocument::fromJson( wamp_client->getSession()->call( RPC_CURRENT_RATES ).toString().toUtf8() ).object();
        if ( j_ret.contains( QString::number( id ) ) ) {
            r = j_ret.value( QString::number( id ) ).toString().toDouble();
        }
    }
    return r;
}

QPair<Asks, Bids> SwiftBot::Market::orderBooks() {
    QMap<double,double> asks;
    QMap<double,double> bids;
    if ( wamp_client->isConnected() ) {
        QJsonObject j_ret = QJsonDocument::fromJson( wamp_client->getSession()->call( RPC_ORDERBOOKS_CACHE ).toString().toUtf8() ).object();
        const QJsonArray j_asks( j_ret.value("asks").toArray() );
        for( auto it = j_asks.begin(); it != j_asks.end(); it++ ) {
            asks[ it->toArray().at(1).toString().toDouble() ] = it->toArray().at(2).toString().toDouble();
        }
        const QJsonArray j_bids( j_ret.value("bids").toArray() );
        for( auto it = j_bids.begin(); it != j_bids.end(); it++ ) {
            bids[ it->toArray().at(1).toString().toDouble() ] = it->toArray().at(2).toString().toDouble();
        }
    }
    return QPair<Asks,Bids>({Asks( asks ),Bids( bids )});
}

SwiftBot::Exchange SwiftBot::Market::exchange() {
    return Exchange( exchange_id );
}

SwiftBot::Currency SwiftBot::Market::baseCurrency() {
    return Currency( base_currency_id );
}

SwiftBot::Currency SwiftBot::Market::quoteCurrency() {
    return Currency( quote_currency_id );
}

SwiftBot::Market::Market(const quint32 &market_id):
    id( market_id ),
    _storage( SwiftCore::getAssets() )
{
    exchange_id = _storage->getMarketExchangeId( id );
    arbitrage_pair_id = _storage->getMarketArbitragePairId( id );
    base_currency_id = _storage->getMarketBaseCurrency( id );
    quote_currency_id = _storage->getMarketPriceCurrency( id );
    is_enabled = _storage->isMarketActive( id );
    amount_precision = _storage->getMarketAmountPrecision( id );
    price_precision = _storage->getMarketPricePrecision( id );
    name = _storage->getMarketName( id );
}

void SwiftBot::Coin::forEachCoin(SwiftBot::CoinsEach func) {
    QList<quint32> _coins( SwiftCore::getAssets()->getCoins() );
    for( auto it = _coins.begin(); it != _coins.end(); it++ ) {
        func( Coin( *it ) );
    }
}

void SwiftBot::Coin::forEachCurrency(SwiftBot::CurrenciesEach func) {
    const Currencies c( getCurrencies() );
    if ( !c.isEmpty() ) {
        for( auto it = c.begin(); it!= c.end(); it++ ) {
            func( it.value() );
        }
    }
}

SwiftBot::Currencies SwiftBot::Coin::getCurrencies() {
    Currencies ret;
    if ( id > 0 ) {
        QList<quint32> _curs( _storage->getCurrenciesByCoin( id ) );
        for ( auto it = _curs.begin(); it != _curs.end(); it++ ) {
            ret.insert( *it, Currency( *it ) );
        }
    }
    return ret;
}

SwiftBot::Coin::Coin(const quint32 &coin_id) : id( coin_id ), _storage( SwiftCore::getAssets() ) {
    is_enabled = true;
    name = _storage->getCoinName( id );
}

QJsonArray SwiftBot::Coin::getSparkLine() {
    QJsonArray ret;
    QSqlQuery q("SELECT AVG(r.rate) as rr, HOUR(r.ts) as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 )  AND r.ts >= date_sub(NOW(), interval 24 hour) GROUP BY HOUR(r.ts) ORDER BY HOUR(r.ts) DESC");
    if ( q.exec() ) {
        while( q.next() ) {
            ret.push_back( QString::number( q.value("rr").toDouble(), 'f', 8 ) );
        }
    }
    return ret;
}

double SwiftBot::Coin::usd_rate() const {
    QSqlQuery q("SELECT r.rate as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 ) ORDER BY r.ts DESC LIMIT 1");
    if ( q.exec() && q.next() ) {
        return q.value("rrr").toDouble();
    } else {
        return 1;
    }
}

double SwiftBot::Coin::usd_rate_changed() {

    QSqlQuery q("SELECT r.rate as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 )  AND r.ts <= date_sub(NOW(), interval 24 hour) ORDER BY r.ts DESC LIMIT 1");
    if ( q.exec() && q.next() ) {
        return usd_rate() - q.value("rrr").toDouble();
    } else {
        return 0;
    }
}

SwiftBot::Coin SwiftBot::Currency::coin() {
    return Coin( coin_id );
}

SwiftBot::Exchange SwiftBot::Currency::exchange() {
    return Exchange( exchange_id );
}

double SwiftBot::Currency::totalUsd() {
    static double _r = 0;
    static quint64 last_update = 0;
    if ( QDateTime::currentSecsSinceEpoch() - last_update > 10 ) {
        if ( wamp_client != nullptr && wamp_client && wamp_client->isConnected() ) {
            _r = SwiftBot::method( RPC_BALANCE_GET, {id} ).toDouble();
            last_update = QDateTime::currentSecsSinceEpoch();
        }
    }
    return _r;
}

double SwiftBot::Currency::balance() {
    static double _r = 0;
    static quint64 last_update = 0;
    if ( QDateTime::currentSecsSinceEpoch() - last_update > 10 ) {
        if ( wamp_client != nullptr && wamp_client && wamp_client->isConnected() ) {
            _r = SwiftBot::method( RPC_BALANCE_GET, {id} ).toDouble();
            last_update = QDateTime::currentSecsSinceEpoch();
        }
    }
    return _r;
}

SwiftBot::Currency::Currency(const quint32 &currency_id) : id( currency_id ), _storage( SwiftCore::getAssets() ) {
    coin_id = _storage->getCurrencyCoin( id );
    exchange_id = _storage->getCurrencyExchangeId( id );
    is_enabled = _storage->isCurrencyActive( id );
    name = _storage->getCurrencyName( id );
}

SwiftBot::Markets SwiftBot::Exchange::markets() {
    Markets r;
    QList<quint32> _pairs( _storage->getExchangePairs( id ) );
    for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
        r.insert( *it, Market( *it ) );
    }
    return r;
}

void SwiftBot::Exchange::forEachExchanges(SwiftBot::ExchangesEach func) {
    QList<quint32> _exchs( SwiftCore::getAssets()->getAllExchanges() );
    for( auto it = _exchs.begin(); it != _exchs.end(); it++ ) {
        func( Exchange( *it ) );
    }
}

void SwiftBot::Exchange::forEachMarket(SwiftBot::MarketsEach func) {
    Markets all( markets() );
    for( auto it = all.begin(); it != all.end(); it++ ) {
        func( it.value() );
    }
}

SwiftBot::Exchange::Exchange(const QString &name_) : name( name_ ), _storage( SwiftCore::getAssets() ) {
    id = _storage->getExchangeId( name );
    is_enabled = true;
}

SwiftBot::Exchange::Exchange(const quint32 &exchange_id) : id( exchange_id ), _storage( SwiftCore::getAssets() ) {
    name = _storage->getExchangeName( id );
    is_enabled = true;
}

bool SwiftBot::Exchange::isRequstsSeparated() {
    return _storage->isSeparatedApi( id );
}

void SwiftBot::ArbitragePair::eachPair(SwiftBot::ArbitragePairsEach pair_each) {
    static ArbitragePairs _a;
    if ( _a.isEmpty() ) {
        _a = ArbitragePair::all();
    }
    for( auto it = _a.begin(); it != _a.end(); it++ ) {
        pair_each( it.value() );
    }
}

SwiftBot::ArbitragePairs SwiftBot::ArbitragePair::all() {
    static ArbitragePairs _r;
    if ( _r.isEmpty() ) {
        QList<quint32> _apids( SwiftCore::getAssets()->getArbitragePairs().keys() );
        for( auto it = _apids.begin(); it != _apids.end(); it++ ) {
            _r.insert( *it, ArbitragePair( *it ) );
        }
    }
    return _r;
}

SwiftBot::Markets SwiftBot::ArbitragePair::markets() {
    Markets _r;
    for( auto it = _markets.begin(); it != _markets.end(); it++ ) {
        _r.insert( *it, Market( *it ) );
    }
    return _r;
}

void SwiftBot::ArbitragePair::eachMarkets(const SwiftBot::MarketsEach &markets_each) {
    for( auto it = _markets.begin(); it != _markets.end(); it++ ) {
        markets_each( Market( *it ) );
    }
}

SwiftBot::ArbitragePair::ArbitragePair(const quint32 &arbitrage_pair_id) : id(arbitrage_pair_id), _storage( SwiftCore::getAssets() ) {
    _markets = _storage->getArbitragePairs().value( id );
    name = _storage->getArbitragePairName( id );
    base_coin_id = _storage->getArbitragePairBaseCoinId( id );
    quote_coin_id = _storage->getArbitragePairMarketCoinId( id );

}

SwiftBot::AritrageWindowEvents SwiftBot::ArbitragePair::dailyEvents() {
    AritrageWindowEvents r;
    QSqlQuery q("SELECT * FROM arbitrage_events WHERE arbitrage_pair_id="+QString::number( id )+" AND ts >= date_sub(NOW(), interval 24 hour) ");
    if ( q.exec() ) {
        while( q.next() ) {
            r.push_back( SwiftBot::ArbitrageWindowEvent( q.record() ) );
        }
    }
    return r;
}

SwiftBot::Order::Order() {

}

SwiftBot::Order SwiftBot::Order::create(const quint32 &market_id, const double &amount, const double &rate, const quint32 &type)
{

    Order o( market_id, amount, rate, type );
    if ( SwiftBot::appParam("is_debug", false ).toBool() ) {
        addLog("Creating order: " + QJsonDocument( o.toJson() ).toJson( QJsonDocument::Compact ), "DEBUG");
    }
    o.save();
    return o;
}

void SwiftBot::Order::update() {
    if ( local_id == 0 ) {
        save();
    } else {
        QSqlQuery q;
        QString q_str;
        if ( status < 2 ) {
            q_str = "UPDATE `orders` SET `remote_id` = '"+remote_id+"', \
                    `status` = "+QString::number( status )+", \
                    `amount_left` = "+QString::number( amount_left, 'f', 8 )+", \
                    `fee_amount` = "+QString::number( fee_amount , 'f', 8 )+" \
                    WHERE `id` = "+QString::number( local_id );
        } else {
            q_str = "UPDATE `orders` SET `remote_id` = '"+remote_id+"', \
                    `status` = "+QString::number( status )+", \
                    `amount_left` = "+QString::number( amount_left, 'f', 8 )+", \
                    `completed_at` = '"+completed_at.toString("yyyy-MM-dd hh:mm:ss")+"', \
                    `fee_amount` = "+QString::number( fee_amount , 'f', 8 )+" \
                    WHERE `id` = "+QString::number( local_id );
        }
        if ( !q.exec( q_str ) ) {
            qWarning() << "Cant save update for order object:" << q.lastError().text();
            SwiftBot::addError( "Cant save update for order object: "+q.lastError().text(), "CRITICAL" );
        }
    }
}

SwiftBot::Exchange SwiftBot::Order::exchange() {
    return Exchange(exchange_id);
}

SwiftBot::Market SwiftBot::Order::market() {
    return Market( market_id );
}

void SwiftBot::Order::save() {
    QSqlQuery q;
    QString q_str;
    if ( status <= 1 ) {
        q_str = "INSERT INTO `orders` (`remote_id`,`exchange_id`,`market_id`,`type`,`status`,`created_at`,`amount_left`,`amount`,`rate`,`price`,`fee`,`fee_amount`) VALUES "
                "('"+remote_id+"',\
                "+QString::number( exchange_id )+",\
                "+QString::number( market_id )+",\
                "+QString::number( type )+",\
                "+QString::number( status )+",\
                '"+created_at.toString("yyyy-MM-dd hh:mm:ss")+"',\
                "+QString::number( amount_left, 'f', 8 )+",\
                "+QString::number( amount, 'f', 8 )+",\
                "+QString::number( rate, 'f', 8 )+",\
                "+QString::number( price, 'f', 8 )+",\
                "+QString::number( fee, 'f', 4 )+",\
                "+QString::number( fee_amount, 'f', 8 )+") ON DUPLICATE KEY UPDATE status=VALUES(status),amount_left=VALUES(amount_left);";
    } else {
        q_str = "INSERT INTO `orders` (`remote_id`,`exchange_id`,`market_id`,`type`,`status`,`created_at`,`amount_left`,`completed_at`,`amount`,`rate`,`price`,`fee`,`fee_amount`) VALUES "
                "('"+remote_id+"',\
                "+QString::number( exchange_id )+",\
                "+QString::number( market_id )+",\
                "+QString::number( type )+",\
                "+QString::number( status )+",\
                '"+created_at.toString("yyyy-MM-dd hh:mm:ss")+"',\
                "+QString::number( amount_left, 'f', 8 )+",\
                '"+completed_at.toString("yyyy-MM-dd hh:mm:ss")+"',\
                "+QString::number( amount, 'f', 8 )+",\
                "+QString::number( rate, 'f', 8 )+",\
                "+QString::number( price, 'f', 8 )+",\
                "+QString::number( fee, 'f', 4 )+",\
                "+QString::number( fee_amount, 'f', 8 )+") ON DUPLICATE KEY UPDATE status=VALUES(status),amount_left=VALUES(amount_left),completed_at=VALUES(completed_at);";
    }
    if ( !q.exec( q_str ) ) {
        qWarning() << "Cant save new order object:" << q.lastError().text();
        SwiftBot::addError( "Cant save new order object: "+q.lastError().text(), "CRITICAL" );
    } else {
        if ( q.numRowsAffected() > 0 ) {
            local_id = q.lastInsertId().toUInt();
        } else {
            QSqlQuery qs("SELECT id FROM `orders` WHERE `exchange_id`="+QString::number(exchange_id)+" AND remote_id='"+remote_id+"'");
            if ( qs.exec() && qs.next() ) {
                local_id = qs.value("id").toUInt();
            } else {
                qWarning() << qs.lastError().text();
            }
        }
    }
}

SwiftBot::Order::Order(const quint32 &market_id_, const double &amount_, const double &rate_, const quint32 &type_)
    : local_id(0),
      amount( amount_ ),
      rate( rate_ ),
      market_id( market_id_ ),
      type( type_ )
{
    exchange_id = SwiftBot::Market( market_id ).exchange_id;
    fee = SwiftCore::getExchangeFee( exchange_id );
    amount_left = amount;
    created_at = QDateTime::currentDateTime();
    status = 0;
}

SwiftBot::Order::Order(const QSqlRecord &q) {
    local_id = q.value("id").toUInt();
    remote_id = q.value("remote_id").toUInt();
    amount = q.value("amount").toDouble();
    amount_left = q.value("amount_left").toDouble();
    rate = q.value("rate").toDouble();
    price = q.value("price").toDouble();
    fee = q.value("fee").toDouble();
    fee_amount = q.value("fee_amount").toDouble();
    created_at = q.value("created_at").toDateTime();
    completed_at = q.value("completed_at").toDateTime();
    market_id = q.value("market_id").toUInt();
    exchange_id = q.value("exchange_id").toUInt();
    type = q.value("type").toUInt();
    status = q.value("status").toUInt();
    fixvals();
}

SwiftBot::Order::Order(const quint32 &id) {
    QSqlQuery q("SELECT * FROM orders WHERE id="+QString::number( id ) );
    if ( q.exec() && q.next() ) {
        local_id = q.value("id").toUInt();
        remote_id = q.value("remote_id").toUInt();
        amount = q.value("amount").toDouble();
        amount_left = q.value("amount_left").toDouble();
        rate = q.value("rate").toDouble();
        price = q.value("price").toDouble();
        fee = q.value("fee").toDouble();
        fee_amount = q.value("fee_amount").toDouble();
        created_at = q.value("created_at").toDateTime();
        completed_at = q.value("completed_at").toDateTime();
        market_id = q.value("market_id").toUInt();
        exchange_id = q.value("exchange_id").toUInt();
        type = q.value("type").toUInt();
        status = q.value("status").toUInt();
    }
    fixvals();
}

SwiftBot::Order::Order(const QJsonObject &j_data) {
    update( j_data );
}

QJsonObject SwiftBot::Order::toJson() {
    QJsonObject j_obj;
    j_obj["market_id"] = QString::number( market_id );
    j_obj["market_name"] = SwiftCore::getAssets()->getMarketName( market_id );
    j_obj["exchange_name"] = exchange().name;
    j_obj["exchange_id"] = QString::number( exchange_id );
    j_obj["local_id"] = QString::number( local_id );
    j_obj["remote_id"] = remote_id;
    j_obj["type"] = type == 0 ? "sell" : "buy";
    j_obj["created_at"] = QString::number( created_at.toSecsSinceEpoch() );
    j_obj["completed_at"] = QString::number( completed_at.toSecsSinceEpoch() );
    j_obj["amount"] = QString::number( amount, 'f', market().amount_precision );
    j_obj["amount_left"] = QString::number( amount_left, 'f', market().amount_precision );
    j_obj["rate"] = QString::number( rate, 'f', market().price_precision );
    j_obj["price"] = QString::number( price, 'f', market().price_precision );
    j_obj["status"] = QString::number( status );
    j_obj["fee"] = QString::number( fee, 'f', 4 );
    j_obj["fee_amount"] = QString::number( fee_amount, 'f', 4 );
    return j_obj;
}

void SwiftBot::Order::cancel(Wamp::Session *session) {
    if ( SwiftBot::appParam("is_debug", false ).toBool() ) {
        addLog("Canceling order: " + QJsonDocument( toJson() ).toJson( QJsonDocument::Compact ), "DEBUG");
    }
    QString str( QJsonDocument( toJson() ).toJson( QJsonDocument::Compact ));
    if ( session != nullptr ) {
        session->call("swift.api.order.cancel."+exchange().name, {str} ).toULongLong();
    } else {
        SwiftBot::method( "swift.api.order.cancel."+exchange().name, {str} ).toULongLong();
    }
}

bool SwiftBot::Order::place(Wamp::Session *session) {
    if ( SwiftBot::appParam("is_debug", false ).toBool() ) {
        addLog("Placing order: " + QJsonDocument( toJson() ).toJson( QJsonDocument::Compact ), "DEBUG");
    }
    QString str( QJsonDocument( toJson() ).toJson( QJsonDocument::Compact ));
    if ( session != nullptr ) {
        quint64 uid = session->call("swift.api.order.create."+exchange().name, {str} ).toULongLong();
        return ( uid > 100 );
    } else {
        quint64 uid = SwiftBot::method( "swift.api.order.create."+exchange().name, {str} ).toULongLong();
        return ( uid > 100 );
    }
}

void SwiftBot::Order::update(const QJsonObject &j_data) {
    market_id = j_data.value("market_id").toString().toUInt();
    local_id = j_data.contains("local_id") && j_data.value("local_id").toString().toUInt() > 0 ? j_data.value("local_id").toString().toUInt() : 0;
    remote_id = j_data.value("remote_id").toString();
    type = j_data.value("type").toString() == "sell" ? 0 : 1;
    status = j_data.value("status").toString().toUInt();
    quint64 ctime = j_data.value("created_at").toVariant().toULongLong();
    quint64 utime = j_data.value("updated_at").toVariant().toULongLong() > 100000 ? j_data.value("updated_at").toVariant().toULongLong() : j_data.value("completed_at").toVariant().toULongLong();
    if ( utime < 1000000 && status >= 2 ) {
        utime = ctime;
    }
    created_at = QDateTime::fromSecsSinceEpoch( ctime > 1000000000000 ? ctime / 1000 : ctime );
    completed_at = QDateTime::fromSecsSinceEpoch( utime > 1000000000000 ? utime / 1000 : utime );
    amount = j_data.value("amount").toString().toDouble();
    amount_left = j_data.value("amount_left").toString().toDouble();
    rate = j_data.value("rate").toString().toDouble();
    price = j_data.value("price").toString().toDouble();
    exchange_id = j_data.value("exchange_id").toString().toUInt();
    fixvals();
}

void SwiftBot::Order::fixvals() {
    if ( price <= 0 && exchange_id > 0 && amount > 0  ) {
        price = ( amount_left <= 0 ? amount :  amount - amount_left ) * rate;
    }
    if ( fee <= 0 && exchange_id > 0 && amount > 0 ) {
        fee = SwiftCore::getExchangeFee( exchange_id );
    }
    if ( fee_amount <= 0 && exchange_id > 0 && amount > 0 ) {
        fee_amount = ( amount_left <= 0 ? amount :  amount - amount_left ) * fee;
    }
    if ( status >= 2 ) {
        if ( completed_at < created_at ) {
            completed_at = created_at;
        }
    }
}

SwiftBot::ArbitrageWindowEvent::ArbitrageWindowEvent(const QSqlRecord &rec) {
    id = rec.value("id").toUInt();

    sell_market_id = rec.value("sell_pair_id").toUInt();
    buy_market_id = rec.value("buy_pair_id").toUInt();

    ts = rec.value("ts").toDateTime();

    min_amount = rec.value("min_amount").toDouble();
    min_sell_rate = rec.value("min_sell_rate").toDouble();
    min_buy_rate = rec.value("min_buy_rate").toDouble();
    max_amount = rec.value("max_amount").toDouble();
    max_sell_rate = rec.value("max_sell_rate").toDouble();
    max_buy_rate = rec.value("max_buy_rate").toDouble();

    min_profit = rec.value("min_profit").toDouble();
    max_profit = rec.value("max_profit").toDouble();

    roi = rec.value("roi").toDouble();
    spread = rec.value("spread").toDouble();
}

SwiftBot::Transaction::Transaction() {

}

SwiftBot::Transaction::Transaction(const QSqlRecord &rec, const quint32 &type_) {
    id = rec.value("id").toUInt();
    type = type_;
    status = rec.value("status").toString().toUInt();
    currency_id = rec.value("currency_id").toUInt();
    exchange_id = SwiftCore::getAssets()->getCurrencyExchangeId( currency_id );
    address = rec.value("address").toString();
    remote_id = rec.value("remote_id").toString();
    txhash = rec.value("txhash").toString();
    amount = rec.value("amount").toDouble();
    fee = rec.value("fee").toDouble();
    created_at = rec.value("created_at").toDateTime();
}

SwiftBot::Transaction::Transaction(const QJsonObject &jdata) {
    id = jdata.value("id").toString().toUInt();
    remote_id = jdata.value("remote_id").toString();
    type = jdata.value("type").toString().toUInt();
    const QString strstatus( jdata.value("status").toString().toLower() );

    if ( strstatus == "ok" || strstatus == "completed" || strstatus == "done" || strstatus == "success" ) {
        status = 2;
    } else if ( strstatus == "error" || strstatus == "canceled" || strstatus == "failed" ) {
        status = 3;
    } else {
        status = 1;
    }

    amount = jdata.value("amount").toString().toDouble();
    address = jdata.contains("deposit_address") ? jdata.value("deposit_address").toString() : jdata.value("address").toString();
    created_at = QDateTime::fromSecsSinceEpoch( jdata.value("created_at").toString().toUInt() );
    currency_id = jdata.value("currency_id").toString().toUInt();
    txhash = jdata.value("tx").toString();
    if ( remote_id == "" || remote_id.isNull() || remote_id.isEmpty() ) {
        if ( !txhash.isEmpty() ) {
            remote_id = txhash;
        }
    }
}

void SwiftBot::Transaction::update(const QJsonObject &jdata) {
    id = jdata.value("id").toString().toUInt();
    remote_id = jdata.value("remote_id").toString();
    type = jdata.value("type").toString().toUInt();
    const QString strstatus( jdata.value("status").toString().toLower() );
    if ( strstatus == "ok" || strstatus == "completed" || strstatus == "done" || strstatus == "success" ) {
        status = 2;
    } else if ( strstatus == "error" || strstatus == "canceled" || strstatus == "failed" ) {
        status = 3;
    } else {
        status = 1;
    }
    amount = jdata.value("amount").toString().toDouble();
    address = jdata.contains("deposit_address") ? jdata.value("deposit_address").toString() : jdata.value("address").toString();
    created_at = QDateTime::fromSecsSinceEpoch( jdata.value("created_at").toString().toUInt() );
    currency_id = jdata.value("currency_id").toString().toUInt();
    txhash = jdata.value("tx").toString();
    if ( remote_id == "" || remote_id.isNull() || remote_id.isEmpty() ) {
        if ( !txhash.isEmpty() ) {
            remote_id = txhash;
        }

        // Withdraw has address
        if ( type == 0 ) {
            remote_id = address+QString::number( created_at.toSecsSinceEpoch() );
        } else {
            // Deposit not have address

        }
    }
}

QJsonObject SwiftBot::Transaction::toJson() const {
    QJsonObject jr;
    jr["id"] = QString::number( id );
    jr["remote_id"] = remote_id;
    jr["currency_id"] = QString::number( id );
    jr["amount"] = QString::number( amount, 'f', 8 );
    jr["created_at"] = QString::number( created_at.toSecsSinceEpoch() );
    jr["status"] = QString::number( status );
    jr["deposit_address"] = address;
    jr["type"] = QString::number( type );
    jr["tx"] = txhash;
    return jr;
}

void SwiftBot::Transaction::save(){
    const QString tbl = (type==1?"deposits":"withdraws");
    QSqlQuery q("INSERT INTO `"+tbl+"` \
                (`remote_id`, \
                 `currency_id`, \
                 `amount`, \
                 `address`, \
                 `created_at`, \
                 `txhash`, \
                 `status`, \
                 `fee` ) \
                VALUES \
                ('"+remote_id+"', \
                 "+QString::number( currency_id)+", \
                 "+QString::number( amount, 'f', 8)+", \
                 '"+address+"', \
                 '"+created_at.toString("yyyy-MM-dd hh:mm:ss")+"', \
                 '"+txhash+"', \
                 '"+QString::number( status )+"', \
                 "+QString::number( fee, 'f', 8)+") ON DUPLICATE KEY UPDATE status=VALUES(status);");
            if ( !q.exec() ) {
        qWarning() << q.lastError().text();
    }

    if ( q.numRowsAffected() > 0 ) {
        id = q.lastInsertId().toUInt();
    }
}

OrdersVariants::OrdersVariants(const Asks &a, const Bids &b, const double &sf, const double &bf, const double &min_order_size, const double &max_order_size, const double &step) :
    sell_fee(sf), buy_fee(bf), asks(a), bids(b)
{
    for( double size = min_order_size; size <= max_order_size; size += step ) {
        if ( hasAmountAsks( size ) && hasAmountBids( size ) ) {
            const double buy_price = asks.getRate( size );
            const double sell_price = bids.getRate( size );
            if ( sell_price > buy_price ) {
                _variants.push_back( OrderbooksVariant( size, bids.getRate(size), asks.getRate( size ), sell_fee, buy_fee ) );
            }
        }
    }
}

bool OrdersVariants::hasAmountAsks(const double &target_amount) {
    return asks.amount() >= target_amount;
}

bool OrdersVariants::hasAmountBids(const double &target_amount) {
    return bids.amount() >= target_amount;
}

QList<OrderbooksVariant> OrdersVariants::getProfitable() {
    QList<OrderbooksVariant> _r;
    for( auto it = _variants.begin(); it!= _variants.end(); it++ ) {
        if ( it->profit > 0 ) {
            _r.push_back( *it );
        }
    }
    return _r;
}

OrderbooksVariant::OrderbooksVariant(const double &a, const double &rs, const double &br, const double &sf, const double &bf)
    : amount( a ), sell_rate( rs ), buy_rate( br ), sell_fee(sf ), buy_fee( bf ) {
    fee = sellFee() + buyFee();
    profit = sellPrice() - buyPrice() - fee;
}

double OrderbooksVariant::sellPrice() const {
    return amount * sell_rate;
}

double OrderbooksVariant::buyPrice() const {
    return amount * buy_rate;
}

double OrderbooksVariant::sellFee() const {
    return sellPrice() * sell_fee;
}

double OrderbooksVariant::buyFee() const {
    return buyPrice() * buy_fee;
}

Bids::Bids(const QMap<double, double> &a) {
    _amount = 0;
    for( auto it = a.begin(); it != a.end(); it++ ) {
        _items.push_back( Bid( it.key(), it.value() ) );
        _amount += it.value();
    }
}

bool Bids::hasSizes(const double &am) {
    return amount() >= am;
}

double Bids::getRate(const double &amount) {
    double rate = 0;
    double r_filled = 0;
    for( auto it = _items.rbegin(); it != _items.rend(); it++ ) {
        if ( r_filled < amount ) {
            const double amleft = amount - r_filled;
            r_filled += amleft > it->amount ? it->amount : amleft;
            rate = it->rate;
        } else {
            return rate;
        }
    }
    return rate;
}

double Bids::getPrice(const double &amount) {
    double r = 0;
    double r_filled = 0;
    for( auto it = _items.rbegin(); it != _items.rend(); it++ ) {
        if ( r_filled < amount ) {
            const double amleft = amount - r_filled;
            r += it->price( amleft );
            r_filled += amleft > it->amount ? it->amount : amleft;
        } else {
            return r;
        }
    }
    return amount * _items.first().rate;
}

double Bids::amount() const {
    return _amount;
}

Asks::Asks(const QMap<double, double> &a) {
    _amount = 0;
    for( auto it = a.begin(); it != a.end(); it++ ) {
        _items.push_back( Ask( it.key(), it.value() ) );
        _amount += it.value();
    }
}

bool Asks::hasSizes(const double &am) {
    return amount() >= am;
}

double Asks::getRate(const double &amount) {
    double rate = 0;
    double r_filled = 0;
    for( auto it = _items.begin(); it != _items.end(); it++ ) {
        if ( r_filled < amount ) {
            const double amleft = amount - r_filled;
            r_filled += amleft > it->amount ? it->amount : amleft;
            rate = it->rate;
        } else {
            return rate;
        }
    }
    return rate;
}

double Asks::getPrice(const double &amount) {
    double r = 0;
    double r_filled = 0;
    for( auto it = _items.begin(); it != _items.end(); it++ ) {
        if ( r_filled < amount ) {
            const double amleft = amount - r_filled;
            r += it->price( amleft );
            r_filled += amleft > it->amount ? it->amount : amleft;
        } else {
            return r;
        }
    }
    return amount * _items.last().rate;
}

double Asks::amount() const {
    return _amount;
}
