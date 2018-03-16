// Libraries
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <Hash.h>


// Lamp pin
const int lamp_pin = 5;

// WiFi parameters
#define WLAN_SSID       "EmbedNet2"
#define WLAN_PASS       "betonprint"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "pirvandrei"
#define AIO_KEY         "c96c6d2a75754e389b5351bbf63c94ab"

#include "FastLED.h"
//Lights
#define DATA_PIN D2
#define NUM_LEDS 23
// Define the array of leds
CRGB leds[NUM_LEDS];
//String inString = "";

// Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[]     = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[]   = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[]   = AIO_USERNAME;
const char MQTT_PASSWORD[]   = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed called 'lamp' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char LAMP_FEED[]  = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Subscribe temp = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED);

/*************************** Sketch Code ************************************/

void setup() {
  //do light
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);


  // Set temp pin to output
  pinMode(lamp_pin, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Adafruit IO Example"));




  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // listen for events on the temp feed
  mqtt.subscribe(&temp);

  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;
  if (! mqtt.ping(3)) {
    // reconnect to adafruit io
    if (! mqtt.connected())
      connect();
  }

  while (subscription = mqtt.readSubscription(1000)) {
    if (subscription == &temp) {

      char *value = (char *)temp.lastread;
      int x = String(value).toInt();

      Serial.print(F("Received: "));
      Serial.println(x);

      for (int i = 0; i <= NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
        FastLED.show();
      }
      if (x >= 24)
      {
        leds[x / 2] = CRGB::Red;
        FastLED.show();
        delay(30);
      }
      else if (x < 24)
      {

        leds[x / 2] = CRGB::Blue;
        FastLED.show();
        delay(30);
      }
    }


  }

}

void red() {
  for (int i = 0; i < NUM_LEDS; i = i++)
  {
    leds[i] = CRGB::Red;

  }
  FastLED.show();
  delay(30);
}
void blue() {
  for (int i = 0; i < NUM_LEDS; i = i++)
  {
    leds[i] = CRGB::Blue;

  }
  FastLED.show();
  delay(30);
}


// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if (ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}

