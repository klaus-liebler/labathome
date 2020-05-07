const compression = require('compression');
const express = require('express');
var app = express();
app.use(compression());
app.use(express.static('C:/repos/wasmtest001/bin/Release/netstandard2.1/wwwroot'));
app.use(express.static('C:/repos/wasmtest001/wwwroot'));
//app.use(express.static('C:/repos/whsplc01/wasm/WebApplication2/Client/bin/Release/netstandard2.1/publish/wwwroot'));
app.listen(3000, function () {
    console.log('Example app listening on port 3000!');
  });