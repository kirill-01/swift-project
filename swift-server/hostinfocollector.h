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




signals:
public slots:
    void collectInfo();

    double getCurrentValue();

};

#endif // HOSTINFOCOLLECTOR_H
