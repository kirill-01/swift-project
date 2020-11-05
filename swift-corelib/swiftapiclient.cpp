#include "swiftapiclient.h"

SwiftApiClient::SwiftApiClient(QObject *parent) : SwiftWorker(parent),
    limits_timer(new QTimer() ),
    wamp_connected(false),
    _queues_timer(new QTimer() ),
    _state_timer( new QTimer()),
    netman( new QNetworkAccessManager(this) ),
    _uuids_counter( QDateTime::currentMSecsSinceEpoch() ) ,
    request_pause(false)
{
    debug = SwiftCore::getSettings()->value(SETTINGS_NAME_API_DEBUG, false ).toBool();
    _requests_debug_log = SwiftCore::getSettings()->value(SETTINGS_NAME_API_DEBUG, false ).toBool();
    qRegisterMetaType<SwiftApiClient::AsyncMethods>("SwiftApiClient::AsyncMethods");
    connect( netman, &QNetworkAccessManager::finished, this, &SwiftApiClient::onNetworkReply );
}

QString SwiftApiClient::getTopic(const SwiftApiClient::WampTopics &topic) const {
    if ( topic == ApiRpcAsyncResults ) {
        return FEED_ASYNC_RESULTS;
    } else if ( topic == LoggerLogs ) {
        return RPC_SERVER_LOGGER_LOGS ;
    }  else if ( topic == LoggerErrs ) {
        return RPC_SERVER_LOGGER_ERRORS;
    }
    return FEED_ASYNC_RESULTS;
}

QString SwiftApiClient::exchangeParamsPath(const QString &exchange_name, const QString &param) {
    return exchange_name+"/"+param;
}

bool SwiftApiClient::responseSuccess(QJsonObject *result) const {
    if ( result->contains("success") ) {
        if ( result->value("success").isBool() ) {
            return result->value("success").toBool();
        }
        return ( result->value("success").toString() == "true" || result->value("success").toString() == "TRUE" );
    }
    return false;
}

QString SwiftApiClient::getMethodName(const SwiftApiClient::AsyncMethods &method) {
    static QMap<SwiftApiClient::AsyncMethods,QString> _methods;
    if ( _methods.isEmpty() ) {
        _methods[TimeSyncMethod] = "synctime";
        _methods[GetCurrencies] = "currencies";
        _methods[GetMarkets] = "markets";
        _methods[GetOrderbooks] = "orderbooks";
        _methods[OrderPlace] = "order.create";
        _methods[OrderCancel] = "order.cancel";
        _methods[OrderGet] = "order.get";
        _methods[WithdrawHistory] = "withdraw.history";
        _methods[WithdrawCreate] = "withdraw.create";
        _methods[WithdrawInner] = "withdraw.inner";
        _methods[WithdrawGetFee] = "withdraw.fee";
        _methods[GetBalances] = "balances";
        _methods[GetDeposits] = "deposits.history";
        _methods[GetDepositAddress] = "deposits.address";
        _methods[TradeHistory] = "trade.history";
        _methods[TradeOpenOrders] = "trade.active";
        _methods[TradeGetFee] = "trade.fee";
    }
    return _methods.value( method, "unknown" );
}

quint32 SwiftApiClient::getExchangeId() {
    static quint32 _id = 0;
    if ( _id == 0 ) {
        SwiftCore::getAssets()->loadAssets();
        _id = SwiftCore::getAssets()->getExchangeId( getExchangeName() );
    }
    return _id;
}

QString SwiftApiClient::getApiVersionString() {
    return QCoreApplication::applicationVersion();
}

QString SwiftApiClient::getExchangeApiKey() const {
    return SwiftBot::moduleParam( SETTINGS_NAME_API_KEY,"").toString().trimmed();
}

QString SwiftApiClient::getExchangeApiSecret() const {
    return SwiftBot::moduleParam( SETTINGS_NAME_API_SECRET,"").toString().trimmed();
}

QString SwiftApiClient::getExchangeApiAdditional() const {
    return SwiftBot::moduleParam( SETTINGS_NAME_API_USER,"").toString().trimmed();
}

void SwiftApiClient::customParser(const SwiftApiClient::AsyncMethods &method, const QJsonObject &j_result) {
    Q_UNUSED(j_result)
    Q_UNUSED(method)
}

void SwiftApiClient::customMethod(const QJsonObject &j_params, const quint64 &async_uuid) {
    Q_UNUSED(j_params)
    Q_UNUSED(async_uuid)
}

/**
 * @brief SwiftApiClient::createAsyncWaiter
 *
 * Registering request, assing and return unique id
 * @param method
 * @param j_params
 * @return
 */
quint64 SwiftApiClient::createAsyncWaiter(const SwiftApiClient::AsyncMethods &method, const QJsonObject &j_params = QJsonObject() ) {
    // Allow call public methods without api keys
    if ( method != AsyncMethods::GetMarkets && method != AsyncMethods::GetOrderbooks && method != AsyncMethods::GetCurrencies ) {
        if ( api_key.isEmpty() || api_key == "" ) {
            addError("Api key or secret is empty: " + getExchangeName() + " Key: " + getExchangeApiKey()+ " Secret: " + getExchangeApiSecret() );
            return 1;
        }
    }
    if ( request_pause ) {
        return 2;
    }

    const quint64 uuid = registerAsyncCall( method );
    if ( method == SwiftApiClient::AsyncMethods::OrderPlace ) {
        registerOrder( j_params, uuid );
    }
    _async_params.insert( uuid, j_params );
    _async_queue.enqueue( uuid );
    if ( isApiDebug() ) {
        qWarning() << getExchangeName() << getMethodName( method ) << "ASYNC REQUEST REGISTERED" << uuid << "PARAMS" << j_params;
    }
    if ( isApiDebug() ) {
        addLog( QString::number( uuid ) + " : " +  getMethodName( method ) + " : " + QJsonDocument( j_params ).toJson( QJsonDocument::Compact ) );
    }
    return uuid;
}

QNetworkAccessManager *SwiftApiClient::getManager() {
    return netman;
}

void SwiftApiClient::onWampSession_(Wamp::Session *session ) {

    delay_between_requests = getSettings()->value( SETTINGS_NAME_EXCHANGE_DELAY_REQUESTS, 50 ).toUInt();
    delay_between_same_requests = getSettings()->value( SETTINGS_NAME_EXCHANGE_DELAY_SAME_REQUESTS, 500 ).toUInt();

    // Updating settings from worker
    api_key = getExchangeApiKey();
    api_secret = getExchangeApiSecret();
    api_user = getExchangeApiAdditional();


    connect( this, &SwiftApiClient::pubWamp, session, &Wamp::Session::publishMessage, Qt::QueuedConnection );
    wamp_connected = true;
    _state_timer->setInterval( 25000 );
    limits_timer->setInterval( 60000 );
    _req_total_limit = 60;
    _req_total_counter = 0;

    connect( limits_timer, &QTimer::timeout, this, &SwiftApiClient::resetLimits );
    limits_timer->start();
    _queues_timer->setInterval( 250 );

    connect( _queues_timer, &QTimer::timeout, this, &SwiftApiClient::processAsyncQueue );
    connect( _queues_timer, &QTimer::timeout, this, &SwiftApiClient::processAsyncQueueResult );
    _queues_timer->start();
    _started_at = QDateTime::currentDateTime();
    _last_request_at = QDateTime::currentDateTime();
    connect( this, &SwiftApiClient::apiParsedResponse, this, &SwiftApiClient::onApiResponseParsed );


    addLog( "Init exchange methods ["+getExchangeName()+"]: ");
    for (int i = 0; i <= 16; i++)
    {
        const SwiftApiClient::AsyncMethods met( static_cast<SwiftApiClient::AsyncMethods> ( i ) );
        const QString rpc_path( "swift.api."+getMethodName( met )+"."+getExchangeName() );
        session->provide( rpc_path, [=](const QVariantList &args, const QVariantMap &kwargs ) {
                Q_UNUSED(kwargs)
                if ( args.isEmpty() ) {
                    const QJsonObject j_params;
                    const quint64 uuid =  createAsyncWaiter( met, j_params );
                    return uuid;
                } else {
                    const QJsonObject j_params( QJsonDocument::fromJson( args.at(0).toString().toUtf8() ).object() );
                    const quint64 uuid =  createAsyncWaiter( met, j_params );
                    return uuid;
                }
            });
    }

    session->provide("swift.api.status."+getExchangeName(), [=](const QVariantList &args, const QVariantMap &kwargs ) {
        Q_UNUSED(args)
        Q_UNUSED(kwargs)
        QJsonObject j_ret;
        j_ret["exchange_id"] = QString::number( getExchangeId() );
        j_ret["public_methods"] = true;
        j_ret["private_methods"] = ( !api_key.isEmpty() && !api_secret.isEmpty() );
        const QString ret( QJsonDocument( j_ret ).toJson( QJsonDocument::Compact ) );
        return ret;
    });

    addLog("Api client connected to wamp. RPC Methods registered");
    if ( getExchangeApiKey().isEmpty() || getExchangeApiKey() == "" ) {
        addError("Api key or secret is empty: " + getExchangeName() + " Key: " + getExchangeApiKey()+ " Secret: " + getExchangeApiSecret(), "APIKEYS" );
        QString msg("<em><b>"+getExchangeName()+"</b></em> - Api key is not set!\nClient can execute only public methods.\nPlease, provide correct params inside config file:\n\n<b>/opt/swift-bot/modules/"+getExchangeName()+"/"+getExchangeName()+".ini</b>\n\nYou can simply disable this exchange client module =)");
        session->call( RCP_TELEGRAM_NOTIFY, {msg} );
    }

    // Check exchange assets
    if ( SwiftCore::getAssets()->getExchangeId( getExchangeName() ) == 0 ) {
        addLog("Exchange not inited");
        // Exchange not in database
        QSqlQuery q("LOCK TABLES exchanges WRITE; INSERT INTO exchanges (name,is_enabled) VALUES ('"+getExchangeName()+"', TRUE) ON DUPLICATE KEY UPDATE name='"+getExchangeName()+"'; UNLOCK TABLES;");
        if ( !q.exec() ) {
            addError( q.lastError().text() );
        }
        addLog(  "Assigned exchange id: " + QString::number( SwiftCore::getAssets(true)->getExchangeId( getExchangeName()  ) ) );
        addLog("Receive currencies assets info");
        createAsyncWaiter( SwiftApiClient::GetCurrencies, QJsonObject() );
        SwiftCore::getAssets(true);
        QTimer::singleShot( 3000, this,[=](){
            addLog("Receive pairs assets info");
            createAsyncWaiter( SwiftApiClient::GetMarkets , QJsonObject() );
        });

    }



}

void SwiftApiClient::onNetworkReply(QNetworkReply *reply) {
    const quint64 rep_uuid = reply->property("uuid").toString().toULongLong();
    const QByteArray data( reply->readAll().constData() );

    if ( !reply->property("customRequest").isNull() ) {
        emit customRequestResult( reply->property("customRequest").toString(), data );
    } else {
        QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
        const quint32 http_code = statusCode.toUInt();
        if ( http_code != 200 && http_code != 201) {
            if ( http_code == 521 ) {
                // Server down
                addError("API response 521 - Freeze requestst and try again later");
                request_pause = true;
                QTimer::singleShot( 30000, this, &SwiftApiClient::unfreeze );
                return;
            } else if ( http_code == 400 ) {
                // Request error
                if ( isDebug() ) {
                    addError( getExchangeName() + http_code + data );
                }
                addError("API response 400 - Bad request" + QString::number( _async_dictionary.value( rep_uuid ) ) + data );
               // request_pause = true;
               // QTimer::singleShot( 30000, this, &SwiftApiClient::unfreeze );
                return;
            } else if ( http_code == 403 ) {
                if ( isDebug() ) {
                   addError( getExchangeName() + http_code + data );
                }
                 const SwiftApiClient::AsyncMethods method( _async_dictionary.value(rep_uuid) );
                addError("API response 403 " +  getMethodName( method ) + "- Freeze requestst and try again later: "+ data);
                request_pause = true;
                QTimer::singleShot( 30000, this, &SwiftApiClient::unfreeze );
                QJsonObject jret;
                jret["success"] = false;
                jret["async_uuid"] = QString::number( rep_uuid );
                jret["nonce"] = QString::number( rep_uuid );
                jret["method"] = getMethodName( method );
                jret["exchange"] = getExchangeName();
                jret["error"] = QString::number( http_code );
                onApiResponseParsed( rep_uuid, jret );
                return;
                // API KEY BROKE
            } else if ( http_code == 429 ) {
                if ( isDebug() ) {
                   addError( getExchangeName() + http_code + data );
                }
                addError("API response 429 - Freeze requestst and try again later");
                request_pause = true;
                QTimer::singleShot( 120000, this, &SwiftApiClient::unfreeze );
                return;
                // DDOS!
            } else {
                const SwiftApiClient::AsyncMethods method( _async_dictionary.value(rep_uuid) );


                    if ( isDebug() ) {
                        addError( getExchangeName() + http_code + data );
                    }

                QJsonObject jret;
                jret["success"] = false;
                jret["async_uuid"] = QString::number( rep_uuid );
                jret["nonce"] = QString::number( rep_uuid );
                jret["method"] = getMethodName( method );
                jret["exchange"] = getExchangeName();
                jret["error"] = QString::number( http_code );
                if ( isDebug() ) {
                    addError( getExchangeName() + getMethodName( method ) + "NETWORK WRONG RESPONSE RECEIVED: " + data );
                }
                onApiResponseParsed( rep_uuid, jret );
            }
        } else {

                const SwiftApiClient::AsyncMethods method( _async_dictionary.value(rep_uuid) );


                _last_request_at = QDateTime::currentDateTime();
                if ( reply->property("req_caching") == 1 ) {
                    _caching_targets[ rep_uuid ] = reply->property("market_id").toInt();
                }
                emit parseApiResponse( rep_uuid, method, data );
        }
    }
    reply->deleteLater();
}

void SwiftApiClient::processAsyncQueueResult() {
    if ( !_async_results_queue.isEmpty() ) {
        const quint64 next_uuid = _async_results_queue.dequeue();
        if ( !sendAsyncResult( next_uuid ) ) {
            if ( isDebug() ) {
                addError( getExchangeName() + QString::number( next_uuid ) + "CANT PROCESS ASYNC RESULT" );
            }
        }
    }
}

void SwiftApiClient::processAsyncQueue() {
    if ( !_async_queue.isEmpty() && !request_pause) {
        const quint64 next_uuid = _async_queue.dequeue();
        if ( _async_dictionary.contains( next_uuid ) ) {

            if ( QDateTime::currentMSecsSinceEpoch() - _last_request_ts <= delay_between_requests ) {
                _async_queue.enqueue( next_uuid );
                if ( isApiDebug() ) {
                    qWarning() << getExchangeName() <<"Delay request";
                }
                return;
            }

            const SwiftApiClient::AsyncMethods next_method( _async_dictionary.value( next_uuid ) );

            if ( QDateTime::currentMSecsSinceEpoch() - _last_methods_ts.value(next_method) <= delay_between_same_requests ) {
                _async_queue.enqueue( next_uuid );
                if ( isApiDebug() ) {
                    qWarning() << getExchangeName() << "Delay same request" << next_method;
                }
                return;
            }

            const QJsonObject j_params( _async_params.value( next_uuid ) );
            _req_total_counter++;
            _methods_counter[next_method] = QDateTime::currentSecsSinceEpoch();
            if (next_method == GetCurrencies) {
                getCurrencies( j_params, next_uuid );
            } else if (next_method == GetOrderbooks) {
                getOrderbooks( j_params, next_uuid );
            } else if (next_method == GetMarkets) {
                getMarkets( j_params, next_uuid );
            } else if (next_method == OrderPlace) {
                orderPlace(j_params, next_uuid );
            } else if (next_method == OrderCancel) {
                registerOrder( j_params, next_uuid  );
                orderCancel(j_params, next_uuid );
            } else if (next_method == OrderGet) {
                orderGet(j_params, next_uuid );
            } else if (next_method == WithdrawHistory) {
                withdrawHistory(j_params, next_uuid );
            } else if (next_method == WithdrawCreate) {
                withdrawCreate(j_params, next_uuid );
            } else if (next_method == WithdrawInner) {
                withdrawInner(j_params, next_uuid );
            } else if (next_method == WithdrawGetFee) {
                withdrawGetFee(j_params, next_uuid );
            } else if (next_method == GetBalances) {
                getBalances(j_params, next_uuid );
            } else if (next_method == GetDeposits) {
                getDeposits(j_params, next_uuid );
            } else if (next_method == GetDepositAddress) {
                getDepositAddress(j_params, next_uuid );
            } else if (next_method == TradeHistory) {
                tradeHistory(j_params, next_uuid );
            } else if (next_method == TradeOpenOrders) {
                tradeOpenOrders(j_params, next_uuid );
            } else if (next_method ==TradeGetFee) {
                tradeGetFee(j_params, next_uuid );
            } else if ( next_method == TimeSyncMethod ) {
                customMethod( j_params, next_uuid );
            } else {
                addError("Unknown method to process: " + QString::number( next_method ) + getMethodName( next_method ) );
            }
        }
    }
}

QJsonObject &  SwiftApiClient::parseBalancesGroup( QJsonObject  & j_result )  {
    QJsonArray j_arr( j_result.value("balances").toArray() );
    for( auto it = j_arr.begin(); it != j_arr.end(); it++ ) {
        setCachedBalance( it->toObject().value("currency_id").toString().toUInt(), it->toObject().value("total").toString().toDouble(), it->toObject().value("reserved").toString().toDouble() );
    }

    return j_result;
}



QJsonObject & SwiftApiClient::parseTradingGroup( QJsonObject & j_result)  {
    if ( j_result.contains("orders") && j_result.value("orders").isArray() && !j_result.value("orders").toArray().isEmpty() ) {
        // Here assigning local id by remote id
        QJsonArray _items( j_result.value("orders").toArray() );
        QJsonArray _items_updated;
        for( auto it = _items.begin(); it != _items.end(); it++ ) {
           QJsonObject _order_obj( it->toObject() );
           if ( _order_obj.contains("remote_id") && !_order_obj.contains("local_id") ) {
               if ( hasOrderRemoteIdRelation( _order_obj.value("remote_id").toString() ) ) {
                   _order_obj["local_id"] = getOrderRemoteIdRelation( _order_obj.value("remote_id").toString() );
               } else {
                   _order_obj["local_id"] = createOrderRemoteIdRelation( _order_obj.value("remote_id").toString() );
               }
           }
        }
        j_result["orders"] = _items_updated;
    } else {
        if ( j_result.contains( "remote_id" ) && !j_result.contains("local_id")  ) {
            const QString locid = getRegisteredOrder( j_result.value("async_uuid").toString().toULongLong() ).value("local_id").toString();
            j_result["local_id"] = locid;
            _orders_uids_relations.insert( j_result.value("remote_id").toString(), locid );
        }
    }
    return j_result;
}

QJsonObject & SwiftApiClient::parsePublicGroup( QJsonObject  & j_result ) {
    if ( j_result.contains("markets") && j_result.value("markets").isArray() && !j_result.value("markets").toArray().isEmpty() ) {

        const QStringList _skip_pairs_settings( SwiftCore::getSettings()->value(SETTINGS_NAME_SKIP_PAIRS,"").toString().split(",") );
        QStringList ins_parts;
        const QJsonArray j_items( j_result.value("markets").toArray() );
        for( auto it = j_items.begin(); it != j_items.end(); it++ ) {
            const quint32 base_currency_id = it->toObject().value("base_currency_id").toString().toUInt();
            const quint32 market_currency_id = it->toObject().value("market_currency_id").toString().toUInt();

            if ( base_currency_id > 0 && SwiftCore::getAssets()->isCurrencyActive( base_currency_id )
                 && market_currency_id > 0 && SwiftCore::getAssets()->isCurrencyActive( market_currency_id )
                 && getExchangeId() > 0 )
            {
                const QString basecoinname( SwiftCore::getAssets()->getCurrencyCoinName( it->toObject().value("base_currency_id").toString().toUInt() ));
                const QString marketcoinname( SwiftCore::getAssets()->getCurrencyCoinName( it->toObject().value("market_currency_id").toString().toUInt() ));
                const QString arbpairname( QStringList({basecoinname,marketcoinname}).join("-"));
                ins_parts.push_back("("+it->toObject().value("base_currency_id").toString()+","+it->toObject().value("market_currency_id").toString()+","+QString::number(getExchangeId())+",'"+it->toObject().value("name").toString()+"', "+QString(_skip_pairs_settings.contains(arbpairname )?"0":"1")+")");
            }
        }

        if ( !ins_parts.isEmpty() ) {
            QSqlQuery q( "LOCK TABLES pairs WRITE; UPDATE pairs SET `is_enabled`=0 WHERE `exchange_id`="+QString::number( getExchangeId() )+"; INSERT INTO pairs (`base_currency_id`,`market_currency_id`,`exchange_id`,`name`,`is_enabled`) VALUES "+ ins_parts.join(",") + " ON DUPLICATE KEY UPDATE `is_enabled`=VALUES(is_enabled); UNLOCK TABLES;" );
            if ( !q.exec() ) {
                qWarning() << q.lastError().text();
            }
        }
    } else if (  j_result.contains("currencies") && j_result.value("currencies").isArray() && !j_result.value("currencies").toArray().isEmpty() ) {
        QStringList ins_parts;
        QStringList default_enabled_names( SwiftCore::getSettings()->value(SETTINGS_NAME_DEFAULT_COINS,"").toString().split(",") );
        if ( default_enabled_names.contains("btc") ) {
            default_enabled_names.push_back("xbt");
            default_enabled_names.push_back("xxbt");
        }
        if ( default_enabled_names.contains("usdt") ) {
            default_enabled_names.push_back("usdt20");
        }
        if ( default_enabled_names.contains("eth") ) {
            default_enabled_names.push_back("xeth");
        }
        const QJsonArray j_items( j_result.value("currencies").toArray() );

        for( auto it = j_items.begin(); it != j_items.end(); it++ ) {
            const bool isactive = ( getExchangeId() > 0 && default_enabled_names.contains( it->toObject().value("name").toString().toLower() ) );
            ins_parts.push_back("('"+it->toObject().value("name").toString()+"',"+QString::number(getExchangeId())+","+QString::number( isactive ? 1 : 0 )+")");
        }
        if ( !ins_parts.isEmpty() ) {
            QSqlQuery q( "LOCK TABLES currencies WRITE; UPDATE currencies SET `is_enabled`=0 WHERE `exchange_id`="+QString::number( getExchangeId() )+"; INSERT INTO currencies (`name`,`exchange_id`,`is_enabled`) VALUES "+ ins_parts.join(",")+ " ON DUPLICATE KEY UPDATE `is_enabled`=VALUES(is_enabled); UNLOCK TABLES;" );
            if ( !q.exec() ) {
                addError( "MySQL ERROR: " + q.lastError().text() );
            }
        }
    }
    return j_result;
}

void SwiftApiClient::initAssets() {

}

void SwiftApiClient::onApiResponseParsed(const quint64 &uuid, const QJsonObject &j_result) {

    if ( _asyncs.contains( uuid ) ) {

        QJsonObject result( j_result );
        const SwiftApiClient::AsyncMethods method( _async_dictionary.value(uuid) );

        if ( !responseSuccess( &result ) ) {
            result["success"] = false;
            result["async_uuid"] = QString::number( uuid );
            result["method"] = getMethodName( method );
            if ( method == SwiftApiClient::AsyncMethods::OrderPlace ) {
                result["local_id"] = _regitered_orders.value( uuid );
                result["status"] = "4";
            }
            registerAsyncResult( uuid, result );
            addError( QJsonDocument( result ).toJson(QJsonDocument::Compact ) );
            return;
        }

        if ( method == SwiftApiClient::AsyncMethods::GetCurrencies
            || method == SwiftApiClient::AsyncMethods::GetMarkets
            || method == SwiftApiClient::AsyncMethods::GetOrderbooks
            || method == SwiftApiClient::AsyncMethods::TimeSyncMethod
             ) {


            if ( method == SwiftApiClient::AsyncMethods::GetOrderbooks  ) {
                const bool cachingorderbooks = getSettings()->value( SETTINGS_NAME_EXCHANGE_CACHE_ORDERBOOKS, false ).toBool();
                if( cachingorderbooks ) {
                    const QJsonObject casks( result.value("asks").toObject() );
                    const QJsonObject cbids( result.value("bids").toObject() );
                    const QList<QString> _pairs( casks.keys() );
                    for( auto it = _pairs.begin(); it != _pairs.end(); it++ ) {
                        _cached_orderbooks[ it->toUInt() ] = QJsonObject(
                        {
                             {"asks",casks.value(*it).toArray()},
                             {"bids",cbids.value(*it).toArray()}
                        });
                        _cached_orderbooks_timers[ it->toUInt() ] = QDateTime::currentMSecsSinceEpoch();
                    }
                    if ( _caching_targets.contains( uuid ) ) {
                        const quint32 targt = _caching_targets.value( uuid );
                        result["asks"] = _cached_orderbooks[ targt ].value("asks").toArray();
                        result["bids"] = _cached_orderbooks[ targt ].value("bids").toArray();
                    }
                }
            }

            parsePublicGroup( result );
        } else if ( method == SwiftApiClient::AsyncMethods::GetDeposits
            || method == SwiftApiClient::AsyncMethods::GetDepositAddress
            || method == SwiftApiClient::AsyncMethods::TradeGetFee
            || method == SwiftApiClient::AsyncMethods::WithdrawInner
            || method == SwiftApiClient::AsyncMethods::WithdrawCreate
            || method == SwiftApiClient::AsyncMethods::WithdrawHistory
            ) {
            parseBalancesGroup( result );

            if ( method == SwiftApiClient::AsyncMethods::WithdrawHistory  ) {
                raiseEvent( result.value("withdraws").toArray(), FEED_EVENTS_BALANCES, EVENTS_NAME_WITHDRAWS_HISTORY );
            } else if(  method == SwiftApiClient::AsyncMethods::GetDeposits ) {
                raiseEvent( result.value("deposits").toArray(), FEED_EVENTS_BALANCES, EVENTS_NAME_DEPOSITS_HISTORY );
            } else if ( method == SwiftApiClient::WithdrawCreate ) {
                raiseEvent( QJsonArray({result}), FEED_EVENTS_BALANCES, EVENTS_NAME_WITHDRAWS_CREATED );
            } else if ( method == SwiftApiClient::GetBalances ) {
                raiseEvent( result.value("balances").toArray(), FEED_EVENTS_BALANCES, EVENTS_NAME_BALANCES_UPDATE );
            }

        } else if ( method == SwiftApiClient::AsyncMethods::OrderGet
            || method == SwiftApiClient::AsyncMethods::OrderPlace
            || method == SwiftApiClient::AsyncMethods::OrderCancel
            || method == SwiftApiClient::AsyncMethods::TradeGetFee
            || method == SwiftApiClient::AsyncMethods::TradeHistory
            || method == SwiftApiClient::AsyncMethods::TradeOpenOrders
            ) {

            parseTradingGroup( result );

            // Rising order events
            if ( method == SwiftApiClient::AsyncMethods::OrderPlace ) {
                result["local_id"] = _regitered_orders.value( uuid );
                _orders_uids_relations[ result.value("remote_id").toString() ] = _regitered_orders.value( uuid );
                if ( result.value("status").toString().toUInt() == 1 ) {
                    raiseEvent( QJsonArray({result}), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDER_PLACED );
                } else if ( result.value("status").toString().toUInt() == 2 ) {
                    raiseEvent( QJsonArray({result}), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDER_COMPLETED );
                }
            } else if ( method == SwiftApiClient::AsyncMethods::OrderCancel ) {
                raiseEvent( QJsonArray({result}), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDER_CANCELED );
            } else if ( method == SwiftApiClient::AsyncMethods::OrderGet ) {
                if ( result.value("status").toString().toUInt() >=2 ) {
                    raiseEvent( QJsonArray({result}), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDER_COMPLETED );
                } else {
                    raiseEvent( QJsonArray({result}), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDER_UPDATED );
                }
            } else if ( method == SwiftApiClient::AsyncMethods::TradeHistory ) {
                raiseEvent( result.value("orders").toArray(), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDERS_HISTORY );
            } else if ( method == SwiftApiClient::AsyncMethods::TradeOpenOrders ) {
                raiseEvent( result.value("orders").toArray(), FEED_EVENTS_ORDERS, EVENTS_NAME_ORDERS_ACTIVE );
            }
        }

        result["success"] = true;
        result["exchange_id"] = QString::number( getExchangeId() );
        result["exchange_name"] = getExchangeName();
        result["async_uuid"] = QString::number( uuid );
        result["method"] = getMethodName( method );
        registerAsyncResult( uuid, result );
    }
}

void SwiftApiClient::customMethodParsers(const SwiftApiClient::AsyncMethods &method, const QJsonObject &j_result) {
    if ( method == SwiftApiClient::AsyncMethods::GetBalances ) {
        _cached_balances = j_result.value("balances").toArray();
        for( auto it = _cached_balances.begin(); it != _cached_balances.end(); it++ ) {
            for( auto it2 = _cached_currencies.begin(); it2 != _cached_currencies.end(); it2++ ) {
                if( it2->toObject().value("currency_id").toString().toUInt() == it->toObject().value("currency_id").toString().toUInt() ) {
                    QJsonObject re( it2->toObject() );
                    re["deposit_address"] = it->toObject().value("deposit_address").toString();
                    _cached_currencies.replace( it2.i, re );
                    break;
                }
            }
        }
    } else if ( method == SwiftApiClient::AsyncMethods::GetMarkets  ){
        _cached_markets = j_result.value("markets").toArray();
        for( auto it = _cached_markets.begin(); it != _cached_markets.end(); it++ ) {
            _trade_fees[ it->toObject().value("market_id").toString().toUInt() ] = it->toObject().value("trade_fee").toString().toDouble();
            _min_trade_sizes[ it->toObject().value("market_id").toString().toUInt() ] = it->toObject().value("min_trade_size").toString().toDouble();
        }
    } else if ( method == SwiftApiClient::AsyncMethods::GetCurrencies  ){
        _cached_currencies = j_result.value("currencies").toArray();
        for( auto it = _cached_currencies.begin(); it != _cached_currencies.end(); it++ ) {
            _withdraw_fees[ it->toObject().value("currency_id").toString().toUInt() ] = it->toObject().value("withdraw_fee").toString().toDouble();
        }
    } else if ( method == SwiftApiClient::AsyncMethods::TradeOpenOrders  ) {
        _cached_orders = j_result.value("orders").toArray();
    }
    customParser( method, j_result );
}

void SwiftApiClient::methodState(const SwiftApiClient::AsyncMethods &method, const bool &result) {
    const QString& prefix("swift.api."+getExchangeName()+".");
    if ( session != nullptr &&  session->isJoined() ) {
        session->call(RPC_API_METHODS_STATE, QVariantList({prefix+SwiftApiClient::getMethodName( method ), result}) );
    }
}

bool SwiftApiClient::publishWamp(const WampTopics &topic, const QJsonObject &obj, const quint64 &async_uuid) {
    if ( wamp_connected && session != nullptr && session->isJoined() ) {
        if ( async_uuid > 0 ) {
            QJsonObject obj2( obj );
            obj2["async_uuid"] = QString::number( async_uuid );
            const QString res( QJsonDocument( obj2 ).toJson( QJsonDocument::Compact ) );
            const QString topic_( getTopic( topic ) );
            session->publish( topic_, QVariantList{res});
        } else {
            const QString res( QJsonDocument( obj ).toJson( QJsonDocument::Compact ) );
            const QString topic_( getTopic( topic ) );
            session->publish( topic_, QVariantList{res});
        }
        return true;
    } else {
        const QString res( QJsonDocument( obj ).toJson( QJsonDocument::Compact ) );
        addError( "WAMP client not connected! : " + res );
        return false;
    }
}

void SwiftApiClient::addError(const QString &message, const QString& group) {
    insertError(group, message );
}

QJsonValue SwiftApiClient::updateCachedOrder(const QJsonArray &j_orders, const quint64 &uuid) const {
    QJsonArray ret;
    for( auto it = j_orders.begin(); it != j_orders.end(); it++ ) {
        ret.push_back( updateCachedOrder( it->toObject(), uuid ).toObject() );
    }
    return QJsonValue( ret );
}

QJsonValue SwiftApiClient::updateCachedOrder(const QJsonObject &j_result, const quint64 &uuid) const {
    Q_UNUSED( uuid )
    QJsonObject ret, j_cached_obj;
    if ( j_result.contains("local_id") ) {
    }
    return QJsonValue( ret );
}

void SwiftApiClient::raiseEvent(const QJsonArray &j_jtems, const QString &event_feed, const QString &event_name) {
    QJsonObject j_event({{"exchange_id",QString::number( getExchangeId() )},{"items", j_jtems}});
    const QString _event_data( QJsonDocument( j_event ).toJson( QJsonDocument::Compact ) );
    if ( session != nullptr && session->isJoined() ) {
        session->publish(event_feed, { event_name, _event_data } );
    } else {
        addError( "Cant ppublish event. WAMP not connected" );
    }
}

void SwiftApiClient::registerOrder(const QJsonObject &j, const quint64 &uuid) {
    _regitered_orders_objects[ j.value("local_id").toString() ] = j;
    _regitered_orders[uuid] =  j.value("local_id").toString();
}

QJsonObject SwiftApiClient::getRegisteredOrder(const quint64 &uuid) {
    if ( _regitered_orders.contains( uuid ) ) {
        return _regitered_orders_objects.take( _regitered_orders.take( uuid ) );
    } else {
        return QJsonObject();
    }
}

QJsonObject SwiftApiClient::getRegisteredOrder(const QString &local_uuid) {
    if ( _regitered_orders.values().contains( local_uuid ) ) {
        return getRegisteredOrder( _regitered_orders.key( local_uuid ) );
    } else if ( _regitered_orders_objects.contains( local_uuid ) ) {
        return _regitered_orders_objects.take( local_uuid );
    } else {
        return QJsonObject();
    }
}

bool SwiftApiClient::isDebug() {
    return isApiDebug();
}

void SwiftApiClient::setCachedBalance(const quint32 &currency_id, const double &total, const double &inner) {
    _balances_inner_cached[currency_id] = inner;
    _balances_total_cached[currency_id] = total;
}

double SwiftApiClient::getTotalCachedBalance(const quint32 &currency_id) const {
    return _balances_total_cached.value( currency_id, 0 );
}

double SwiftApiClient::getInnerCachedBalance(const quint32 &currency_id) const {
    return _balances_inner_cached.value( currency_id, 0 );
}

void SwiftApiClient::resetLimits() {
    _req_total_counter = 0;
}

void SwiftApiClient::unfreeze() {
    request_pause = false;
}

void SwiftApiClient::addLog(const QString& message, const QString& group) {
    insertLog(group, message );
}

quint64 SwiftApiClient::registerAsyncCall(const SwiftApiClient::AsyncMethods &method) {
    const quint64 uuid = getNextUuid();
    _asyncs.push_back( uuid );
    _async_dictionary.insert( uuid, method );
    return uuid;
}

QSettings *SwiftApiClient::getSettings() const {
    return SwiftCore::getModuleSettings( getExchangeName() );
}

bool SwiftApiClient::isApiDebug() {
    return SwiftBot::appParam( SETTINGS_NAME_API_DEBUG, false ).toBool();
}

bool SwiftApiClient::hasOrderRemoteIdRelation(const QString &remote_id) const {
    return _orders_uids_relations.contains( remote_id );
}

QString SwiftApiClient::getOrderRemoteIdRelation(const QString &remote_id) const {
    return _orders_uids_relations.value( remote_id );
}

QString SwiftApiClient::createOrderRemoteIdRelation(const QString &remote_id) {
    _orders_uids_relations[ remote_id ] = "loc"+QString::number( getExchangeId() )+remote_id;
    return getOrderRemoteIdRelation( remote_id );
}

quint64 SwiftApiClient::getNextUuid() {
    if ( QDateTime::currentMSecsSinceEpoch() - _uuids_counter > 300  )  {
        _uuids_counter =  QDateTime::currentMSecsSinceEpoch();
    }
    const quint64 last_assigned =  _uuids_counter;
    quint64 new_variant = ++_uuids_counter;
    if ( last_assigned < new_variant ) {
        return new_variant;
    } else {
        while ( last_assigned <= new_variant ) {
            new_variant = ++_uuids_counter;
        }
        return  ++_uuids_counter;
    }
}

void SwiftApiClient::registerAsyncResult(const quint64 &uuid, const QJsonObject &j_result) {
    _async_results.insert( uuid, j_result );
    _async_results_queue.enqueue( uuid );
    _asyncs.removeOne( uuid );
}


bool SwiftApiClient::sendAsyncResult(const quint64 &uuid) {
    if ( _async_results.contains( uuid ) ) {
        if ( publishWamp( WampTopics::ApiRpcAsyncResults,  _async_results.value( uuid ),  uuid ) ) {
            const AsyncMethods method = _async_dictionary.value( uuid );
            const QJsonObject res( _async_results.value( uuid ) );
            methodState( method, res.value("success").toBool(false) );
            emit apiParsedResponse( method, res );
            _async_params.remove( uuid );
            _async_dictionary.remove( uuid );
            _async_results.remove( uuid );
            _asyncs.removeOne( uuid );
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void SwiftApiClient::unrealizedMethod(const QJsonObject &j_params, const quint64 &async_uuid)
{
    Q_UNUSED(j_params)
    QJsonObject ret;
    ret["success"] = false;
    ret["async_uuid"] = QString::number( async_uuid );
    ret["error"] = "Method not implemented";
    ret["method"] = getMethodName( _async_dictionary.value( async_uuid ) );
    ret["exchange_name"] = getExchangeName();
    ret["exchange_id"] = QString::number( getExchangeId() );
    registerAsyncResult( async_uuid, ret );
}

QString SwiftApiClient::paramMapToStr(const QMap<QString, QString>& params) const {
    QStringList result;
    for(auto it = params.cbegin(); it != params.cend(); ++it)
        result.append(it.key() + "=" + it.value());
    return result.join("&");
}

bool SwiftApiClient::isParamExist(const QStringList& params, const QJsonObject& j_params, QString& error) const {
    QStringList absentParams;
    for(auto& p : params){
        if(!j_params.contains(p))
            absentParams.append(p);
    }
    if(absentParams.isEmpty()){
        error = "";
        return true;
    }
    else {
        error = "Absent parameters: <" + absentParams.join(",") + ">";
        return false;
    }
}

bool SwiftApiClient::isUintValid(quint32 id, QString& error, const QString& id_name) const {
    if(id > 0)
        return true;
    error = "Invalid parameter - " + id_name;
    return false;
}

bool SwiftApiClient::isDoublePlus(double val, QString& error, const QString& id_name) const {
    if(val > 0)
        return true;
    error = "Invalid parameter - " + id_name;
    return false;
}

bool SwiftApiClient::isSideValid(const QString &side, QString &error) const
{
    if(side == "sell" || side == "buy")
        return true;
    error = "Invalid parameter - type";
    return false;
}

bool SwiftApiClient::isUintInRange(quint32 val, quint32 r_min, quint32 r_max, bool inclusive) const
{
    if(inclusive)
        return (val >= r_min && val <= r_max) ? true : false;
    else
        return (val > r_min && val < r_max) ? true : false;
}


void SwiftApiClient::initWorker(Wamp::Session *sess)
{
    settings = SwiftBot::moduleSettings();
    onWampSession_(sess);
}

QString SwiftApiClient::getWorkerName() const
{
    return getExchangeName();
}


QStringList SwiftApiClient::listenParams()
{
    return QStringList({
       exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_KEY),
       exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_SECRET),
       exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_USER)
    });
}

void SwiftApiClient::onParamChanged(const QString &param, const QString &value)
{
    if ( param == exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_KEY) ) {
        api_key = value;
    } else if ( param == exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_SECRET) ) {
        api_secret = value;
    } else if ( param == exchangeParamsPath( getExchangeName().toLower() ,SETTINGS_NAME_API_USER) ) {
        api_user = value;
    }
}
