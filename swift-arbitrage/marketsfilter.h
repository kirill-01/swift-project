#ifndef MARKETSFILTER_H
#define MARKETSFILTER_H


#include "../swift-corelib/wampclient.h"
#include "../swift-corelib/swiftcore.h"
#include "../swift-corelib/assetsstorage.h"


#include <QObject>


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
        return amount * _items.first().rate;
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
        return amount * _items.last().rate;
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
        profit = ( sellPrice() - sellFee() ) - ( buyPrice() + buyFee() );
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
        for( double size = min_order_size; size < max_order_size; size += step ) {
            if ( hasAmountAsks( size ) && hasAmountBids( size ) ) {
                const double buy_price = asks.getPrice( size );
                const double sell_price = bids.getPrice( size );
                if ( sell_price > buy_price ) {
                    _variants.push_back( OrderbooksVariant( size, sell_price / size, buy_price / size, sell_fee, buy_fee ) );
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


class MarketsFilter : public QObject
{
    Q_OBJECT
public:
    explicit MarketsFilter(QObject *parent = nullptr);
    void addLog( const QString& msg, const QString& group = "INFO" );
    void addError( const QString& msg, const QString& group = "WARNING" );
signals:
    void sendWindowEvent( const QJsonObject& j_window );
public slots:
    void onWindowEventSend( const QJsonObject& j_window );
    void onWampSession( Wamp::Session * sess );

    void recalcSizeSettings();

private:
    bool is_debug;
    QVector<quint64> _last_timings;
    QPair<quint32,quint32> _known_pairs;
    QMap<quint32,double> _rates;
    QMap<quint32,double> _cointousd_rates;
    QHash<quint32,QList<quint32>> _arbitrage_pairs;
    double btcusdrate;
    QMap<quint32,double> _order_min_sizes;
    QMap<quint32,double> _order_max_sizes;
    QMap<quint32,double> _order_step_sizes;

    quint64 getFilerRate();
    QMap<quint32,quint64> _pair_calc_count;
    QMap<quint32,quint64> _pair_check_timers;
    QMap<quint32,double> _pair_sell_rate;
    QMap<quint32,double> _pair_buy_rate;
    QMap<quint32,double> _pair_sell_volume;
    QMap<quint32,double> _pair_buy_volume;
    quint64 _recalcs_counter;
    quint64 _profitable_calc_counter;
    quint64 _profitable_variants_counter;
    quint64 _orderbooks_processed;
    Wamp::Session * session;
    QDateTime started_time;
};

#endif // MARKETSFILTER_H
