// http://www.cplusplus.com/forum/beginner/4639/


void addLightPattern(lightPatternFunction _function, String _name)
{
  if (allocatedLightPatterns > lightPatternsAssigned)
  {
    lightPatternFunctions[lightPatternsAssigned] = _function;
    lightPatternNames[lightPatternsAssigned] = _name;
    lightPatternsAssigned++;
  }

}



void setupLed()
{
 FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
 FastLED.setMaxRefreshRate(200); 

  
}


void loopLed()
{
 


  FastLED.show(scale);
}

