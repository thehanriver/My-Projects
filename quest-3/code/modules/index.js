var $ = require( "jquery" );
var express = require('express');
var app = express();
var path = require('path');
var fs = require('fs');
const readline = require('readline');
const Stream = require('stream');
const bodyParser = require('body-parser');

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());


var lastMessage = "";
var lastNMessages = [];
var N = 25;
var led_status = 0;
var latestTime = -1;

// viewed at http://localhost:8080
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});



app.post('/status', (req,res) => {
  led_status = req.body.led_status;
  res.end('yes');
});

app.get('/status', function(req,res) {
  res.send(led_status);
});


// request data at http://localhost:8080/data or just "/data"
app.get('/data', function(req, res) {
  res.send(lastNMessages);
});

// // request data at http://localhost:8080/data or just "/data"
// app.get('/data/last', function(req, res) {
//   data_last = []
//   last_lastLine = '';
//   const filename = '../data/sensors.csv';
//   getLastLine(filename, 1)
//     .then((lastLine)=> {
//       if (lastLine === last_lastLine){
//         return;
//       } else {
//         last_lastLine = lastLine;
//         data_last.push(lastLine.split(','))
//         res.send(data_last);
//       }
//     })
//     .catch((err)=> {
//         console.error(err);
//         res.send(err);
//     });
// });

// request data at http://localhost:8080/data or just "/data"
app.get('/data/last', function(req, res) {
  res.send(lastMessage.split(','))
});


app.listen(4000);



// Required module
var dgram = require('dgram');

// Port and IP
var PORT = 1234;
var HOST = '192.168.1.111';

// Create socket
var server = dgram.createSocket('udp4');

// led_status = 0;

// function toggleLED() {
//     if (!led_status)
//     {
//       led_status=1;
//     }
//     else
//     {
//       led_status=0
//     }
// }


// Create server
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

// On connection, print out received message
server.on('message', function (message, remote) {
    var temp = message.toString().split(',')
    var time = temp[0]
    // if (time >= 0)
    // {
    //   lastNMessages.push(message.toString());
    //   lastMessage = message.toString();
    // }
    if (time > latestTime)
    {
      latestTime = time;
      lastMessage = message.toString();
      lastNMessages.push(lastMessage);
    }
    else
    {
      lastMessage = message.toString();
      lastNMessages = [];
      lastNMessages.push(lastMessage)
      latestTime = time;
    }
    if (lastNMessages.length>N)
    {
      lastNMessages.shift();
    }
    console.log(remote.address + ':' + remote.port +' - ' + message);
    //num = (num+1)%2;
    // Send Ok acknowledgement
    server.send(led_status.toString(),remote.port,remote.address,function(error){
      if(error){
        console.log('Error: failed to acknowledge');
      }
    });

});

// Bind server to port and IP
server.bind(PORT, HOST);