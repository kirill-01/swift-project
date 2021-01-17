#ifndef STATSNOTIFIER_H
#define STATSNOTIFIER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlResult>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <swiftcore.h>
#include <QTimer>
#include <wampclient.h>
#include <assetsstorage.h>

struct FilteringStats {
    quint32 sell_pair;
    quint32 buy_pair;
    double min_size;
    double max_size;
    double min_profit;
    double max_profit;
    double weight;
    double roi;
    double spread;
    quint32 events_count;
    QJsonObject toJson() const;
};

class StatsNotifier : public QObject
{
    Q_OBJECT
public:
    explicit StatsNotifier(QObject *parent = nullptr);

signals:
    void insertLog( const QString& message, const QString& group = "INFO" );
    void insertError( const QString& message, const QString& group = "WARNING" );
    void updatedStats( const QJsonObject& jstats);
public slots:
    void locaDailyStats();
    void onWampSession( Wamp::Session * sess );
private:
    Wamp::Session * session;

};

#endif // STATSNOTIFIER_H
