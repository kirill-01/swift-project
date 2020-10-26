#ifndef HOSTINFOCOLLECTOR_H
#define HOSTINFOCOLLECTOR_H

#include <QObject>
#include "../swift-corelib/swiftcore.h"
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
static struct sysinfo memInfo;
#define HOSTINFO_BINARY "/usr/bin/hostinfo.sh"
#define HOSTINFO_STATUS_FILE "/opt/swift-bot/system_status.json"

class HostInfoCollector : public QObject
{
    Q_OBJECT
public:
    explicit HostInfoCollector(QObject *parent = nullptr);



signals:
public slots:
    void collectInfo() {


        sysinfo (&memInfo);

        long long virtualMemUsed = memInfo.totalram - memInfo.freeram;

        //Add other values in next statement to avoid int overflow on right hand side...
        virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
        virtualMemUsed *= memInfo.mem_unit;

        const double ram_usage_percent( double(virtualMemUsed / ( memInfo.totalram * memInfo.mem_unit ) ) * 100 );
        qWarning() << "Memory usage: " << QString::number( ram_usage_percent, 'f', 2 );
        if ( ram_usage_percent >= 90 ) {
            qWarning() << "WARNING! HIGH MEMORY USAGE!!";
        }
        const double cpuusage( getCurrentValue() );
        if ( cpuusage > 50 ) {
            qWarning() << "cpu usage" << getCurrentValue();
        }
        QTimer::singleShot( SwiftCore::getSettings()->value("hostinfo_interval", 5000 ).toUInt(), this, &HostInfoCollector::collectInfo );

    }

    double getCurrentValue(){
        double percent;
        FILE* file;
        unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

        file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
            &totalSys, &totalIdle);
        fclose(file);

        if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
            totalSys < lastTotalSys || totalIdle < lastTotalIdle){
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else{
            total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                (totalSys - lastTotalSys);
            percent = total;
            total += (totalIdle - lastTotalIdle);
            percent /= total;
            percent *= 100;
        }

        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;

        return percent;
    }

};

#endif // HOSTINFOCOLLECTOR_H
