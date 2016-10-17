var net = require('net');

var HOST = IN.host;
var PORT = IN.port;

net.createServer(function(sock) {
    
    console.log('connected: ' + sock.remoteAddress + ':' + sock.remotePort);
    
    sock.on('data', function(data) {
       
       console.log('data: ' + data);
       
       sock.write('"' + data + '" received.');
       
       var sendData = data.toString();
       sendOUT({recv : sendData});
    });
    
    sock.on('close', function(data) {
        console.log('client closed.');
    });
    
}).listen(PORT, HOST);

