#ifndef MARKETSFILTER_H
#define MARKETSFILTER_H


#include "../swift-corelib/wampclient.h"
#include <swiftcore.h>
#include "../swift-corelib/assetsstorage.h"

struct filter_settings {
    QMap<quint32,double> _min_sizes;
    QMap<quint32,double> _max_sizes;
    QMap<quint32,double> _stp_sizes;

};

#include <QElapsedTimer>

#include <QObject>


class MarketsFilter : public QObject
{
    Q_OBJECT
public:
    explicit MarketsFilter(QObject *parent = nullptr);
    void addLog( const QString& msg, const QString& group = "INFO" );
    void addError( const QString& msg, const QString& group = "WARNING" );
signals:
    void sendWindowEvent( const QJsonObject& j_window );
    void orderbooksSnapshot( const QJsonObject& j_data );
public slots:
    void precessSnapShot( const QJsonObject& j_data );
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
