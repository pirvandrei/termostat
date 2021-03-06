/***************************************************
  Adafruit ESP8266 Motor Controller Module 
 
 Given variables 
 TEMP DESIRED 22 degree

 TEMP froum outside ?? 10 

 Adjust the motor based on the given values
 ****************************************************/

// Libraries
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//temperature
#define TEMP_DESIRED  24
#define TEMP_OUTSIDE  10
#define TEMP_ALIEVE   false

// WiFi parameters
#define WLAN_SSID       "EmbedNet2"
#define WLAN_PASS       "betonprint"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "pirvandrei"
#define AIO_KEY         "c96c6d2a75754e389b5351bbf63c94ab"


// Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[]      = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[]    = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[]    = AIO_USERNAME;
const char MQTT_PASSWORD[]    = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed called 'lamp' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char Motor_FEED[]   = AIO_USERNAME "/feeds/motor";
Adafruit_MQTT_Subscribe motor = Adafruit_MQTT_Subscribe(&mqtt, Motor_FEED);

const char LAMP_FEED[]  = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Subscribe temp = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED);

/*************************** Sketch Code ************************************/

int motorASpeed = 1023;

#define motor_a_speed 5
#define motor_a_dir 0

void setup() {

  // Set motor  pin to output
  pinMode(motor_a_speed, OUTPUT);  // speed
  pinMode(motor_a_dir, OUTPUT);

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
  mqtt.subscribe(&temp);
  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;

  // ping adafruit io a few times to make sure we remain connected
  if (! mqtt.ping(3)) {
    // reconnect to adafruit io
    if (! mqtt.connected())
      connect();
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the motor events
    if (subscription == &motor) {

      // convert mqtt ascii payload to int
      char *value = (char *)motor.lastread;
      Serial.print(F("Received command: "));
      Serial.println(value);

      // execute command from server
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
    
    // Listen to temp values 
    else if (subscription == &temp) {

      char *value = (char *)temp.lastread;
      int x = String(value).toInt();

      Serial.print(F("Received temp: "));
      Serial.println(x);

      // Received temp value less then 24, increase a bit the motor
      if (x < TEMP_DESIRED)
      {
        //turn on
        Serial.println(F("Turning ON: "));
        analogWrite(motor_a_speed, motorASpeed);
        digitalWrite(motor_a_dir, 1);
        
      }
      else if (x >= TEMP_OUTSIDE)
      {
        // Received out temp value less then 24, so keep the motor off
        //turn off 
        Serial.println(F("Turning OFF: "));
        analogWrite(motor_a_speed, motorASpeed);
        digitalWrite(motor_a_dir, 0); 
      }
      else if (x == TEMP_OUTSIDE)
      {
        // same temp, reduce a bit !!!!
        //turn off 
        Serial.println(F("Turning OFF: "));
        analogWrite(motor_a_speed, motorASpeed);
        digitalWrite(motor_a_dir, 0); 
      }
    }
  }
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
