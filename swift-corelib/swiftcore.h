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
    static double getExchangeFee( const quint32& exchange_id );

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
    static QSettings * getSettings( const QString& s = "");

    /**
     * @brief getModuleSettings
     *
     * get module params file
     * @param module_name
     * @return
     */
    static QSettings * getModuleSettings( const QString& s = "");
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
    Asks( const QMap<double, double> & a );
    bool hasSizes( const double& am );

    double getRate( const double& amount );
    double getPrice( const double& amount );

    double amount() const;
    double _amount;
    QVector<Ask> _items;
};


struct Bids {
    Bids( const QMap<double, double> & a );
    bool hasSizes( const double& am );
    double getRate( const double& amount );
    double getPrice( const double& amount );
    double amount() const;
    double _amount;
    QVector<Bid> _items;
};

struct OrderbooksVariant {
    OrderbooksVariant( const double&a, const double&rs, const double&br, const double& sf, const double&bf );
    double sellPrice() const;
    double buyPrice() const;
    double sellFee() const;
    double buyFee() const;

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
                    const double& step = 0.1 );
    bool hasAmountAsks(  const double& target_amount );
    bool hasAmountBids(  const double& target_amount );
    QList<OrderbooksVariant> getProfitable();
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
        static void forEachCoin( CoinsEach func );

        void forEachCurrency( CurrenciesEach func );

        Currencies getCurrencies();

        Coin( const quint32 & coin_id );

        QJsonArray getSparkLine();

        double usd_rate() const;

        double usd_rate_changed();
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

        static void forEachExchanges( ExchangesEach func );

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
        Transaction();
        Transaction( const QSqlRecord& rec, const quint32& type_ );
        Transaction( const QJsonObject& jdata );
        void update( const QJsonObject& jdata );
        QJsonObject toJson() const;

        void save();
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

        Exchange exchange();
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

        void cancel( Wamp::Session * session = nullptr );

        bool place( Wamp::Session * session = nullptr );

        void update ( const QJsonObject & j_data );
        void fixvals();
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
        ArbitrageWindowEvent( const QSqlRecord& rec );

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

    class ArbitragePattern {
    public:

        ArbitragePattern( const QSqlRecord & rec ) {
            id = rec.value("id").toUInt();
        }

        quint32 id;

        quint32 sell_pair_id;
        quint32 buy_pair_id;

        quint32 frames_count;

        QVector<double> _sell_rate_frames;
        QVector<double> _buy_rate_frames;

        QVector<double> _sell_vol_frames;
        QVector<double> _buy_vol_frames;

        double tolerance;
    private:

    };
}

#endif // SWIFTCORE_H
