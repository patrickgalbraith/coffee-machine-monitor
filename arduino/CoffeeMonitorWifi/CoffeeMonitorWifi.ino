#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

const int ANALOG_PIN       = A0;
const int LED_PIN          = 14;
const int BAUD_RATE        = 115200;
const char* WIFI_SSID      = "";
const char* WIFI_PASSWORD  = "";
const char* WEBSERVICE_URL = "http://SOMEIP/";

unsigned int shakeThreshold           = 10;
unsigned int shakeStartTimeHysteresis = 4000;
unsigned int shakeStopTimeHysteresis  = 6000;
unsigned long shakeStateChangeTime    = 0;
unsigned long shakeStartTime          = 0;

int lastPinVal = 0;

bool notifyFlag = false;

enum {
  NO_SHAKING_LONG, // Haven't been shaking for a long time
  NO_SHAKING,      // Hasn't been any shaking
  PRE_SHAKING,     // Started shaking, pre-hysteresis
  SHAKING,         // Currently shaking
  POST_SHAKING     // Stopped shaking, pre-hysteresis
} shakingState = NO_SHAKING;

enum sensorShakeReturn {
  SENSOR_SHAKING,
  SENSOR_NOT_SHAKING
};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(BAUD_RATE);
  connectToWifi();
  ledOff();
  Serial.println("Setup finished!");
  Serial.println("----------------");
}

void loop() {
  shakeLoop();
}

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

void shakeLoop(void) {
  sensorShakeReturn sensorState = checkShake();

  if (sensorState == SENSOR_SHAKING) {
    switch (shakingState) {
      case NO_SHAKING_LONG:
      case NO_SHAKING: // If we haven't been shaking
        shakingState = PRE_SHAKING; // Set mode to pre-shaking
        shakeStateChangeTime = millis(); // Log state change time

        Serial.println("Maybe brewing...");
        break;
      case PRE_SHAKING: // If we're pre-hysteresis shaking
        if (millis() - shakeStateChangeTime >= shakeStartTimeHysteresis) {
          // If we've passed hysteresis time
          shakingState = SHAKING; // Set mode to shaking
          ledOn();
          Serial.println("Brewing started");
          notifyFlag = true; // Flag that we need to notify when shaking stops
        }
        break;
      case SHAKING: // If we're already shaking
        break; // Do nothing
      case POST_SHAKING: // If we didn't stop shaking before hysteresis
        shakingState = SHAKING; // Go back to shaking
        break;
    }
  }
  else if (sensorState == SENSOR_NOT_SHAKING) {
    switch (shakingState) {
      case NO_SHAKING_LONG: // If we haven't been shaking for a long time
        break; // Do nothing
      case NO_SHAKING: // If we haven't been shaking
        if (millis() - shakeStateChangeTime >= (shakeStopTimeHysteresis)) {
          // Check if it's been a long time
          ledOff();

          shakingState = NO_SHAKING_LONG;

          if (notifyFlag == true) {
            notifyFlag = false;

            notifyWebservice();

            Serial.println("Brewing finished");
            Serial.println("----------------");
          }
        }
        break;
      case PRE_SHAKING: // If we're pre-hysteresis shaking
        if (millis() - shakeStateChangeTime >= 4000) {
          shakingState = NO_SHAKING; // Go back to no shaking
          Serial.println("Guess not");
        }
        break;
      case SHAKING: // If we're already shaking
        shakingState = POST_SHAKING; // Go to hysteresis cooldown
        shakeStateChangeTime = millis();
        break; // Do nothing
      case POST_SHAKING: // If we're in the shake cooldown state
        if (millis() - shakeStateChangeTime >= shakeStartTimeHysteresis) {
          shakingState = NO_SHAKING;
          Serial.println("Brewing stopping...");
        }
        break;
      }
  }

  delay(50);
}

sensorShakeReturn checkShake(void) {
  static unsigned long lastShakeCheck = 0;

  int curPinVal = analogRead(ANALOG_PIN);
  int pinDelta = abs(lastPinVal - curPinVal);

  lastPinVal = curPinVal;

  if (pinDelta >= shakeThreshold)
    return SENSOR_SHAKING;
  else
    return SENSOR_NOT_SHAKING;
}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    ledOff();
    delay(500);
    Serial.print(".");
    ledOn();
    delay(500);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void notifyWebservice() {
  HTTPClient http;

  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  Serial.print("[HTTP] Request begin...\n");

  http.begin(WEBSERVICE_URL);

  // Probably should really use POST since we are updating something
  // GET is easier to work with and it's an internal api #shruglife ¯\_(ツ)_/¯
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
