## Feeds:



## Methods:

- [swift.api.synctime.zb](#swiftapisynctimezb)
- [swift.api.currencies.zb](#swiftapicurrencieszb)
- [swift.api.markets.zb](#swiftapimarketszb)
- [swift.api.orderbooks.zb](#swiftapiorderbookszb)
- [swift.api.order.create.zb](#swiftapiordercreatezb)
- [swift.api.order.cancel.zb](#swiftapiordercancelzb)
- [swift.api.order.get.zb](#swiftapiordergetzb)
- [swift.api.withdraw.history.zb](#swiftapiwithdrawhistoryzb)
- [swift.api.withdraw.create.zb](#swiftapiwithdrawcreatezb)
- [swift.api.withdraw.inner.zb](#swiftapiwithdrawinnerzb)
- [swift.api.withdraw.fee.zb](#swiftapiwithdrawfeezb)
- [swift.api.balances.zb](#swiftapibalanceszb)
- [swift.api.depostist.history.zb](#swiftapidepostisthistoryzb)
- [swift.api.deposits.address.zb](#swiftapidepositsaddresszb)
- [swift.api.trade.history.zb](#swiftapitradehistoryzb)
- [swift.api.trade.active.zb](#swiftapitradeactivezb)
- [swift.api.trade.fee.zb](#swiftapitradefeezb)


# API for module: zb

ZB API client module

# Methods

## swift.api.synctime.zb

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

## swift.api.currencies.zb

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

## swift.api.markets.zb

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

## swift.api.orderbooks.zb

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

## swift.api.order.create.zb

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

## swift.api.order.cancel.zb

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

## swift.api.order.get.zb

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

## swift.api.withdraw.history.zb

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

## swift.api.withdraw.create.zb

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

## swift.api.withdraw.inner.zb

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

## swift.api.withdraw.fee.zb

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

## swift.api.balances.zb

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

## swift.api.depostist.history.zb

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

## swift.api.deposits.address.zb

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

## swift.api.trade.history.zb

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

## swift.api.trade.active.zb

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

## swift.api.trade.fee.zb

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

