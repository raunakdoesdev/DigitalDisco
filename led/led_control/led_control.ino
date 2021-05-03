#include "FastLED.h"


// how many leds in your strip?
#define NUM_LEDS 60
#define DATA_PIN 19  // 
CRGB leds[NUM_LEDS];
float switch_time;

//float times[100]; // the timestamps at which notes change

void setup() {
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  float times[] = {0.0, 1.0, 2.0, 4.0, 8.0, 4.0, 2.0, 0.0};  // integrate with alyssa's work (i.e. somehow receive times[])
  switch_time = millis();

}


void loop() {
  // basic light show
  FastLED.show();
  for (int dot = 0; dot < 10; dot++) {
    while (millis() - switch_time < 300) {
      leds[dot] = CRGB::Blue;
      FastLED.show();
      }
      switch_time = millis();
      leds[dot] = CRGB::Black;    
  }

}
