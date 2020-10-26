#include "marketsfilter.h"
#include <QtConcurrent/QtConcurrent>

using namespace QtConcurrent;

struct filter_settings {
    QMap<quint32,double> _min_sizes;
    QMap<quint32,double> _max_sizes;
    QMap<quint32,double> _stp_sizes;

};

MarketsFilter::MarketsFilter(QObject *parent) : QObject(parent), is_debug( false ), session(nullptr)
{
    _recalcs_counter = 0;
    _profitable_calc_counter =0;
    _profitable_variants_counter =0;
    is_debug = SwiftBot::appSettings()->value( SETTINGS_NAME_MODULES_DEBUG, false ).toBool();
    connect( this, &MarketsFilter::sendWindowEvent, this, &MarketsFilter::onWindowEventSend );
}

void MarketsFilter::addLog(const QString &msg, const QString& group) {
    if ( is_debug &&  session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_LOGS, {"arbitrage", group, msg });
    }
}

void MarketsFilter::addError(const QString &msg, const QString& group) {
    if ( is_debug &&  session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_ERRORS, {"arbitrage", group, msg });
    }
}

void MarketsFilter::onWindowEventSend(const QJsonObject &j_window) {
    if ( session != nullptr && session->isJoined() ) {
        const QString evobject( QJsonDocument( j_window ).toJson( QJsonDocument::Compact ) );
        session->publish( FEED_EVENTS_ARBITRAGE, {"WINDOW", evobject });
    }
}

void MarketsFilter::onWampSession(Wamp::Session * sess) {
    session = sess;
    addLog("Connected and ready to work", "DEBUG");


    started_time = QDateTime::currentDateTime();

    session->provide("swift.arbitrage.info",[=]( const QVariantList& v, const QVariantMap& m ) {
        Q_UNUSED(m)
        Q_UNUSED(v)
        QJsonObject j_ret;


        for( auto it = _arbitrage_pairs.begin(); it != _arbitrage_pairs.end(); it++ ) {
            const quint32 apid( it.key() );
            const QString ap_name( SwiftCore::getAssets()->getArbitragePairName( apid ) );
            const QList<quint32> _pids( it.value() );
            QJsonObject j_pairs;
            for( auto it1 = _pids.begin(); it1!= _pids.end(); it1++ ) {
                const quint32 pid( *it1 );
                const QString exch_name( SwiftCore::getAssets()->getMarketExchangeName( pid ) );
                j_pairs[exch_name] = QJsonObject({
                                                    {"sell",QString::number( _pair_sell_rate.value(pid, 0), 'f', 8)},
                                                     {"buy",QString::number( _pair_buy_rate.value(pid, 0), 'f', 8)},
                                                     {"updated",QString::number( _pair_check_timers.value( pid, 0 ))},
                                                     {"sell_volume",QString::number( _pair_sell_volume.value( pid, 0 ))},
                                                     {"buy_volume",QString::number( _pair_buy_volume.value( pid, 0 ))},
                                                 });
            }
            j_ret[ ap_name ] = j_pairs;
        }
        const QString ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
        return ret;
    });

    session->subscribe( FEED_ORDERBOOKS_SNAPSHOT, [=]( const QVariantList& v, const QVariantMap& m ) {
        QElapsedTimer t;
        t.start();
        QString _msg;
        _recalcs_counter++;
        Q_UNUSED(m);
        const QString str( v.at(0).toString() );
        QJsonArray asks( QJsonDocument::fromJson( str.toUtf8() ).object().value("asks").toArray() );
        QJsonArray bids( QJsonDocument::fromJson( str.toUtf8() ).object().value("bids").toArray() );

        QMap<quint32, QMap<double,double>> __bids;
        for( auto it_ = bids.begin(); it_ != bids.end(); it_++ ) {
            const QJsonArray jrecs( it_->toArray() );
            for( auto it = jrecs.begin(); it!= jrecs.end(); it++ ) {
                const quint32 pid = it->toArray().at(0).toString().toUInt();
                const double rate = it->toArray().at(1).toString().toDouble();
                const double amount = it->toArray().at(2).toString().toDouble();
                __bids[ pid ].insert( rate, amount );
            }
        }
        QMap<quint32, QMap<double,double>> __asks;
        for( auto it_ = asks.begin(); it_ != asks.end(); it_++ ) {
            const QJsonArray jrecs( it_->toArray() );
            for( auto it = jrecs.begin(); it!= jrecs.end(); it++ ) {
                const quint32 pid = it->toArray().at(0).toString().toUInt();
                const double rate = it->toArray().at(1).toString().toDouble();
                const double amount = it->toArray().at(2).toString().toDouble();
                __asks[ pid ].insert( rate, amount );
            }
        }

        _orderbooks_processed += bids.count();
        _orderbooks_processed += asks.count();

        QList<QPair<quint32,quint32>> _checked;

        const QList<quint32> __b( __bids.keys() );
        const QList<quint32> __a( __asks.keys() );
        for( auto sellit = __b.begin(); sellit != __b.end(); sellit++ ) {
            for( auto buyit = __a.begin(); buyit != __a.end(); buyit++ ) {
                if ( sellit != buyit ) {
                    if ( SwiftCore::getAssets()->getMarketArbitragePairId( *sellit ) == SwiftCore::getAssets()->getMarketArbitragePairId(*buyit) ) {
                        _checked.push_back(QPair<quint32,quint32>({ *sellit, *buyit }));
                    }
                }
            }
        }
        if ( is_debug ) {
            _msg += QString::number( _checked.count() ) + " pairs ";
        }
        QMap<QPair<quint32,quint32>, quint32> _arbpairs_rel;
        if ( !_order_max_sizes.isEmpty() ) {
            filter_settings sizes;
            sizes._max_sizes = _order_max_sizes;
            sizes._min_sizes = _order_min_sizes;
            sizes._stp_sizes = _order_step_sizes;
            QMap<QPair<quint32,quint32>, QList<OrderbooksVariant>> _p;
            for( auto it = _checked.begin(); it != _checked.end();it++ ) {
                const quint32 arbp = SwiftCore::getAssets()->getMarketArbitragePairId( it->first );
                if ( !__asks.value( it->first ).isEmpty() && !__bids.value( it->second ).isEmpty() ) {
                    _pair_check_timers[ it->first ] = QDateTime::currentSecsSinceEpoch();
                    _pair_check_timers[ it->second ] = QDateTime::currentSecsSinceEpoch();
                    _pair_sell_rate[ it->first ] = __asks.value( it->first ).firstKey();
                    _pair_buy_rate[ it->second ] = __bids.value( it->second ).lastKey();
                    _pair_sell_volume[ it->first  ] = Asks( __asks.value( it->first ) ).amount();
                    _pair_buy_volume[ it->second  ] = Bids( __bids.value( it->second ) ).amount();


                    OrdersVariants vars_filter( __asks.value( it->first ), __bids.value( it->second ),
                        SwiftCore::getAssets()->getExchangeTradeFee( SwiftCore::getAssets()->getMarketExchangeId( it->first ) ),
                        SwiftCore::getAssets()->getExchangeTradeFee( SwiftCore::getAssets()->getMarketExchangeId( it->second ) ),
                        sizes._min_sizes.value( arbp, SwiftCore::getModuleSettings("arbitrage")->value("min_order_size",0.1).toDouble() ),
                        sizes._max_sizes.value( arbp, SwiftCore::getModuleSettings("arbitrage")->value("max_order_size",5.1).toDouble() ),
                        sizes._stp_sizes.value( arbp, SwiftCore::getModuleSettings("arbitrage")->value("step_order_size",0.1).toDouble() )
                       );
                    QList<OrderbooksVariant> pp = vars_filter.getProfitable();
                    if ( !pp.isEmpty() ) {
                        _p[ *it ]=pp;
                    }
                }
            }
            if ( is_debug ) {
                _msg += " checked";
            }

            QMap<QPair<quint32,quint32>,QList<OrderbooksVariant>> _profitable = _p;

            if ( !_profitable.isEmpty() ) {
                if ( is_debug ) {
                    _msg += " ( has profitable )";
                }
                    for( auto it = _profitable.begin(); it != _profitable.end(); it++ ) {
                        QList<OrderbooksVariant> _vars( it.value() );
                        std::sort( _vars.begin(), _vars.end(),[]( const OrderbooksVariant &v1, const OrderbooksVariant &v2 ){
                            return v1.profit > v2.profit;
                        } );

                        double min_amount = 0;
                        double max_amount = 0;
                        for( auto it = _vars.begin(); it != _vars.end(); it++ ) {
                            if ( min_amount > it->amount || min_amount == 0 ) {
                                min_amount = it->amount;
                            }
                            if ( max_amount < it->amount || max_amount == 0 ) {
                                max_amount = it->amount;
                            }
                        }

                        _profitable_variants_counter += _vars.count();
                        QJsonObject event_data;

                        event_data["spread"] = QString::number( ( _vars.first().sell_rate - _vars.first().buy_rate ) / _vars.first().buy_rate * 100,'f',6 );
                        event_data["roi"] = QString::number( _vars.first().profit / ( _vars.first().sell_rate * _vars.first().amount ) * 100 ,'f',6 );
                        event_data["arbitrage_pair_id"] = QString::number( SwiftCore::getAssets()->getMarketArbitragePairId( it.key().first ) );
                        event_data["sell_market_id"] = QString::number( it.key().first );
                        event_data["buy_market_id"] = QString::number( it.key().second );
                        event_data["sell_exchange_name"] = SwiftCore::getAssets()->getMarketExchangeName( it.key().first ) ;
                        event_data["buy_exchange_name"] = SwiftCore::getAssets()->getMarketExchangeName( it.key().second ) ;
                        event_data["pair_name"] = SwiftCore::getAssets()->getArbitragePairName( _arbpairs_rel.value( it.key() ) );

                        event_data["min_amount"] = QString::number( min_amount, 'f', 8 );
                        event_data["min_profit"] = QString::number( _vars.last().profit, 'f', 8 );
                        event_data["min_sell_rate"] = QString::number( _vars.last().sell_rate, 'f', 8 );
                        event_data["min_sell_price"] = QString::number( _vars.last().sellPrice(), 'f', 8 );
                        event_data["min_sell_fee"] = QString::number( _vars.last().sellFee(), 'f', 8 );
                        event_data["min_buy_rate"] = QString::number( _vars.last().buy_rate, 'f', 8 );
                        event_data["min_buy_price"] = QString::number( _vars.last().buyPrice(), 'f', 8 );
                        event_data["min_buy_fee"] = QString::number( _vars.last().buyFee(), 'f', 8 );


                        event_data["max_amount"] = QString::number( max_amount, 'f', 8 );
                        event_data["max_profit"] = QString::number( _vars.first().profit, 'f', 8 );
                        event_data["max_sell_rate"] = QString::number( _vars.first().sell_rate, 'f', 8 );
                        event_data["max_sell_price"] = QString::number( _vars.first().sellPrice(), 'f', 8 );
                        event_data["max_sell_fee"] = QString::number( _vars.first().sellFee(), 'f', 8 );
                        event_data["max_buy_rate"] = QString::number( _vars.first().buy_rate, 'f', 8 );
                        event_data["max_buy_price"] = QString::number( _vars.first().buyPrice(), 'f', 8 );
                        event_data["max_buy_fee"] = QString::number( _vars.first().buyFee(), 'f', 8 );


                        emit sendWindowEvent( event_data );

                        const quint32 bcid = SwiftCore::getAssets()->getMarketBaseCurrency( it.key().first );
                        const quint32 mcid = SwiftCore::getAssets()->getMarketPriceCurrency( it.key().second );
                        const double sell_balance = session->call( RPC_BALANCE_GET, {bcid}).toDouble();
                        const double buy_balance = session->call( RPC_BALANCE_GET, {mcid}).toDouble();
                        if ( sell_balance > 0 && buy_balance > 0 ) {

                            // Calculate max order and send placing it
                        } else {
                            addLog("Has profitable pair but no balanecs available..");
                        }
                        QString msg_str;
                        QTextStream msg( &msg_str );
                        msg << "Founded profitable window:";
                        msg << "ROI: " << event_data.value("roi").toString();
                        msg << "Spread: " << event_data.value("spread").toString() << " Profit: " << event_data.value("max_profit").toString();
                        addLog(msg_str , "EVENTS");
                    }
            }

        } else {
            if ( is_debug ) {
                qWarning() << "No filters applyed";
            }
        }
        if ( is_debug ) {
            _msg += " at " + QString::number( t.elapsed() ) + " msecs";
            qWarning() << _msg;
        }
        _last_timings.push_back( t.elapsed() );
        if ( _last_timings.count() >= 1250 ) {
            quint64 recommended = std::accumulate( _last_timings.begin(), _last_timings.end(), 0 ) / _last_timings.count() * 2.25;
            qWarning() << recommended << " interval is being optimal for current settings";
            _last_timings.clear();
            if ( SwiftBot::moduleSettings()->value(SETTINGS_NAME_ORDERBOOKS_ADOPT_INTERVAL,false).toBool() ) {
                SwiftBot::moduleSettings("orderbooks")->setValue("publish_interval", recommended );
                qWarning() << "Interval changed";
            }
        }
        // Group by arbitrage pair and calculate here!
    });
    QTimer::singleShot( 1500, this, &MarketsFilter::recalcSizeSettings );
}

void MarketsFilter::recalcSizeSettings() {
    double btc_size_min = SwiftCore::getModuleSettings("arbitrage")->value("min_order_size",0.01).toDouble();
    double btc_size_max = SwiftCore::getModuleSettings("arbitrage")->value("max_order_size",65.00).toDouble();
    double btc_size_step = SwiftCore::getModuleSettings("arbitrage")->value("step_order_size",0.05).toDouble();

    _arbitrage_pairs = SwiftCore::getAssets()->getArbitragePairs();
    const QList<quint32> _apids( _arbitrage_pairs.keys( ) );

    if ( session != nullptr && session->isJoined() ) {
        QString ret = session->call("swift.rates").toString();
        const QJsonObject jrates( QJsonDocument::fromJson( ret.toUtf8()).object());
        QStringList keys( jrates.keys() );
        for( auto it = keys.begin(); it!= keys.end(); it++ ) {
            _rates[ it->toUInt() ] = jrates.value( *it ).toString().toDouble();
        }
    }

    quint32 btcarbpair = 0;
    for( int i = 0; i < _arbitrage_pairs.count(); i++ ) {
        const quint32 apid = _apids.at(i);
        const QString bname( SwiftCore::getAssets()->getArbitragePairBaseCurrencyName( apid ) );
        const QString mname( SwiftCore::getAssets()->getArbitragePairMarketCurrencyName( apid ) );
        quint32 basecoinid = SwiftCore::getAssets()->getMarketBaseCurrency( _arbitrage_pairs[apid].first() );
        QList<quint32> _pairs( _arbitrage_pairs.value( apid ) );

        if ( bname == "BTC" && mname == "USDT" ) {
            btcarbpair = basecoinid;
            _order_min_sizes[ apid ] = btc_size_min;
            _order_max_sizes[ apid ] = btc_size_max;
            _order_step_sizes[ apid ] = btc_size_step;
            for( auto cit = _pairs.begin(); cit != _pairs.end(); cit++ ) {
                if ( _rates.contains( *cit ) && _rates.value( *cit, 0 ) > 0  ) {
                    _cointousd_rates[ basecoinid ] = _rates.value( *cit );
                }
            }
        } else if ( mname == "USDT" ) {
            for( auto cit = _pairs.begin(); cit != _pairs.end(); cit++ ) {
                if ( _rates.contains( *cit ) && _rates.value( *cit, 0 ) > 0 ) {
                    _cointousd_rates[ basecoinid ] = _rates.value( *cit );
                }
            }
        }
    }

    for( int i = 0; i < _arbitrage_pairs.count(); i++ ) {
        const quint32 apid = _apids.at(i);
        const QString bname( SwiftCore::getAssets()->getArbitragePairBaseCurrencyName( apid ) );
        const QString mname( SwiftCore::getAssets()->getArbitragePairMarketCurrencyName( apid ) );
        quint32 basecoinid = SwiftCore::getAssets()->getMarketBaseCurrency( _arbitrage_pairs[apid].first() );
        QList<quint32> _pairs( _arbitrage_pairs.value( apid ) );

        if ( bname != "BTC" &&  mname == "USDT" ) {
            for( auto cit = _pairs.begin(); cit != _pairs.end(); cit++ ) {
                if ( _rates.contains( *cit ) && _rates.value( *cit, 0 ) > 0 ) {
                    _cointousd_rates[ basecoinid ] = _rates.value( *cit );
                    _order_min_sizes[ apid ] = btc_size_min * _cointousd_rates.value( btcarbpair ) / _cointousd_rates.value( basecoinid ) ;
                    _order_max_sizes[ apid ] = btc_size_max  * _cointousd_rates.value( btcarbpair ) / _cointousd_rates.value( basecoinid );
                    _order_step_sizes[ apid ] = btc_size_step * _cointousd_rates.value( btcarbpair ) / _cointousd_rates.value( basecoinid );

                }
            }
        }
    }

    QTimer::singleShot( 120000, this, &MarketsFilter::recalcSizeSettings );
}

quint64 MarketsFilter::getFilerRate() {
    const quint64 secs_from_start = QDateTime::currentSecsSinceEpoch() - started_time.toSecsSinceEpoch();
    const QList<quint64> _counters( _pair_calc_count.values() );
    const quint64 total_markets_filtered = std::accumulate( _counters.begin(), _counters.end(), 0 );
    if ( secs_from_start <= 0 && total_markets_filtered <= 0 ) {
        return 0;
    }
    return total_markets_filtered / secs_from_start;
}
