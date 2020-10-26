# API description for module huobi
## Methods
### swift.api.synctime.huobi
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

```### swift.api.currencies.huobi
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

```### swift.api.markets.huobi
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

```### swift.api.orderbooks.huobi
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

```### swift.api.order.create.huobi
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

```### swift.api.order.cancel.huobi
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

```### swift.api.order.get.huobi
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

```### swift.api.withdraw.history.huobi
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

```### swift.api.withdraw.create.huobi
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

```### swift.api.withdraw.inner.huobi
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

```### swift.api.withdraw.fee.huobi
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

```### swift.api.balances.huobi
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

```### swift.api.depostist.history.huobi
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

```### swift.api.deposits.address.huobi
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

```### swift.api.trade.history.huobi
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

```### swift.api.trade.active.huobi
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

```### swift.api.trade.fee.huobi
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