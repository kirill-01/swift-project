#ifndef HOSTINFOCOLLECTOR_H
#define HOSTINFOCOLLECTOR_H

#include <QObject>
#include <swiftcore.h>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"


class HostInfoCollector : public QObject
{
    Q_OBJECT
public:
    explicit HostInfoCollector(QObject *parent = nullptr);

    double getLastCpu() const {
        return last_cpu_usage;
    }
    double getLastRam() const {
        return last_ram_usage;
    }
    unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
signals:
public slots:
    void collectInfo();

    double getCurrentValue();
private:
    double last_cpu_usage;
    double last_ram_usage;

};

#endif // HOSTINFOCOLLECTOR_H
