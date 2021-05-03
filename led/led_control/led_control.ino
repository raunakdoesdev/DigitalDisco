#include "FastLED.h"


// how many leds in your strip?
#define NUM_LEDS 60
#define DATA_PIN 19  // 
CRGB leds[NUM_LEDS];
float switch_time;
float show_time;

const uint8_t IDLE = 0;
const uint8_t BLUE = 1;
const uint8_t GREEN = 2;
uint8_t state;

//float times[100]; // the timestamps at which notes change

void setup() {
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  float times[] = {0.0, 1.0, 2.0, 4.0, 8.0, 4.0, 2.0, 0.0};  // integrate with alyssa's work (i.e. somehow receive times[])
  switch_time = millis();
  show_time = millis();
  state = IDLE;
}

void light_show_1() {
  // basic light show #1: flash blue ("chase across")
  FastLED.show();
  for (int dot = 0; dot < 10; dot++) {
    while (millis() - switch_time < 300) {
      leds[dot] = CRGB::Blue;
      FastLED.show();
    }
    switch_time = millis();
    leds[dot] = CRGB::Black;
  }
  //show_time = millis();
}

void light_show_2() {
  // basic light show #1: alternate colors
  FastLED.show();
  switch (state) {
    case IDLE:
      state = BLUE;
      switch_time = millis();
      break;
    case BLUE:
      while (millis() - switch_time < 300) {
        for (int dot = 0; dot < 10; dot++) {
          leds[dot] = CRGB::Blue;
        }
        FastLED.show();
      }
      state = GREEN;
      switch_time = millis();
      break;
    case GREEN:
      while (millis() - switch_time < 300) {
        for (int dot = 0; dot < 10; dot++) {
          leds[dot] = CRGB::Red;
        }
        FastLED.show();
      }
      switch_time = millis();
      state = BLUE;
      break;
  }
}

void loop() {
  //
  //  while (millis() - show_time < 500) { // play show for 10 seconds
  //    light_show_1();
  //  }
  // basic light show #2: alternate colors
  show_time = millis();
  while (millis() - show_time < 10000) { // play show for 10 seconds
    light_show_2();
  }

}
