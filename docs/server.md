## Feeds:

- [swift.system.feed.logs](#swiftsystemfeedlogs)
- [swift.system.feed.logs](#swiftsystemfeedlogs)


## Methods:

- [swift.module.cmd](#swiftmodulecmd)
- [swift.host.status](#swifthoststatus)
- [swift.server.cmd](#swiftservercmd)
- [swift.logger.logs](#swiftloggerlogs)
- [swift.logger.errors](#swiftloggererrors)
- [swift.methodstate](#swiftmethodstate)
- [swift.module.list](#swiftmodulelist)
- [swift.module.exchanges](#swiftmoduleexchanges)
- [swift.apimodule.report](#swiftapimodulereport)
- [swift.methodstate.summary](#swiftmethodstatesummary)
- [swift.assets.get](#swiftassetsget)


# API for module: server

Main server process

# Feeds

## swift.system.feed.logs

System logs feed (Only messages from logger)

#### Message 

```json
[
    "Message object"
]

```

---

## swift.system.feed.logs

System errors feed (Only messages from logger)

#### Message 

```json
[
    "Message object"
]

```

---

# Methods

## swift.module.cmd

Run specific command to controll module process. Allowed commands: [START,STOP,RESTART]

#### Arguments 

```json
[
    "MODULENAME",
    "COMMAND"
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

## swift.host.status

Get current RAM and CPU usage info

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    50.5,
    75.12
]

```

[Go to top](#methods-1)
---

## swift.server.cmd

Run specific shell command on host machine

#### Arguments 

```json
[
    "COMMAND"
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

## swift.logger.logs

Insert log message into Logger component

#### Arguments 

```json
[
    "SENDER_NAME",
    "GROUP",
    "MESSAGE"
]

```

#### Response 

```json
[
    1
]

```

[Go to top](#methods-1)
---

## swift.logger.errors

Insert error into Logger component

#### Arguments 

```json
[
    "SENDER_NAME",
    "GROUP",
    "MESSAGE"
]

```

#### Response 

```json
[
    1
]

```

[Go to top](#methods-1)
---

## swift.methodstate

Registering api method call result

#### Arguments 

```json
[
    "METHOD_URL",
    "TRUE"
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

## swift.module.list

Get all available modules list with status and info

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    {
        "binary": "/PATH/TO/BINARY",
        "is_enabled": true,
        "is_exchange": false,
        "name": "module_name",
        "status": "status",
        "version": "version"
    },
    {
        "binary": "/PATH/TO/BINARY",
        "is_enabled": true,
        "is_exchange": false,
        "name": "module_name",
        "status": "status",
        "version": "version"
    }
]

```

[Go to top](#methods-1)
---

## swift.module.exchanges

Run specific shell command on host machine

#### Arguments 

```json
[
]

```

#### Response 

```json
[
    "exchange_name_1,exchange_name_2,exchange_name_3"
]

```

[Go to top](#methods-1)
---

## swift.apimodule.report

Get api methods usage report

#### Arguments 

```json
[
    "MODULENAME",
    "MODULESTATUS"
]

```

#### Response 

```json
[
    1
]

```

[Go to top](#methods-1)
---

## swift.methodstate.summary

Run specific command to controll module process. Allowed commands: [START,STOP,RESTART]

#### Arguments 

```json
[
    "MODULENAME",
    "COMMAND"
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

## swift.assets.get

Run specific command to controll module process. Allowed commands: [START,STOP,RESTART]

#### Arguments 

```json
[
    "MODULENAME",
    "COMMAND"
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



[Back to list](docs/api.md)

