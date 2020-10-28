#ifndef BLOCKCYPHERAPI_H
#define BLOCKCYPHERAPI_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSqlQuery>
#include <QSqlError>
#include <swiftbot.h>
#include <swiftcore.h>


class BlockCypherApi : public QObject
{
    Q_OBJECT
public:
    explicit BlockCypherApi(QObject *parent = nullptr);
    QString createAddress( const quint32 & coin_id ) {
        // 15a23adfbf8e4f5ba2b7f5d42e744a08
        // curl -X POST https://api.blockcypher.com/v1/eth/main/addrs
        QEventLoop loop;
        QString ret("");
        connect( this, &BlockCypherApi::addressCreated, [&ret, &loop]( const QString& address) {
            ret = address;
            loop.quit();
        });
        SwiftBot::Coin coin( coin_id );
        QNetworkRequest request( QUrl("https://api.blockcypher.com/v1/"+coin.name.toLower()+"/main/addrs?token="+token));
        QNetworkReply * rep = netman->post( request, "" );
        rep->setProperty("method", "create");
        rep->setProperty("coin_id", coin_id);
        loop.exec();
        qWarning() << ret << "Address generated";
        return ret;
    }

    double getBalance( const quint32 & address_id ) {
         QEventLoop loop;
         double ret = 0;
         connect( this, &BlockCypherApi::balanceReceived, [&ret, &loop]( const double& bal ) {
             ret = bal;
             loop.quit();
         });
        QSqlQuery q("select * from cypher_addresses WHERE id="+QString::number( address_id ) );
        if ( q.exec() && q.next() ) {
            const quint32 coin_id = q.value("coin_id").toUInt();
            const QString address = q.value("address").toString();
            SwiftBot::Coin coin( coin_id );
            QNetworkRequest request( QUrl( "https://api.blockcypher.com/v1/"+coin.name.toLower()+"/main/addrs/"+address+"/balance" ) );
            QNetworkReply * rep = netman->post( request, "" );
            rep->setProperty("method", "balance");
            rep->setProperty("address", address);
        }
        loop.exec();
        return ret;
    }

    bool withdraw( const quint32& address_id, const QString& toAddress, const double & amount ) {

        // IF ETH
        // curl -sd '{"inputs":[{"addresses": ["add42af7dd58b27e1e6ca5c4fdc01214b52d382f"]}],"outputs":[{"addresses": ["884bae20ee442a1d53a1d44b1067af42f896e541"], "value": 4200000000000000}]}' https://api.blockcypher.com/v1/eth/main/txs/new?token=YOURTOKEN
        //  sign ./signer a83f5bea598e0d217a03a2646d6c49edb2e99daf4537b2c09b008df76b77acec $PRIVATEKEY
        // 3045022100bc04ce017622f9830f955dbd8fafb65c5a72306a674711e507200f5f198954c90220581bc05e2658c258a985d914a158f89f44144a2e082837955b218d12a43a6a38

        // curl -sd '{"tx": {...}, "tosign": [ "a83f5bea598e0d217a03a2646d6c49edb2e99daf4537b2c09b008df76b77acec" ], "signatures": [ "3045022100bc04ce017622f9830f955dbd8fafb65c5a72306a674711e507200f5f198954c90220581bc05e2658c258a985d914a158f89f44144a2e082837955b218d12a43a6a38" ]}' https://api.blockcypher.com/v1/eth/main/txs/send?token=YOURTOKEN


        // IF BTC
        // curl -d '{"inputs":[{"addresses": ["CEztKBAYNoUEEaPYbkyFeXC5v8Jz9RoZH9"]}],"outputs":[{"addresses": ["C1rGdt7QEPGiwPMFhNKNhHmyoWpa5X92pn"], "value": 1000000}]}' https://api.blockcypher.com/v1/btc/main/txs/new
        return false;
    }
signals:
    void addressCreated( const QString& address );
    void balanceReceived( const double& balance );
public slots:
    void onNetworkReply( QNetworkReply * reply ) {
        QString method = reply->property("method").toString();
        const QByteArray data( reply->readAll().constData() );
        const QJsonObject j_reply( QJsonDocument::fromJson( data ).object() );
        qWarning() << j_reply;
        if ( method == "create" ) {
            if ( j_reply.contains("address")  ) {
                QSqlQuery q("INSERT INTO cypher_addresses (coin_id, address, pubkey, seckey) VALUES ("+QString::number(reply->property("coin_id").toUInt() )+",'"+j_reply.value("address").toString()+"','"+j_reply.value("private").toString()+"','"+j_reply.value("public").toString()+"')");
                if ( !q.exec() ) {
                    qWarning() << q.lastError().text();
                    emit addressCreated( "false" );
                    return;
                }
                emit addressCreated( j_reply.value("address").toString( ) );
            } else {
                emit addressCreated( j_reply.value("address").toString( ) );
            }
        } else if ( method == "balance" ) {
            const double balance = j_reply.value("final_balance").toVariant().toULongLong() / 100000000;
            emit balanceReceived( balance );
        }
        reply->deleteLater();
    }
private:

    QString token;
    QNetworkAccessManager * netman;
};

#endif // BLOCKCYPHERAPI_H
