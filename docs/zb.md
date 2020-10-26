# API description for module zb
## Methods
### swift.api.synctime.zb
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

```### swift.api.currencies.zb
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

```### swift.api.markets.zb
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

```### swift.api.orderbooks.zb
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

```### swift.api.order.create.zb
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

```### swift.api.order.cancel.zb
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

```### swift.api.order.get.zb
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

```### swift.api.withdraw.history.zb
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

```### swift.api.withdraw.create.zb
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

```### swift.api.withdraw.inner.zb
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

```### swift.api.withdraw.fee.zb
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

```### swift.api.balances.zb
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

```### swift.api.depostist.history.zb
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

```### swift.api.deposits.address.zb
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

```### swift.api.trade.history.zb
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

```### swift.api.trade.active.zb
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

```### swift.api.trade.fee.zb
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