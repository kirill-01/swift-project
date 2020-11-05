## Feeds:



## Methods:

- [swift.api.synctime.hitbtc](#swiftapisynctimehitbtc)
- [swift.api.currencies.hitbtc](#swiftapicurrencieshitbtc)
- [swift.api.markets.hitbtc](#swiftapimarketshitbtc)
- [swift.api.orderbooks.hitbtc](#swiftapiorderbookshitbtc)
- [swift.api.order.create.hitbtc](#swiftapiordercreatehitbtc)
- [swift.api.order.cancel.hitbtc](#swiftapiordercancelhitbtc)
- [swift.api.order.get.hitbtc](#swiftapiordergethitbtc)
- [swift.api.withdraw.history.hitbtc](#swiftapiwithdrawhistoryhitbtc)
- [swift.api.withdraw.create.hitbtc](#swiftapiwithdrawcreatehitbtc)
- [swift.api.withdraw.inner.hitbtc](#swiftapiwithdrawinnerhitbtc)
- [swift.api.withdraw.fee.hitbtc](#swiftapiwithdrawfeehitbtc)
- [swift.api.balances.hitbtc](#swiftapibalanceshitbtc)
- [swift.api.depostist.history.hitbtc](#swiftapidepostisthistoryhitbtc)
- [swift.api.deposits.address.hitbtc](#swiftapidepositsaddresshitbtc)
- [swift.api.trade.history.hitbtc](#swiftapitradehistoryhitbtc)
- [swift.api.trade.active.hitbtc](#swiftapitradeactivehitbtc)
- [swift.api.trade.fee.hitbtc](#swiftapitradefeehitbtc)


# API for module: hitbtc

HitBTC API client module

# Methods

## swift.api.synctime.hitbtc

If supported by exchange, receiving timestamp from remote server and correct local noncer

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.currencies.hitbtc

Get list of available currencies from exchange API

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "currencies": [
        {
            "id": "0",
            "name": "BTC"
        }
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.markets.hitbtc

Get list of available markets  from exchange API

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "markets": [
        {
            "base_currency_id": "1",
            "exchange_id": "1",
            "id": "0",
            "name": "BTC-USDT",
            "quote_currency_id": "2"
        }
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.orderbooks.hitbtc

Get orderbooks from exchange. Caution: only one market per request!

#### Arguments 

```json
[
    {
        "market_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "asks": [
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ]
    ],
    "bids": [
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ],
        [
            "14000.00",
            "0.001"
        ]
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.order.create.hitbtc

Create new limit order

#### Arguments 

```json
[
    {
        "amount": "1.00000000",
        "local_id": "localuuid",
        "market_id": "1",
        "rate": "12500.00",
        "type": "0"
    }
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "amount": "0.001",
    "amount_left": "0.001",
    "ext_id": "someExtId",
    "local_id": "localId",
    "rate": "14002.01",
    "status": "1",
    "ts": "14030234234234"
}

```

[Go to top](#methods-1)
---

## swift.api.order.cancel.hitbtc

Cancel order

#### Arguments 

```json
[
    {
        "remote_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "amount": "0.001",
    "amount_left": "0.001",
    "ext_id": "someExtId",
    "local_id": "localId",
    "rate": "14002.01",
    "status": "1",
    "ts": "14030234234234"
}

```

[Go to top](#methods-1)
---

## swift.api.order.get.hitbtc

Get order info

#### Arguments 

```json
[
    {
        "remote_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "amount": "0.001",
    "amount_left": "0.001",
    "ext_id": "someExtId",
    "local_id": "localId",
    "rate": "14002.01",
    "status": "1",
    "ts": "14030234234234"
}

```

[Go to top](#methods-1)
---

## swift.api.withdraw.history.hitbtc

Get withdrawals history

#### Arguments 

```json
[
    {
        "currency_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.create.hitbtc

Create new withdraw

#### Arguments 

```json
[
    {
        "address": "someAddressInBlockchain",
        "amount": "1.05",
        "currency_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.inner.hitbtc

Inner move assets between accounts

#### Arguments 

```json
[
    {
        "amount": "1.05",
        "currency_id": "1",
        "from": "trade",
        "to": "main"
    }
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.fee.hitbtc

Get withdraw fees

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.balances.hitbtc

Get balances info

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "balances": [
        {
            "available": "0.100",
            "currency_id": "1",
            "orders": "0.100",
            "total": "0.200"
        }
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.depostist.history.hitbtc

Get deposits history

#### Arguments 

```json
[
    {
        "currency_id": "1"
    }
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.deposits.address.hitbtc

Get deposit address

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---

## swift.api.trade.history.hitbtc

Get closed orders history

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "orders": [
        {
            "amount": "0.001",
            "amount_left": "0.001",
            "ext_id": "someExtId",
            "local_id": "localId",
            "rate": "14002.01",
            "status": "2",
            "ts": "14030234234234"
        }
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.trade.active.hitbtc

Get active orders

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    100000001
]

```

#### Async response 

```json
{
    "orders": [
        {
            "amount": "0.001",
            "amount_left": "0.001",
            "ext_id": "someExtId",
            "local_id": "localId",
            "rate": "14002.01",
            "status": "1",
            "ts": "14030234234234"
        }
    ]
}

```

[Go to top](#methods-1)
---

## swift.api.trade.fee.hitbtc

Get trade fees

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    100000001
]

```

[Go to top](#methods-1)
---



[Back to list](docs/api.md)

