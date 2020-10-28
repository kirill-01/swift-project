#include "hostinfocollector.h"

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
static struct sysinfo memInfo;


HostInfoCollector::HostInfoCollector(QObject *parent) : QObject(parent)
{

}

void HostInfoCollector::collectInfo() {
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
        &lastTotalSys, &lastTotalIdle);
    fclose(file);

    sysinfo (&memInfo);

    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;

    const double ram_usage_percent( double( virtualMemUsed / memInfo.totalram * 100 ) );
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

double HostInfoCollector::getCurrentValue(){
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
