var express = require('express');
var app = express();
var path = require('path');
var fs = require('fs');
var csv = require("csv-parse");
// var getLastLine = require('./fileTools.js');

// viewed at http://localhost:8080
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});
// request data at http://localhost:8080/data or just "/data"
app.get('/data', function(req, res) {
  var data = [];  // Array to hold all csv data
  var last_row = "";
  fs.createReadStream('../data/sensors.csv')  // path to csv
  .pipe(csv())
  .on('data', (row) => {
    // add thing to check time on row to check if it is a new row. push to data only if it is new
    if (row === last_row){
      return;
    } else {
      // console.log(row);
      data.push(row);  // Add row of data to array
      last_row = row;
    }
  })
  .on('end', () => {
    console.log('CSV file successfully processed');
    res.send(data);  // Send array of data back to requestor
  });
});

// // request data at http://localhost:8080/data or just "/data"
// app.get('/data', function(req, res) {
//   var data = [];  // Array to hold all csv data
//   var last_row = "";
//   getLastLine(fileName, 1)
//     .then((lastLine)=> {
//         console.log(lastLine)
//     })
//     .catch((err)=> {
//         console.error(err)
//     })
//   fs.createReadStream('../data/sensors.csv')  // path to csv
//   .pipe(csv())
//   .on('data', (row) => {
//     // add thing to check time on row to check if it is a new row. push to data only if it is new
//     if (row === last_row){
//       return;
//     } else {
//       // console.log(row);
//       data.push(row);  // Add row of data to array
//       last_row = row;
//     }
//   })
//   .on('end', () => {
//     console.log('CSV file successfully processed');
//     res.send(data);  // Send array of data back to requestor
//   });
// });


app.listen(8080);

