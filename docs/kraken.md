# API description for module kraken
## Methods
### swift.api.synctime.kraken
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

```### swift.api.currencies.kraken
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

```### swift.api.markets.kraken
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

```### swift.api.orderbooks.kraken
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

```### swift.api.order.create.kraken
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

```### swift.api.order.cancel.kraken
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

```### swift.api.order.get.kraken
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

```### swift.api.withdraw.history.kraken
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

```### swift.api.withdraw.create.kraken
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

```### swift.api.withdraw.inner.kraken
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

```### swift.api.withdraw.fee.kraken
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

```### swift.api.balances.kraken
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

```### swift.api.depostist.history.kraken
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

```### swift.api.deposits.address.kraken
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

```### swift.api.trade.history.kraken
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

```### swift.api.trade.active.kraken
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

```### swift.api.trade.fee.kraken
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