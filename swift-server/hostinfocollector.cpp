#include "hostinfocollector.h"



HostInfoCollector::HostInfoCollector(QObject *parent) : QObject(parent)
{

}

void HostInfoCollector::collectInfo() {
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
        &lastTotalSys, &lastTotalIdle);
    fclose(file);

    // int sysinfo(struct sysinfo &memInfo);

    struct sysinfo memInfo;
    if (sysinfo(&memInfo) < 0) {
        return;
    }


    const quint64 total_ram = memInfo.totalram;
    const quint64 free_ram = memInfo.freeram;
    const quint64 used_ram = total_ram - free_ram;

    const double ram_usage_percent = double( used_ram) / double(total_ram) * 100;
    last_ram_usage = ram_usage_percent;
    // qWarning() << "Memory usage: " << QString::number( ram_usage_percent, 'f', 2 );
   if ( ram_usage_percent >= SwiftBot::appParam("danger_mem_usage_pcnt", 70).toUInt() ) {
        SwiftBot::addError( "Memory usage:"+QString::number( ram_usage_percent, 'f', 2 ), "CRITICAL");
    }
    const double cpuusage( getCurrentValue() );
    last_cpu_usage = cpuusage;
    if ( cpuusage > SwiftBot::appParam("danger_cpu_usage_pcnt", 60.05).toDouble() ) {
        SwiftBot::addError( "CPU usage:"+QString::number( cpuusage, 'f', 2 ), "CRITICAL");
        //qWarning() << "cpu usage" << getCurrentValue();
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

    total = totalUser + totalUserLow + totalSys;
    percent = total;
    total += totalIdle;
    percent /= total;
    percent *= 100;

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}
