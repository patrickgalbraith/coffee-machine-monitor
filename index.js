const fs = require('fs')
const express = require('express')
const app = express()

let coffeesDrunkTotal = 0;

function getDate() {
  let d     = new Date(),
      month = '' + (d.getMonth() + 1),
      day   = '' + d.getDate(),
      year  = d.getFullYear()

  if (month.length < 2) month = '0' + month
  if (day.length < 2) day = '0' + day

  return [year, month, day].join('-')
}

function recordCoffee(cb) {
  const path      = './data/' + getDate() + '.dat'
  const timestamp = +(Date.now())
  const text      = timestamp + "\n"

  fs.appendFile(path, text, function(err) {
    if (err) {
      console.error(err)
      return cb(err)
    }

    console.log('Saved coffee #' + coffeesDrunkTotal)

    return cb()
  })
}

app.get('/', function (req, res) {
  coffeesDrunkTotal++

  recordCoffee(function(err) {
    if (err) {
      return res.status(500).send('Sorry this coffee couldn\'t be recorded.')
    }

    res.send('The server says you drank coffee number #' + coffeesDrunkTotal)
  })
})

app.listen(3000, function () {
  console.log('Listening on port 3000!')
})