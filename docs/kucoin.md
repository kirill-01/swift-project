## Feeds:



## Methods:

- [swift.api.synctime.kucoin](#swiftapisynctimekucoin)
- [swift.api.currencies.kucoin](#swiftapicurrencieskucoin)
- [swift.api.markets.kucoin](#swiftapimarketskucoin)
- [swift.api.orderbooks.kucoin](#swiftapiorderbookskucoin)
- [swift.api.order.create.kucoin](#swiftapiordercreatekucoin)
- [swift.api.order.cancel.kucoin](#swiftapiordercancelkucoin)
- [swift.api.order.get.kucoin](#swiftapiordergetkucoin)
- [swift.api.withdraw.history.kucoin](#swiftapiwithdrawhistorykucoin)
- [swift.api.withdraw.create.kucoin](#swiftapiwithdrawcreatekucoin)
- [swift.api.withdraw.inner.kucoin](#swiftapiwithdrawinnerkucoin)
- [swift.api.withdraw.fee.kucoin](#swiftapiwithdrawfeekucoin)
- [swift.api.balances.kucoin](#swiftapibalanceskucoin)
- [swift.api.depostist.history.kucoin](#swiftapidepostisthistorykucoin)
- [swift.api.deposits.address.kucoin](#swiftapidepositsaddresskucoin)
- [swift.api.trade.history.kucoin](#swiftapitradehistorykucoin)
- [swift.api.trade.active.kucoin](#swiftapitradeactivekucoin)
- [swift.api.trade.fee.kucoin](#swiftapitradefeekucoin)


# API for module: kucoin

KuCoin API client module

# Methods

## swift.api.synctime.kucoin

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

## swift.api.currencies.kucoin

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

## swift.api.markets.kucoin

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

## swift.api.orderbooks.kucoin

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

## swift.api.order.create.kucoin

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

## swift.api.order.cancel.kucoin

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

## swift.api.order.get.kucoin

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

## swift.api.withdraw.history.kucoin

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

## swift.api.withdraw.create.kucoin

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

## swift.api.withdraw.inner.kucoin

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

## swift.api.withdraw.fee.kucoin

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

## swift.api.balances.kucoin

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

## swift.api.depostist.history.kucoin

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

## swift.api.deposits.address.kucoin

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

## swift.api.trade.history.kucoin

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

## swift.api.trade.active.kucoin

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

## swift.api.trade.fee.kucoin

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

