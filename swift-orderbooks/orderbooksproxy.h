#ifndef ORDERBOOKSPROXY_H
#define ORDERBOOKSPROXY_H

#include <wampclient.h>
#include <swiftcore.h>
#include "../swift-corelib/assetsstorage.h"

#include <QObject>
#include <QQueue>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

using namespace QtConcurrent;

class OrderBooksProxy : public QObject
{
    Q_OBJECT
public:
    explicit OrderBooksProxy(QObject *parent = nullptr);
    bool isDebug();

    void addLog(const QString& msg);
    void addError(const QString& msg);

    void addDebug(const QString& msg );

signals:
    void getOrderBooks(  const quint32& _pair );
    void orderBooks( const QJsonObject& j_books );
public slots:
    void processQueue();
    void publish();
    void getAllOrderBooks();
    void onRequestOrderBooks( const quint32& _pair );
    void onWampSession( Wamp::Session * sess );
    void mergeOrderbooks( const QString& topic, const QJsonObject& j_books );
    void checkOutDated();


    void reloadTargets();
    void sendRates();

    void logRates();
private:
    double getRateDiffHour( const quint32& pid );
    QMap<quint32, QMap<quint64, double>> _rates_history_data;

    QString getReportFormattedMessage();
    double getArbitragePairRate( const quint32 & arbpair );
    QMap<quint32, double> _current_rates;
    QMap<quint32, quint64> _last_updates;

    QJsonObject getSnapshot() const;
    QHash<quint32, QMap<double,double>> _asks;
    QHash<quint32, QMap<double,double>> _bids;

    QMap<quint32,quint64> _outdated_counters;
    QMap<quint32,quint64> _success_counters;
    QMap<quint32,quint64> _errors_counters;
    QMap<quint32,quint64> _requests_counters;



    QMap<quint32, QDateTime> _last_update_times;

    QTimer * orderbooks_interval;
    QTimer * send_rates_interval;
    QMap<quint32, QList<quint64>> _awaiters;
    QMap<quint32,QMap<quint64,quint32>> _uuids_to_pairs;
    QList<quint32> _pairs;
    QStringList _exchanges;
    Wamp::Session * session;
    QTimer * pub_timer;
    QMutex m;
    quint32 limit_records_count;
    quint64 valid_interval;

    QTimer * _rates_logger;
    QMap<quint32, QVector<double>> _rates_history;

    QMap<quint32, quint32> _pairs_errs_counter;
    QMap<quint32, quint32> _exchs_errs_counter;




    QQueue<QPair<quint32, QJsonObject>> _merge_queue;
};

#endif // ORDERBOOKSPROXY_H
