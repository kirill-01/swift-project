## Feeds:



## Methods:

- [swift.api.synctime.bitfinex](#swiftapisynctimebitfinex)
- [swift.api.currencies.bitfinex](#swiftapicurrenciesbitfinex)
- [swift.api.markets.bitfinex](#swiftapimarketsbitfinex)
- [swift.api.orderbooks.bitfinex](#swiftapiorderbooksbitfinex)
- [swift.api.order.create.bitfinex](#swiftapiordercreatebitfinex)
- [swift.api.order.cancel.bitfinex](#swiftapiordercancelbitfinex)
- [swift.api.order.get.bitfinex](#swiftapiordergetbitfinex)
- [swift.api.withdraw.history.bitfinex](#swiftapiwithdrawhistorybitfinex)
- [swift.api.withdraw.create.bitfinex](#swiftapiwithdrawcreatebitfinex)
- [swift.api.withdraw.inner.bitfinex](#swiftapiwithdrawinnerbitfinex)
- [swift.api.withdraw.fee.bitfinex](#swiftapiwithdrawfeebitfinex)
- [swift.api.balances.bitfinex](#swiftapibalancesbitfinex)
- [swift.api.depostist.history.bitfinex](#swiftapidepostisthistorybitfinex)
- [swift.api.deposits.address.bitfinex](#swiftapidepositsaddressbitfinex)
- [swift.api.trade.history.bitfinex](#swiftapitradehistorybitfinex)
- [swift.api.trade.active.bitfinex](#swiftapitradeactivebitfinex)
- [swift.api.trade.fee.bitfinex](#swiftapitradefeebitfinex)


# API for module: bitfinex

bitfinex API client module

# Methods

## swift.api.synctime.bitfinex

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

## swift.api.currencies.bitfinex

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

## swift.api.markets.bitfinex

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

## swift.api.orderbooks.bitfinex

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

## swift.api.order.create.bitfinex

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

## swift.api.order.cancel.bitfinex

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

## swift.api.order.get.bitfinex

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

## swift.api.withdraw.history.bitfinex

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

## swift.api.withdraw.create.bitfinex

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

## swift.api.withdraw.inner.bitfinex

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

## swift.api.withdraw.fee.bitfinex

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

## swift.api.balances.bitfinex

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

## swift.api.depostist.history.bitfinex

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

## swift.api.deposits.address.bitfinex

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

## swift.api.trade.history.bitfinex

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

## swift.api.trade.active.bitfinex

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

## swift.api.trade.fee.bitfinex

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

