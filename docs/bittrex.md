## Feeds:



## Methods:

- [swift.api.synctime.bittrex](#swiftapisynctimebittrex)
- [swift.api.currencies.bittrex](#swiftapicurrenciesbittrex)
- [swift.api.markets.bittrex](#swiftapimarketsbittrex)
- [swift.api.orderbooks.bittrex](#swiftapiorderbooksbittrex)
- [swift.api.order.create.bittrex](#swiftapiordercreatebittrex)
- [swift.api.order.cancel.bittrex](#swiftapiordercancelbittrex)
- [swift.api.order.get.bittrex](#swiftapiordergetbittrex)
- [swift.api.withdraw.history.bittrex](#swiftapiwithdrawhistorybittrex)
- [swift.api.withdraw.create.bittrex](#swiftapiwithdrawcreatebittrex)
- [swift.api.withdraw.inner.bittrex](#swiftapiwithdrawinnerbittrex)
- [swift.api.withdraw.fee.bittrex](#swiftapiwithdrawfeebittrex)
- [swift.api.balances.bittrex](#swiftapibalancesbittrex)
- [swift.api.depostist.history.bittrex](#swiftapidepostisthistorybittrex)
- [swift.api.deposits.address.bittrex](#swiftapidepositsaddressbittrex)
- [swift.api.trade.history.bittrex](#swiftapitradehistorybittrex)
- [swift.api.trade.active.bittrex](#swiftapitradeactivebittrex)
- [swift.api.trade.fee.bittrex](#swiftapitradefeebittrex)


# API for module: bittrex

Bittrex API client module

# Methods

## swift.api.synctime.bittrex

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

## swift.api.currencies.bittrex

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

## swift.api.markets.bittrex

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

## swift.api.orderbooks.bittrex

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

## swift.api.order.create.bittrex

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

## swift.api.order.cancel.bittrex

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

## swift.api.order.get.bittrex

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

## swift.api.withdraw.history.bittrex

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

## swift.api.withdraw.create.bittrex

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

## swift.api.withdraw.inner.bittrex

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

## swift.api.withdraw.fee.bittrex

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

## swift.api.balances.bittrex

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

## swift.api.depostist.history.bittrex

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

## swift.api.deposits.address.bittrex

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

## swift.api.trade.history.bittrex

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

## swift.api.trade.active.bittrex

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

## swift.api.trade.fee.bittrex

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

