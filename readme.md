# :coffee: Monitor

Project to monitor coffee intake at The Digital Embassy.

[![N|Solid](https://i.imgur.com/L2Q3DN4.jpg)](https://i.imgur.com/L2Q3DN4.jpg)

## Information
 - NodeJS webservice using express.
   - run using `node index.js`
   - data is stored in the `data/` directory, a new `.dat` file will be written for each day containing timestamps of coffees drunk.
 - ESP8266 Arduino microcontroller and GY-61 accelerometer code.
   - see `arduino/` directory for details.
   - make sure to change the WiFi and webserver details before uploading.
   - connect serial monitor to `115200` to see log output.