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
#include "../swift-corelib/swiftcore.h"
#include <QTimer>

#include "../swift-corelib/wampclient.h"

#include "../swift-corelib/assetsstorage.h"

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
    QJsonObject toJson() const {
        QJsonObject j;
        j["sell_pair"] = QString::number( sell_pair );
        j["buy_pair"] = QString::number( buy_pair );
        j["min_size"] = QString::number( min_size,'f',4 );
        j["max_size"] = QString::number( max_size,'f',4  );
        j["min_profit"] = QString::number( min_profit,'f',4  );
        j["max_profit"] = QString::number( max_profit ,'f',4 );
        j["weight"] = QString::number( weight ,'f',2);
        j["roi"] = QString::number( weight ,'f',4);
        j["spread"] = QString::number( weight ,'f',4);
        j["events_count"] = QString::number( events_count );
        return j;
    }
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
