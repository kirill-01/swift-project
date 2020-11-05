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

    class Coin {
    public:
        Coin( const quint32 & coin_id );
         quint32 id;
         QString name;
         bool is_enabled;
         AssetsStorage * _storage;
    };

    class Currency {
    public:

        Coin coin();

        double balance();

        Currency( const quint32& currency_id );
        quint32 id;
        quint32 exchange_id;
        quint32 coin_id;

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

    class ArbitragePair {
    public:

        static void eachPair( ArbitragePairsEach pair_each );

        static ArbitragePairs all();

        Markets markets();

        void eachMarkets( const MarketsEach & markets_each );

        ArbitragePair( const quint32& arbitrage_pair_id );

        quint32 id;
        quint32 base_coin_id;
        quint32 quote_coin_id;

        QString name;
        QList<quint32> _markets;

        AssetsStorage * _storage;

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

        QJsonObject toJson();

        bool place() {
            QString str( QJsonDocument( toJson() ).toJson( QJsonDocument::Compact ));
            quint64 uid = SwiftBot::method( "swift.api.order.create."+exchange().name, {str} ).toULongLong();
            return ( uid > 100 );
        }

        void update ( const QJsonObject & j_data );
        quint64 local_id;
        QString remote_id;
        double amount;
        double amount_left;
        double rate;
        double price;
        double fee;
        QDateTime created_at;
        QDateTime completed_at;
        quint32 market_id;
        quint32 exchange_id;
        quint32 type;
        quint32 status;


    };
}

#endif // SWIFTCORE_H
