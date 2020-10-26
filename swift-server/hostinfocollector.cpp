#include "hostinfocollector.h"

HostInfoCollector::HostInfoCollector(QObject *parent) : QObject(parent)
{
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
        &lastTotalSys, &lastTotalIdle);
    fclose(file);
}
