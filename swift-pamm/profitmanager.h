#ifndef PROFITMANAGER_H
#define PROFITMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QMap>

class ProfitManager : public QObject
{
    Q_OBJECT
public:
    explicit ProfitManager(QObject *parent = nullptr);

signals:
public slots:

    void shareProfit() {

    }

    void calculateProfit() {
        // Profit calculation
    }
};

#endif // PROFITMANAGER_H
