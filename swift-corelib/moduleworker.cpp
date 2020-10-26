#include "moduleworker.h"

ModuleWorker::ModuleWorker(QObject *parent) : QObject(parent)
{

}

SwiftBot::AssetsData ModuleWorker::prepareAssets(const QVariantList &v) {
    const QJsonObject j_data( QJsonDocument::fromJson( v.at(0).toString().toUtf8() ).object() );

    SwiftBot::AssetsData to_store_data;

    // to_store_data._currencies =
    return to_store_data;
}

OrderBooks ModuleWorker::prepareOrderbooks(const QVariantList &v) {
    const QJsonArray j_asks( QJsonDocument::fromJson( v.at(0).toString().toUtf8() ).object().value("asks").toArray() );
    OrderBookQuotes _asks;
    for( auto itp = j_asks.begin(); itp != j_asks.end(); itp++ ) {
        const QJsonArray j_itms( itp->toArray() );
        for( auto it = j_itms.begin(); it != j_itms.end(); it++ ) {
            _asks[ it->toArray().at(0).toString().toUInt() ].insert( it->toArray().at(1).toString().toDouble(), it->toArray().at(2).toString().toDouble() );
        }
    }
    OrderBookQuotes _bids;
    const QJsonArray j_bids( QJsonDocument::fromJson( v.at(0).toString().toUtf8() ).object().value("bids").toArray() );
    for( auto itp = j_bids.begin(); itp != j_bids.end(); itp++ ) {
        const QJsonArray j_itms( itp->toArray() );
        for( auto it = j_itms.begin(); it != j_itms.end(); it++ ) {
            _bids[ it->toArray().at(0).toString().toUInt() ].insert( it->toArray().at(1).toString().toDouble(), it->toArray().at(2).toString().toDouble() );
        }
    }
    return OrderBooks({_asks, _bids} );
}
