

// Libraries
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

// DHT 11 sensor
#define DHTPIN D7
#define DHTTYPE DHT11 

// WiFi parameters
#define WLAN_SSID       "Andrei" 
#define WLAN_PASS       "11111111"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "pirvandrei"
#define AIO_KEY         "c96c6d2a75754e389b5351bbf63c94ab"

// DHT sensor
DHT dht(DHTPIN, DHTTYPE, 15);

// Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[]  = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] = AIO_USERNAME;
const char MQTT_PASSWORD[] = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);/****************************** Feeds ***************************************/

// Setup feeds for temperature & humidity
const char TEMPERATURE_FEED[]  = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[]  = AIO_USERNAME "/feeds/humidity";
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);
/****************************** Feeds ***************************************/

// Setup a feed called 'lamp' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char Motor_FEED[]   = AIO_USERNAME "/feeds/motor";
Adafruit_MQTT_Subscribe motor = Adafruit_MQTT_Subscribe(&mqtt, Motor_FEED);

/*************************** Sketch Code ************************************/
int motorASpeed = 1023;
 
#define motor_a_speed 5
#define motor_a_dir 0

void setup() {

 // Set motor  pin to output 
  pinMode(motor_a_speed, OUTPUT);  // speed
  pinMode(motor_a_dir, OUTPUT);
  // Init sensor
  dht.begin();

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

 // listen for events on the motor feed
  mqtt.subscribe(&motor);
  
  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;
  
  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

 // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the motor events
    if (subscription == &motor) {

      // convert mqtt ascii payload to int
      char *value = (char *)motor.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      // Apply message to lamp
      String message = String(value);
      message.trim();
      if (message == "ON") {
        analogWrite(motor_a_speed, motorASpeed);
        digitalWrite(motor_a_dir, 0);
        }
      if (message == "OFF") {
        analogWrite(motor_a_speed, motorASpeed);
        digitalWrite(motor_a_dir, 1);
        } 
    } 
  } 


  // Grab the current state of the sensor
  int humidity_data = (int)dht.readHumidity();
  int temperature_data = (int)dht.readTemperature();

  // Publish data
  if (! temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));

  if (! humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  // Repeat every 10 seconds
  delay(5000);

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

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
