## Feeds:



## Methods:

- [swift.address.generate](#swiftaddressgenerate)
- [swift.address.balance](#swiftaddressbalance)
- [swift.address.withdraw](#swiftaddresswithdraw)


# API for module: blockcypher

BlockCypher API to create and manage crypto wallets

# Methods

## swift.address.generate

Create new address for specified coin

#### Arguments 

```json
[
    "COIN_ID"
]

```

#### Response 

```json
[
    {
        "address": "ADDREDDSTRING",
        "success": true
    }
]

```

[Go to top](#methods-1)
---

## swift.address.balance

Get information about address balance

#### Arguments 

```json
[
    "ADDRESS_ID"
]

```

#### Response 

```json
[
    "BALANCE"
]

```

[Go to top](#methods-1)
---

## swift.address.withdraw

Withdraw assets from address

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "JSONOBJECT"
]

```

[Go to top](#methods-1)
---



[Back to list](docs/api.md)

