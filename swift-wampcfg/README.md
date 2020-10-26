# SwiftBot module template

Clone base SwiftBot repository and install depended packages before start

```shell script
sudo apt update && sudo apt install mysql-server 
``` 


```shell script
git clone apppp.git
``` 

## QUICK USAGE

### Change module_env file

```bash
export MODULE_NAME="NAME OF YOUR MODULE"
export MODULE_DESCRIPTION="DESCRIPTION"
export OPTIONS_SQL=TRUE
export OPTIONS_WAMP=TRUE
export OPTIONS_SQL=TRUE
export OPTIONS_INITIAL_SQL=TRUE
export OPTIONS_LOGGER=TRUE
export OPTIONS_SINGLETON=TRUE
export OPTIONS_AUTHOR="Kirill Kuznetsov"
export OPTIONS_SINGLE_INSTANCE=TRUE
export OPTIONS_ASYNC_WORKER=TRUE
export OPTIONS_LISTEN_ORDERBOOKS=TRUE
export OPTIONS_LISTEN_BALANCE=TRUE
export OPTIONS_LISTEN_ORDERS=TRUE
```

### Run shell command

```shell script
./initmodule.sh
```

### Almost done!

What we got here ?

- Working module, ready to be build and install
- Multithreading template

You just need to add you functionality

### Playground

#### How to listen orderbooks changes

#### How to call internal system command


#### Get all balances info

**CLI command**
```sh
$ > swift-console
Connected. Enter the command:
cmd: > swift.rates
Response: 

{
   "1":"416.53000000",
   "108":"13064.98486197",
   "109":"415.87393876",
   "11":"415.33000000",
   "113":"416.56000000",
   "114":"13080.67000000",
   "12":"13064.50000000",
   "120":"13081.17000000",
   "121":"416.60000000",
   "18":"416.33900000",
   "19":"13075.79000000",
   "2":"13080.40000000",
   "23":"12823.43884952",
   "24":"360.00000000",
   "451":"13096.32438700",
   "453":"417.01690009",
   "7":"13078.36000000",
   "8":"416.35000000"
}
```

**QT C++**
```cpp
// Instantiate WAMP client
WampClient wamp_client;
...
QVariant response = wamp_client->getSession()->call("swift.orderbooks");

// response contain JsonObject packed to QString

QJsonObject j_orderbooks( QJsonDocument::fromJson( response.toString().toUtf8() ).object() );

// Now asks and bids available 
```


**JavaScript**

Using AutobahnJS client
```js
try {
   // for Node.js
   var autobahn = require('autobahn');
} catch (e) {
   // for browsers (where AutobahnJS is available globally)
}

var connection = new autobahn.Connection({url: 'ws://127.0.0.1:9000/', realm: 'realm1'});

connection.onopen = function (session) {

   // 1) subscribe to a topic
   function onevent(args) {
      console.log("Event:", args[0]);
   }
   session.subscribe('swift.system.events', onevent);

   // 2) publish data to feed
   session.publish('swift.custom.events', ['Hello, world!']);

   // 3) register a procedure for remoting
   function add2(args) {
      return args[0] + args[1];
   }
   session.register('swift.rpc.custom.add2', add2);

   // 4) call a remote procedure
   session.call('swift.rpc.custom.add2', [2, 3]).then(
      function (res) {
         console.log("Result:", res);
      }
   );
};

connection.open();
```

