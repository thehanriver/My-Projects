var $ = require( "jquery" );
var express = require('express');
var app = express();
var path = require('path');
var http = require('http').Server(app);
var io = require('socket.io')(http);
var mongo = require('mongodb');

const readline = require('readline');
const Stream = require('stream');
const bodyParser = require('body-parser');
const router = express.Router();

const uri = "mongodb+srv://mario:1GBSt0rage%21@vivcluster.h5rba.mongodb.net/Election?retryWrites=true&w=majority";
const MongoClient = require('mongodb').MongoClient;
const client = new MongoClient(uri, {useUnifiedTopology: true, useNewUrlParser: true});
const fs = require('fs');

var clear_flag = 0;
var all;
var red;
var blue;
var green;

function candVotes(){
    client.connect(function(err, db) {
      if (err) throw err;
      var dbo = db.db("Election");
      var query = { vote: /^R/ };
      dbo.collection("Voters").find(query).toArray(function(err, result) {
        if (err) throw err;
        red = result;
        console.log(result);
        db.close();
      });
      var query = { vote: /^G/ };
      dbo.collection("Voters").find(query).toArray(function(err, result) {
        if (err) throw err;
        green = result;
        console.log(result);
        db.close();
      });
      var query = { vote: /^B/ };
      dbo.collection("Voters").find(query).toArray(function(err, result) {
        if (err) throw err;
        blue = result;
        console.log(result);
        db.close();
      });
    });
}

function readAll(){
  client.connect(function(err, db) {
      if (err) throw err;
      var dbo = db.db("Election");
      dbo.collection("Voters").find({}).toArray(function(err, result) {
        if (err) throw err;
        all = result;
        console.log(result);
        db.close();
      });
    });
}

function clearData(){
    client.connect(function(err, db) {
      if (err) throw err;
      var dbo = db.db("Election");
      dbo.collection("Voters").drop(function(err, delOK) {
        if (err) throw err;
        if (delOK)
        {
          all = [];
          red = [];
          blue = [];
          green = [];
          console.log("Collection deleted");
        }
        db.close();
      });
    });
}

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});

app.get('/all', function(req, res) {
  res.send(all);  // Send array of data back to requestor
});

app.get('/all/green', function(req, res) {
  res.send(green)
});

app.get('/all/blue', function(req, res) {
  res.send(blue;
});
app.get('/all/red', function(req, res) {
  res.send(red);
});

app.post('/clear', (req,res) => {
  clear_flag = req.body.clear;
  res.end('yes');
});

app.get('/clear', function(req,res) {
  res.send(clear_flag);
})

if(clear_flag == 1 ){
  clearData();
  console.log("cleared");
  clear_flag == 0;
}
app.listen(8080);

// Required module
var dgram = require('dgram');

// Port and IP
var PORT = 1234;
var HOST = '192.168.1.111';

// Create socket
var server = dgram.createSocket('udp4');

// Create server
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

// On connection, print out received message
server.on('message', function (message, remote) {
    var id;
    var vote;
    var buffer;
    var myObj;
    var today = new Date();
    var date = today.getFullYear()+'-'+(today.getMonth()+1)+'-'+today.getDate();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
    var dateTime = date+' '+time;
    buffer = message.toString();
    buffer = buffer.split(',');
    id = parseInt(buffer[0]);
    vote = buffer[1];
    myObj = {id: id, vote: vote, date: dateTime};

    client.connect(function(err, db) {
        if (err) throw err;
        var dbo = db.db("Election");
        var myobj = { fob_ID: "1", vote: "R", time: "10:28pm" };
        dbo.collection("Voters").insertOne(myobj, function(err, res) {
          if (err) throw err;
          console.log("1 document inserted");
          db.close();
        });
      });

    console.log(remote.address + ':' + remote.port +' - ' + message);

    server.send(led_status.toString(),remote.port,remote.address,function(error){
      if(error){
        console.log('MEH!');
      }
      else {
        console.log('Sent: ' + );
      }
    });

});

// Bind server to port and IP
server.bind(PORT, HOST);
