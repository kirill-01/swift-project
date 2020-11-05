## Feeds:



## Methods:

- [swift.api.synctime.cexio](#swiftapisynctimecexio)
- [swift.api.currencies.cexio](#swiftapicurrenciescexio)
- [swift.api.markets.cexio](#swiftapimarketscexio)
- [swift.api.orderbooks.cexio](#swiftapiorderbookscexio)
- [swift.api.order.create.cexio](#swiftapiordercreatecexio)
- [swift.api.order.cancel.cexio](#swiftapiordercancelcexio)
- [swift.api.order.get.cexio](#swiftapiordergetcexio)
- [swift.api.withdraw.history.cexio](#swiftapiwithdrawhistorycexio)
- [swift.api.withdraw.create.cexio](#swiftapiwithdrawcreatecexio)
- [swift.api.withdraw.inner.cexio](#swiftapiwithdrawinnercexio)
- [swift.api.withdraw.fee.cexio](#swiftapiwithdrawfeecexio)
- [swift.api.balances.cexio](#swiftapibalancescexio)
- [swift.api.depostist.history.cexio](#swiftapidepostisthistorycexio)
- [swift.api.deposits.address.cexio](#swiftapidepositsaddresscexio)
- [swift.api.trade.history.cexio](#swiftapitradehistorycexio)
- [swift.api.trade.active.cexio](#swiftapitradeactivecexio)
- [swift.api.trade.fee.cexio](#swiftapitradefeecexio)


# API for module: cexio

Cexio API client module

# Methods

## swift.api.synctime.cexio

If supported by exchange, receiving timestamp from remote server and correct local noncer

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.currencies.cexio

Get list of available currencies from exchange API

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.markets.cexio

Get list of available markets  from exchange API

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.orderbooks.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.order.create.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.order.cancel.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.order.get.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.history.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.create.cexio

Create new withdraw

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.inner.cexio

Inner move assets between accounts

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.withdraw.fee.cexio

Get withdraw fees

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.balances.cexio

Get balances info

#### Arguments 

```json
[
    "JSONPARAMS"
]

```

#### Response 

```json
[
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.depostist.history.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.deposits.address.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.trade.history.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.trade.active.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---

## swift.api.trade.fee.cexio

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
    "ASYNC_UUID"
]

```

[Go to top](#methods-1)
---



[Back to list](docs/api.md)

