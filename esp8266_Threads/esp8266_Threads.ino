#include <TimedAction.h>

//pin / state variables
#define ledPin D2
boolean ledState = false;
 

 void blink(){
  ledState ? ledState=false : ledState=true;
  digitalWrite(ledPin,ledState);
  Console.system.println("LN");
} 

//this initializes a TimedAction class that will change the state of an LED every second.
TimedAction timedAction = TimedAction(1000,blink);
 

 
void setup(){
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,ledState);
}
 
void loop(){
  timedAction.check();
}
 

