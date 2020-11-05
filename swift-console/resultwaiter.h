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
    void saveHistory() {
        QFile histfile("/opt/swift-bot/console.history");
        if ( histfile.open( QFile::ReadWrite ) ) {
            histfile.resize(0);
            QString res;
            for( auto it = last_commands.begin(); it != last_commands.end(); it++ ) {
                res += *it + "\n";
            }
            histfile.write( res.toUtf8() );
            histfile.close();
        }
    }
    void loadHistory() {
        QFile histfile("/opt/swift-bot/console.history");
        if ( histfile.open( QFile::ReadOnly ) ) {

            QString res( histfile.readAll().constData() );
            const QStringList itms( res.split("\n") );
            last_commands.clear();
            for( auto it = itms.begin(); it != itms.end(); it++ ) {
                last_commands.push_back( *it );
            }
            histfile.close();
        }
    }
private:
    QString current_command;
    Wamp::Session * session;
    QList<quint64> _asyncs;
    QVector<QString> last_commands;
    quint32 history_index;

};

#endif // RESULTWAITER_H
