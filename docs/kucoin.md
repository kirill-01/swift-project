# API description for module kucoin
## Methods
### swift.api.synctime.kucoin
If supported by exchange, receiving timestamp from remote server and correct local noncer
#### Arguments 
```json
[
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.currencies.kucoin
Get list of available currencies from exchange API
#### Arguments 
```json
[
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.markets.kucoin
Get list of available markets  from exchange API
#### Arguments 
```json
[
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.orderbooks.kucoin
Get orderbooks from exchange. Caution: only one market per request!
#### Arguments 
```json
[
    {
        "market_id": "1"
    }
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.order.create.kucoin
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

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.order.cancel.kucoin
Cancel order
#### Arguments 
```json
[
    {
        "remote_id": "1"
    }
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.order.get.kucoin
Get order info
#### Arguments 
```json
[
    {
        "remote_id": "1"
    }
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.withdraw.history.kucoin
Get withdrawals history
#### Arguments 
```json
[
    {
        "currency_id": "1"
    }
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.withdraw.create.kucoin
Create new withdraw
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.withdraw.inner.kucoin
Inner move assets between accounts
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.withdraw.fee.kucoin
Get withdraw fees
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.balances.kucoin
Get balances info
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.depostist.history.kucoin
Get deposits history
#### Arguments 
```json
[
    {
        "currency_id": "1"
    }
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.deposits.address.kucoin
Get deposit address
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.trade.history.kucoin
Get closed orders history
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.trade.active.kucoin
Get active orders
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```### swift.api.trade.fee.kucoin
Get trade fees
#### Arguments 
```json
[
    "JSONPARAMS"
]

```#### Response 
```json
[
    "ASYNC_UUID"
]

```