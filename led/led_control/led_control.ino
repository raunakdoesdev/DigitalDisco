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
const uint8_t RETAIN = 3;
const uint8_t RED = 4;
uint8_t state;
int notes;

//float times[100]; // the timestamps at which notes change

void setup() {
  Serial.begin(115200);               // Set up serial port
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  //float times[] = {0.0, 0.5, 1.0, 2.0, 4.0, 8.0};  // integrate with alyssa's work (i.e. somehow receive times[])
  switch_time = millis();
  show_time = millis();
  state = IDLE;
  notes = 1;
  show_time = millis();
}

void light_show_1() {
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

void light_show_2() {
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

void light_show_3() {
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
      state = RED;
      break;
    case RED:
      while (millis() - switch_time < 300) {
        for (int dot = 0; dot < 10; dot++) {
          leds[dot] = CRGB::Green;
        }
        FastLED.show();
      }
      switch_time = millis();
      state = BLUE;
      break;
  }
}

float times[] = {0.0 * 1000, 2.0 * 1000, 4.0 * 1000, 8.0 * 1000, 10.0 * 1000, 14.0 * 1000, 18.0 * 1000}; // integrate with alyssa's work (i.e. somehow receive times[])
float start;
float t;

uint8_t next = IDLE;
void light_show_4() {
  FastLED.show();
  switch (state) {
    case IDLE:
      state = BLUE;
      start = millis();
      break;
    case BLUE:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Blue;
      }
      state = RETAIN;
      next = GREEN;
      break;
    case GREEN:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Red;
      }
      state = RETAIN;
      next = RED;
      break;
    case RED:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Green;
      }
      state = RETAIN;
      next = BLUE;
      break;
    case RETAIN:
      while (millis() - start < times[notes]) {
        FastLED.show();
      }
      notes = notes + 1;
      if (notes > 7) {
        FastLED.clear(); // end of show
        next = IDLE;
        break;
      }
      state = next;
      Serial.printf("Switched colors: time = %f\n", millis() - start);
      break;
  }
}



void loop() {
  // basic light show #1: "chase" colors
  while (millis() - show_time < 10000) { // play show for 10 seconds
    light_show_1();
  }
  //   basic light show #2: alternate colors
  show_time = millis();
  while (millis() - show_time < 10000) { // play show for 10 seconds
    light_show_2();
  }
  // light show #3: with time integration
  show_time = millis();
  while (millis() - show_time < 10000) { // play show for 10 seconds
    light_show_3();
  }
//  light_show_4();
}
