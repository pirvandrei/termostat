#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <Hash.h>

WebSocketsServer webSocket = WebSocketsServer(81);

byte scale = 150; // 0-255

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN D2

int sliderValues[5] ;
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"
#define allocatedLightPatterns 10
typedef void (*lightPatternFunction) ();
/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

int lightPatternsAssigned = 0;
int lightPatternSelected = 6;
lightPatternFunction lightPatternFunctions[allocatedLightPatterns];
String lightPatternNames[allocatedLightPatterns];

// How many leds in your strip?
#define NUM_LEDS 28

#define ULTRASONIC 0

String name = "ioGlow-tb888";

// Define the array of leds
CRGB leds[NUM_LEDS];

unsigned long timer = 0;
unsigned int frameRate = 40;

void setup() {
  
  setupLed();

  delay(1000);
  Serial.begin(115200);
  Serial.println();

  //Serial.setDebugOutput(true);
  setupWifi();
  addLightPattern(off, "Off");
  addLightPattern(red, "Red");
  addLightPattern(rgb, "RGB");
  addLightPattern(hsv, "HSV");
  addLightPattern(chase, "Chase");
  addLightPattern(cylon, "cylon");
  addLightPattern(freePattern, "Andrei");
  addLightPattern(disco, "Disco");
  addLightPattern(leftSide, "left");
  addLightPattern(rightSide, "right");

  if (ULTRASONIC)
  {
    addLightPattern(interactive, "Interactive");
    usSetup();
  }


}
void freePattern()
{ 

  // 
  leds[3] = CRGB(0, 255, 0);
  leds[3] = CRGB(255, 0, 0);
  leds[5] = CRGB(255, 0, 0); 
  turndown();
  delay(10);

  leds[3] = CRGB(0, 255, 0);
  leds[3] = CRGB(255, 0, 0);
  leds[6] = CRGB(255, 0, 0); 
  leds[5] = CRGB(255, 0, 0); 
  turndown();
  delay(10);
  
  leds[3] = CRGB(0, 255, 0);
  leds[3] = CRGB(255, 0, 0); 
  leds[5] = CRGB(255, 0, 0); 
  turndown();
  delay(10);

  leds[3] = CRGB(255, 0, 0);
  leds[3] = CRGB(0, 255, 0); 
  turndown();
  delay(10);

   
  
  

  //left right disco
  /*for (int i = 0; i < NUM_LEDS/2; i++)
  {
    leds[i] = CRGB(random(0,255), 0, 0);
  }
  for (int i = NUM_LEDS; i >= NUM_LEDS/2 ; i--)
  {
    leds[i] = CRGB(0, random(0,255), 0);
  } */
}

void leftSide()
{ 
  for (int i = 0; i < NUM_LEDS/2; i++)
  {
    leds[i] = CRGB(random(0,255), 0, 0);
  }
}

void rightSide()
{ 
  for (int i = NUM_LEDS; i >= NUM_LEDS/2 ; i--)
  {
    leds[i] = CRGB(0, random(0,255), 0);
  }
}


void disco()
{ 
  for (int i = 0; i < NUM_LEDS; i=i+2)
  {
    leds[random(0,NUM_LEDS)] = CRGB(random(0,255), random(0,255), random(0,255));
  }
}

void loop() {
  loopWifi();

  if (lightPatternsAssigned > lightPatternSelected && millis() - timer > 1000 / frameRate)
  {
    timer = millis();
    lightPatternFunctions[lightPatternSelected]();
    loopLed();
  }


}


unsigned long blinkTimer = 0;

void off()
{

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 0);
    yield();
  }
  if (millis() - blinkTimer > 1000)
  {
    blinkTimer = millis();
   
    leds[0] = CRGB(0, 255, 0);
  }
}
void hsv() {


  //value = constrain(value,0,100);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(sliderValues[0], sliderValues[1], sliderValues[2]);
    yield();

  }

}

void rgb() {


  //value = constrain(value,0,100);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(sliderValues[0], sliderValues[1], sliderValues[2]);
    yield();

  }
  Serial.println(sliderValues[0]);
}



void red() {

  for (int i = 0; i < NUM_LEDS; i=i+2)
  {
    leds[i] = CRGB(255, 0, 0);
    yield();
  }
}

float value = 0;
void chase() {
  value = (value + 0.2f) ;
  if(value > NUM_LEDS)
  {
    value = 0;
  }

  for (int i = 0; i < NUM_LEDS; i++)
  {
    if ((int)value == i)
    {
      leds[i] = CRGB(255, 0, 0);
    }
    else
    {
      leds[i].fadeToBlackBy( 100 ); // 25% 64/256th
    }
    yield();
  }
}


void interactive()
{
  if (ULTRASONIC)
  {
    float value = usRead();
    int valueMapped = constrain(map(value, 0, 30, 255, 0), 0, 255);
    Serial.println(value);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(valueMapped, 0, 0);
      yield();
    }
  }

}



 


void cylon()
{

  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  Serial.print("x");

  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}
void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }


void turndown(){ for(int i = 0; i < NUM_LEDS; i++) {leds[i] = CRGB(0, 0, 0); }}




