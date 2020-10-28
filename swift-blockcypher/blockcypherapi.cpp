#include "blockcypherapi.h"

BlockCypherApi::BlockCypherApi(QObject *parent) : QObject(parent), netman( new QNetworkAccessManager(this) )
{
    connect( netman, &QNetworkAccessManager::finished, this, &BlockCypherApi::onNetworkReply );
    token = SwiftBot::moduleParam("api_token","").toString();
}
