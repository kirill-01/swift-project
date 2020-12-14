#ifndef BALANCESKEEPER_H
#define BALANCESKEEPER_H

#include <QObject>
#include <QSharedDataPointer>

#include <wampclient.h>
#include <swiftcore.h>
#include "../swift-corelib/assetsstorage.h"

class BalancesKeeperData;

class BalancesKeeper : public QObject
{
    Q_OBJECT
public:
    explicit BalancesKeeper(QObject *parent = nullptr);
    BalancesKeeper(const BalancesKeeper &);
    BalancesKeeper &operator=(const BalancesKeeper &);
    ~BalancesKeeper();

    void setBalanceRequired( const quint32& cid, const double& amount );

    double getBalanceRequired( const quint32& cid ) const;


    void setBalanceAvailable( const quint32& cid, const double& amount );

    double getBalanceAvailable( const quint32& cid ) const;


    void setBalanceTotal( const quint32& cid, const double& amount );

    double getBalanceTotal( const quint32& cid ) const;
signals:
    void updateEvent( const QJsonObject& j_data );
    void depositsEvent( const QJsonObject& j_data );
    void withdrawsEvent( const QJsonObject& j_data );
public slots:
    void calculateRequirements();

    void onUpdateEvent( const QJsonObject& j_data );
    void onDepositsEvent( const QJsonObject& j_data );
    void onWithdrawsEvent( const QJsonObject& j_data );

    void requestBals();
    void requestWithdraws();
    void requestDeposits();
    void onWampSession( Wamp::Session * sess );
    void checkPauseCancel();
private:

    void pauseModule();

    QStringList getConnectedExchanges();

    QStringList _active_clients;
    QTimer * update_bals_timer;
    QTimer * update_deposits_timer;
    QTimer * update_withdraws_timer;
    bool use_only_exchange_accs;

    Wamp::Session * session;
    QSharedDataPointer<BalancesKeeperData> data;
    QTimer * requirements_timer;

    bool is_active_module;
    QMutex m;
};

#endif // BALANCESKEEPER_H
