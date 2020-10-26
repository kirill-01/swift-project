# API description for module livecoin
## Methods
### swift.api.synctime.livecoin
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

```### swift.api.currencies.livecoin
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

```### swift.api.markets.livecoin
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

```### swift.api.orderbooks.livecoin
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

```### swift.api.order.create.livecoin
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

```### swift.api.order.cancel.livecoin
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

```### swift.api.order.get.livecoin
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

```### swift.api.withdraw.history.livecoin
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

```### swift.api.withdraw.create.livecoin
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

```### swift.api.withdraw.inner.livecoin
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

```### swift.api.withdraw.fee.livecoin
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

```### swift.api.balances.livecoin
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

```### swift.api.depostist.history.livecoin
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

```### swift.api.deposits.address.livecoin
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

```### swift.api.trade.history.livecoin
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

```### swift.api.trade.active.livecoin
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

```### swift.api.trade.fee.livecoin
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