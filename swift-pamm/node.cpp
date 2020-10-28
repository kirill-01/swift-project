#include "node.h"

Node::Node(QObject *parent) : QObject(parent)
{
    name = SwiftBot::moduleParam("node_name","").toString();
    type = SwiftBot::moduleParam("node_type","public").toString();
    domain = SwiftBot::moduleParam("domain","").toString();
    controller_secret = SwiftBot::moduleParam("controller_secret","").toString();
    controller_url = SwiftBot::moduleParam("controller_url","").toString();
    reports_interval = SwiftBot::moduleParam("reports_interval", 30000).toUInt();
    max_invest = SwiftBot::moduleParam("max_invests",250000).toDouble();
    telegram_chat_id = SwiftBot::moduleParam("telegram_chat_id","").toString();
}
