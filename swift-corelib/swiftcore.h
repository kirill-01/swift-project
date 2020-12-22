#ifndef SWIFTCORE_H
#define SWIFTCORE_H

#include "swiftbot.h"

#include "swift-corelib_global.h"
#include <QSettings>
#include <QSharedPointer>
#include <QFile>
#include "assetsstorage.h"
#include <QDateTime>

class SWIFTCORELIB_EXPORT SwiftCore
{
public:
    SwiftCore();

    /**
     * @brief createQuery
     * Create query in called thread
     * @param dbname
     * @return
     */
    static QSqlQuery createQuery( const QString& dbname = "default" );

    /**
     * @brief getExchangeFee
     * Shorthand to assets
     * @param exchange_id
     * @return
     */
    static double getExchangeFee( const quint32& exchange_id ) {
        return getAssets()->getExchangeTradeFee( exchange_id );
    }

    /**
     * @brief getAssets
     * Get assets object
     * @return
     */
    static AssetsStorage * getAssets( const bool& force_reload = false ) {
        static AssetsStorage* _i = nullptr;
        if ( _i == nullptr ) {
            _i = new AssetsStorage();
            _i->loadAssets();
        } else {
            if ( force_reload ) {
                _i->loadAssets();
            }
        }

        return _i;
    }

    /**
     * @brief getSettings
     * Get system settings
     * @param home_dir
     * @return
     */
    static QSettings * getSettings( const QString& s = "") {
        Q_UNUSED(s)
        return SwiftBot::appSettings();
    }

    /**
     * @brief getModuleSettings
     *
     * get module params file
     * @param module_name
     * @return
     */
    static QSettings * getModuleSettings( const QString& s = "") {
         Q_UNUSED(s)
        if ( s == "" ) {
            return SwiftBot::moduleSettings();
        } else {
            return SwiftBot::moduleSettings( s );
        }
    }
};



struct Ask {
    Ask( const double&a, const double&b ) : rate( a ), amount( b ) {

    }
    double rate;
    double amount;
    double price( const double& max = 0 ) const {
        if ( max != 0 && max < amount ) {
            return max * rate;
        } else {
            return amount * rate;
        }
    }
};

struct Bid {
    Bid( const double&a, const double&b ) : rate( a ), amount( b ) {

    }
    double rate;
    double amount;
    double price( const double& max = 0 ) const {
        if ( max != 0 && max < amount ) {
            return max * rate;
        } else {
            return amount * rate;
        }
    }
};

struct Asks {
    Asks( const QMap<double, double> & a ) {
        _amount = 0;
        for( auto it = a.begin(); it != a.end(); it++ ) {
            _items.push_back( Ask( it.key(), it.value() ) );
            _amount += it.value();
        }
    }
    bool hasSizes( const double& am ) {
        return amount() >= am;
    }

    double getRate( const double& amount ) {
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
    double getPrice( const double& amount ) {
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

    double amount() const {
        return _amount;
    }
    double _amount;
    QVector<Ask> _items;
};


struct Bids {
    Bids( const QMap<double, double> & a ) {
        _amount = 0;
        for( auto it = a.begin(); it != a.end(); it++ ) {
            _items.push_back( Bid( it.key(), it.value() ) );
            _amount += it.value();
        }
    }
    bool hasSizes( const double& am ) {
        return amount() >= am;
    }
    double getRate( const double& amount ) {
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
    double getPrice( const double& amount ) {
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
    double amount() const {
        return _amount;
    }
    double _amount;
    QVector<Bid> _items;
};

struct OrderbooksVariant {
    OrderbooksVariant( const double&a, const double&rs, const double&br, const double& sf, const double&bf )
        : amount( a ), sell_rate( rs ), buy_rate( br ), sell_fee(sf ), buy_fee( bf ) {
        fee = sellFee() + buyFee();
        profit = sellPrice() - buyPrice() - fee;
    }
    double sellPrice() const {
        return amount * sell_rate;
    }
    double buyPrice() const {
        return amount * buy_rate;
    }
    double sellFee() const {
        return sellPrice() * sell_fee;
    }
    double buyFee() const {
        return buyPrice() * buy_fee;
    }

    double fee;
    double amount;
    double sell_rate;
    double buy_rate;
    double profit;
    double sell_fee;
    double buy_fee;
};

struct OrdersVariants {
    OrdersVariants( const Asks& a, const Bids& b, const double&sf = 0.0018, const double&bf = 0.0018,
                    const double& min_order_size = 0.1,
                    const double& max_order_size = 50,
                    const double& step = 0.1 ) :
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
    bool hasAmountAsks(  const double& target_amount ) {
        return asks.amount() >= target_amount;
    }
    bool hasAmountBids(  const double& target_amount ) {
        return bids.amount() >= target_amount;
    }
    QList<OrderbooksVariant> getProfitable() {
        QList<OrderbooksVariant> _r;
        for( auto it = _variants.begin(); it!= _variants.end(); it++ ) {
            if ( it->profit > 0 ) {
                _r.push_back( *it );
            }
        }
        return _r;
    }
    double sell_fee;
    double buy_fee;
    Asks asks;
    Bids bids;
    QList<OrderbooksVariant> _variants;
};


namespace SwiftBot {


class Exchange;
class ArbitrageWindowEvent;

    typedef std::function<void(Coin)> CoinsEach;

    class Currency {
    public:

        Coin coin();

        Exchange exchange();

        double totalUsd();
        double balance();

        Currency( const quint32& currency_id );
        quint32 id;
        quint32 exchange_id;
        quint32 coin_id;

        QString name;

        bool is_enabled;

        AssetsStorage * _storage;
    };

    typedef std::function<void(Currency)> CurrenciesEach;

    class Coin {
    public:
        static void forEachCoin( CoinsEach func ) {
            QList<quint32> _coins( SwiftCore::getAssets()->getCoins() );
            for( auto it = _coins.begin(); it != _coins.end(); it++ ) {
                func( Coin( *it ) );
            }
        }

        void forEachCurrency( CurrenciesEach func ) {
            const Currencies c( getCurrencies() );
            if ( !c.isEmpty() ) {
                for( auto it = c.begin(); it!= c.end(); it++ ) {
                    func( it.value() );
                }
            }
        }

        Currencies getCurrencies() {
            Currencies ret;
            if ( id > 0 ) {
                QList<quint32> _curs( _storage->getCurrenciesByCoin( id ) );
                for ( auto it = _curs.begin(); it != _curs.end(); it++ ) {
                    ret.insert( *it, Currency( *it ) );
                }
            }
            return ret;
        }

        Coin( const quint32 & coin_id );

        QJsonArray getSparkLine() {
            QJsonArray ret;
            QSqlQuery q("SELECT AVG(r.rate) as rr, HOUR(r.ts) as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 )  AND r.ts >= date_sub(NOW(), interval 24 hour) GROUP BY HOUR(r.ts) ORDER BY HOUR(r.ts) DESC");
            if ( q.exec() ) {
                while( q.next() ) {
                    ret.push_back( QString::number( q.value("rr").toDouble(), 'f', 8 ) );
                }
            }
            return ret;
        }

        double usd_rate() const {
            QSqlQuery q("SELECT r.rate as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 ) ORDER BY r.ts DESC LIMIT 1");
            if ( q.exec() && q.next() ) {
               return q.value("rrr").toDouble();
            } else {
                return 1;
            }
        }

        double usd_rate_changed() {

            QSqlQuery q("SELECT r.rate as rrr FROM rates r WHERE r.pair_id IN ( SELECT p.id FROM pairs p LEFT JOIN arbitrage_pairs ap ON p.arbitrage_pair_id=ap.id WHERE ap.base_coin_id="+QString::number( id )+" AND ap.price_coin_id=3 )  AND r.ts <= date_sub(NOW(), interval 24 hour) ORDER BY r.ts DESC LIMIT 1");
            if ( q.exec() && q.next() ) {
                return usd_rate() - q.value("rrr").toDouble();
            } else {
                return 0;
            }
        }
        quint32 id;
        QString name;
        bool is_enabled;
        AssetsStorage * _storage;
    };

    class Market {
    public:

        double rate();

        QPair<Asks,Bids> orderBooks();

        Exchange exchange();

        Currency baseCurrency();

        Currency quoteCurrency();

        Market( const quint32& market_id );
        quint32 id;

        quint32 exchange_id;
        quint32 arbitrage_pair_id;
        quint32 base_currency_id;
        quint32 quote_currency_id;

        quint32 amount_precision;
        quint32 price_precision;

        QString name;

        bool is_enabled;

        AssetsStorage * _storage;
    };

    typedef std::function<void(Market)> MarketsEach;
    typedef   std::function<void(Exchange)> ExchangesEach;

    class Exchange {

    public:
        Markets markets();

        static void forEachExchanges( ExchangesEach func ) {
            QList<quint32> _exchs( SwiftCore::getAssets()->getAllExchanges() );
            for( auto it = _exchs.begin(); it != _exchs.end(); it++ ) {
                func( Exchange( *it ) );
            }
        }

        void forEachMarket( MarketsEach func );

        Exchange( const QString& name_ );
        Exchange( const quint32 & exchange_id );
        bool isRequstsSeparated();
        quint32 id;
        QString name;
        bool is_enabled;
        AssetsStorage * _storage;
    };



    typedef std::function<void(ArbitragePair)> ArbitragePairsEach;

    typedef QList<SwiftBot::ArbitrageWindowEvent> AritrageWindowEvents;

    class ArbitragePair {
    public:

        static void eachPair( ArbitragePairsEach pair_each );

        static ArbitragePairs all();

        Markets markets();

        void eachMarkets( const MarketsEach & markets_each );

        ArbitragePair( const quint32& arbitrage_pair_id );

        AritrageWindowEvents dailyEvents();
        quint32 id;
        quint32 base_coin_id;
        quint32 quote_coin_id;

        QString name;
        QList<quint32> _markets;

        AssetsStorage * _storage;

    };

    class Transaction {
        public:
        Transaction() {

        }
        Transaction( const QSqlRecord& rec, const quint32& type_ ) {
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
        Transaction( const QJsonObject& jdata ) {
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
        void update( const QJsonObject& jdata ) {
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
        QJsonObject toJson() const {
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

        void save(){
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
        quint32 id;
        quint32 type;
        quint32 status;
        quint32 exchange_id;
        quint32 currency_id;
        QString address;
        QString remote_id;
        QString txhash;
        double amount;
        double fee;
        QDateTime created_at;
        QDateTime completed_at;

    };

    class Order {
    public:

        Order();
        static Order create( const quint32& market_id,
                      const double& amount,
                      const double& rate,
                      const quint32& type
                      );

        /**
         * @brief update
         *
         * Update existing order
         *
         */
        void update();

        Exchange exchange() {
            return Exchange(exchange_id);
        }
        Market market();

        /**
         * @brief save
         * Inserting new order
         */
        void save();

        Order( const quint32& market_id_,
               const double& amount_,
               const double& rate_,
               const quint32& type_ );

        Order( const QSqlRecord& q );

        Order( const quint32& id );

        Order( const QJsonObject& j_data );

        QJsonObject toJson();

        bool place( Wamp::Session * session = nullptr ) {
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

        void update ( const QJsonObject & j_data );
        void fixvals() {
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
        quint64 local_id;
        QString remote_id;
        double amount;
        double amount_left;
        double rate;
        double price;
        double fee;
        double fee_amount;
        QDateTime created_at;
        QDateTime completed_at;
        quint32 market_id;
        quint32 exchange_id;
        quint32 type;
        quint32 status;
    };


    class ArbitrageWindowEvent {
    public:
        ArbitrageWindowEvent( const QSqlRecord& rec ) {
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

        quint32 id;

        quint32 sell_market_id;
        quint32 buy_market_id;

        QDateTime ts;

        double min_amount;
        double min_sell_rate;
        double min_buy_rate;

        double max_amount;
        double max_sell_rate;
        double max_buy_rate;

        double min_profit;
        double max_profit;

        double roi;
        double spread;
    };
}

#endif // SWIFTCORE_H
