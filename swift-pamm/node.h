#ifndef NODE_H
#define NODE_H

#include <QObject>

#include <swiftbot.h>
#include <swiftcore.h>
/*
 *
 * node_type=public|private
node_name=swiftbot
node_description=Official node
node_domain=node.swiftbot.space
node_controller_secret=secretphrase
node_controller_url=https://controller.swiftbot.space
reports_interval=30000
max_invest=
telegram_chat_id=


 * */
class Node : public QObject
{
    Q_OBJECT
public:
    QJsonObject toJson() {
        QJsonObject j;
        j["name"] = name;
        j["type"] = type;
        j["description"] = name;
        j["controller_url"] = controller_url;
        j["max_invest"] = QString::number( max_invest, 'f', 2 );
        QJsonArray arbpairs;
        SwiftBot::ArbitragePair::eachPair([&arbpairs]( SwiftBot::ArbitragePair pair ) {
            arbpairs.push_back(QJsonObject({{"name", pair.name }}));
        });
        j["pairs"] = arbpairs;
        QJsonArray exchanges;
        SwiftBot::Exchange::forEachExchanges([&exchanges]( SwiftBot::Exchange exch ) {
            exchanges.push_back(QJsonObject({{"name", exch.name }}));
        });
        j["exchanges"] = exchanges;
        return j;
    }
    explicit Node(QObject *parent = nullptr);
    QString name;
    QString type;
    QString domain;
    QString controller_secret;
    QString controller_url;
    quint64 reports_interval;
    double max_invest;
    QString telegram_chat_id;
signals:

};

#endif // NODE_H
