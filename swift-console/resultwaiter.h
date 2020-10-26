#ifndef RESULTWAITER_H
#define RESULTWAITER_H

#include <QObject>
#include "../swift-corelib/wampclient.h"
#include "../swift-corelib/swiftcore.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCoreApplication>

#include <iostream>

class ResultWaiter : public QObject
{
    Q_OBJECT
public:
    explicit ResultWaiter(QObject *parent = nullptr);
    void waitFor( const quint64& uid );

signals:
    void resultReceived( const QString& );
public slots:
    void onWampSession( Wamp::Session * sess );
    void onKeyPress( char key );
private:
    QString current_command;
    Wamp::Session * session;
    QList<quint64> _asyncs;
    QVector<QString> last_commands;
    quint32 history_index;

};

#endif // RESULTWAITER_H
