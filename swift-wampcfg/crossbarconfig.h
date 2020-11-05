#ifndef CROSSBARCONFIG_H
#define CROSSBARCONFIG_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#define EXCHS_LIST "bittrex bitfinex binance hitbtc zb kucoin huobi kraken idcm livecoin cexio"
class CrossbarConfig : public QObject
{
    Q_OBJECT
public:
    explicit CrossbarConfig(QObject *parent = nullptr);
    static QJsonObject getRuleObj( const QString& uri );

    static QJsonArray getPermissions( const QStringList& allow_call, const QStringList & allow_register,const QStringList& allow_publish, const QStringList & allow_subscribe );
    QJsonObject _root;
    QJsonObject _controller;
    QJsonArray _workers;
    QJsonObject _worker;
    QJsonArray _worker_realms;
    QJsonArray _worker_realms_roles;

    QJsonObject j_web;
    QJsonArray _worker_transports;
    QJsonObject _worker_transports_auth;
    QJsonObject _worker_transports_auth_wampcra;
    QJsonObject _worker_transports_auth_wampcra_users;

    void loadConfig( const QString & config_filename );

    void addUser( const QString& username, const QJsonObject & j_user );

    void addRole( const QJsonObject & j_role );

    QJsonObject getConfig();

    void saveConfig( const QString & config_filename );

signals:
private:



};

#endif // CROSSBARCONFIG_H
