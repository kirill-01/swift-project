# Swift Bot Examples

### JavaScript


#### Get orderbooks

[Autobahn-js](https://github.com/crossbario/autobahn-js)

```js

try {
   // for Node.js
   var autobahn = require('autobahn');
} catch (e) {
   // for browsers (where AutobahnJS is available globally)
}

var connection = new autobahn.Connection({url: 'ws://127.0.0.1:9000/', realm: 'realm1'});
var async_waiters = [];
connection.onopen = function (session) {

   // 1) subscribe to a topic with async results of API methods
   function onAsyncResult(args) {
      var res_obj = JSON.parse( args[0] );
      var async_uuid = parseInt( res_obj.async_uuid );
      if ( async_waiters.includes( async_uuid ) ) {
          // Here is exactly required response
          console.log("Result:", args[0]);
      }
   }
   session.subscribe('swift.api.asyncresults', onAsyncResult);

  
   // 2) call a remote procedure on exchange client
   session.call('swift.api.bittrex.orderbooks', [JSON.stringify( {"market_id":"1"} )]).then(
      function (res) {
         // Here we receive async_uuid of request. Keep it .
         async_waiters.push( parseInt(res) );
      }
   );
};

connection.open();


```

#### Place new limit order

```js

try {
   // for Node.js
   var autobahn = require('autobahn');
} catch (e) {
   // for browsers (where AutobahnJS is available globally)
}

var connection = new autobahn.Connection({url: 'ws://127.0.0.1:9000/', realm: 'realm1'});
var async_waiters = [];
connection.onopen = function (session) {

   // 1) subscribe to a topic with async results of API methods
   function onAsyncResult(args) {
      var res_obj = JSON.parse( args[0] );
      var async_uuid = parseInt( res_obj.async_uuid );
      if ( async_waiters.includes( async_uuid ) ) {
          // Here is exactly required response
          console.log("Result:", args[0]);
      }
   }
   session.subscribe('swift.api.asyncresults', onAsyncResult);

  
   // 2) call a remote procedure on exchange client
   session.call('swift.api.bittrex.order.place', [JSON.stringify( {"market_id":"1","type":"sell","amount":"123.00000000","rate":"12500.125","local_id":"somelocalhash"} )]).then(
      function (res) {
         // Here we receive async_uuid of request. Keep it .
         async_waiters.push( parseInt(res) );
      }
   );
};

connection.open();


```

### PHP

#### Get orderbooks

```php

```

#### Place new limit order

```php

```

### C++

#### Get orderbooks

```cpp

```

#### Place new limit order

```cpp

```
