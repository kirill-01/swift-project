## Feeds:

- [swift.arbitrage.windows](#swiftarbitragewindows)


## Methods:

- [swift.arbitrage.volume](#swiftarbitragevolume)
- [swift.arbitrage.info](#swiftarbitrageinfo)


# API for module: arbitrage

Arbitrage trading module

# Feeds

## swift.arbitrage.windows

Information about profitable windows publishing into this feed

#### Message 

```json
[
    "Message object"
]

```

---

# Methods

## swift.arbitrage.volume

Request arbitrage volume info report (sending to telegram)

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "OK"
]

```

[Go to top](#methods-1)
---

## swift.arbitrage.info

Request arbitrage summary indormation

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    {
        "BTC-USDT": {
            "binance": {
                "buy": "14328.78000000",
                "buy_volume": "10.6875",
                "sell": "14328.79000000",
                "sell_volume": "103.066",
                "updated": "1604553668"
            },
            "bitfinex": {
                "buy": "14335.00000000",
                "buy_volume": "2.5138",
                "sell": "14335.78624678",
                "sell_volume": "4.65887",
                "updated": "1604553668"
            },
            "bittrex": {
                "buy": "14320.55810147",
                "buy_volume": "5.58654",
                "sell": "14329.42100000",
                "sell_volume": "7.85037",
                "updated": "1604553668"
            },
            "hitbtc": {
                "buy": "14329.21000000",
                "buy_volume": "2.01127",
                "sell": "14329.22000000",
                "sell_volume": "9.27742",
                "updated": "1604553668"
            },
            "huobi": {
                "buy": "14328.59000000",
                "buy_volume": "7.99638",
                "sell": "14328.60000000",
                "sell_volume": "5.35188",
                "updated": "1604553668"
            },
            "idcm": {
                "buy": "14319.99000000",
                "buy_volume": "10.9069",
                "sell": "14332.60000000",
                "sell_volume": "0.7697",
                "updated": "1604553668"
            },
            "kucoin": {
                "buy": "14326.30000000",
                "buy_volume": "8.98781",
                "sell": "14326.40000000",
                "sell_volume": "1.14448",
                "updated": "1604553668"
            },
            "zb": {
                "buy": "14317.59000000",
                "buy_volume": "1.9688",
                "sell": "14319.56000000",
                "sell_volume": "1.0846",
                "updated": "1604553668"
            }
        },
        "ETH-USDT": {
            "binance": {
                "buy": "404.41000000",
                "buy_volume": "525.13",
                "sell": "404.42000000",
                "sell_volume": "329.815",
                "updated": "1604553668"
            },
            "bitfinex": {
                "buy": "404.91000000",
                "buy_volume": "195.103",
                "sell": "404.92000000",
                "sell_volume": "156.605",
                "updated": "1604553668"
            },
            "bittrex": {
                "buy": "404.23800012",
                "buy_volume": "66.2",
                "sell": "404.73832941",
                "sell_volume": "117.414",
                "updated": "1604553668"
            },
            "hitbtc": {
                "buy": "404.46800000",
                "buy_volume": "95.2447",
                "sell": "404.50300000",
                "sell_volume": "90.7083",
                "updated": "1604553668"
            },
            "huobi": {
                "buy": "404.45000000",
                "buy_volume": "1257.97",
                "sell": "404.46000000",
                "sell_volume": "214.122",
                "updated": "1604553668"
            },
            "idcm": {
                "buy": "404.16000000",
                "buy_volume": "80.44",
                "sell": "404.89000000",
                "sell_volume": "144.544",
                "updated": "1604553668"
            },
            "kraken": {
                "buy": "404.56000000",
                "buy_volume": "162.306",
                "sell": "404.78000000",
                "sell_volume": "249.079",
                "updated": "1604553668"
            },
            "kucoin": {
                "buy": "404.44000000",
                "buy_volume": "235.124",
                "sell": "404.49000000",
                "sell_volume": "66.5186",
                "updated": "1604553668"
            },
            "zb": {
                "buy": "404.35000000",
                "buy_volume": "205.21",
                "sell": "404.45000000",
                "sell_volume": "59.601",
                "updated": "1604553668"
            }
        }
    }
]

```

[Go to top](#methods-1)
---



[Back to list](docs/api.md)

