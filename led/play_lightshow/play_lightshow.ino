#include "FastLED.h"


// how many leds in your strip?
#define NUM_LEDS 60
#define DATA_PIN 19  // 
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
float switch_time;
float show_time;

const uint8_t IDLE = 0;
const uint8_t BLUE = 1;
const uint8_t GREEN = 2;
const uint8_t RED = 3;
const uint8_t COLOR = 4;
const uint8_t RETAIN = 5;
uint8_t state;
int notes;  // "index" into times[]; notes=n --> we are on nth note
float freq;
int dot;

//float times[100]; // the timestamps at which notes change

void setup() {
  Serial.begin(115200);               // Set up serial port
  FastLED.addLeds<WS2811, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  //float times[] = {0.0, 0.5, 1.0, 2.0, 4.0, 8.0};  // integrate with alyssa's work (i.e. somehow receive times[])
  switch_time = millis();
  show_time = millis();
  state = IDLE;
  notes = 1;
  dot = 0;  // for light_show_2 (make sure to take care of initialization = 0 at every song)
  show_time = millis();
}

////////////////////Beats, Frequency////////////////////////////////////////
// frequency passed as 2 * (one of 128 pitch bins)
float times[] = {0.0 * 1000, 2.0 * 1000, 4.0 * 1000, 6.0 * 1000, 8.0 * 1000, 10.0 * 1000, 12.0 * 1000, 14.0 * 1000, 16.0 * 1000, 18.0 * 1000, 20.0 * 1000, 22.0 * 1000, 24.0 * 1000}; // integrate with alyssa's work (i.e. somehow receive times[])
float freqs[] = { // toy example
  0, 32, 64, 96, 128, 160, 192, 224, 0, 32, 64, 96, 128, 160, 192, 224    //beat 4
};

////////////////////////////////////////////////////////////////////////////
float start;
float t;

uint8_t next = IDLE;

void light_show_1() {
  FastLED.show();
  switch (state) {
    case IDLE:
      state = COLOR;
      start = millis();
      break;
    case COLOR:
      freq = freqs[notes - 1];  // len(freq) = len(beats-1)
      for (int dot = 0; dot < NUM_LEDS; dot++) {
        leds[dot] = CHSV(freq,255,127);
      }
      state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < times[notes]) {
        FastLED.show(); 
      }
      notes = notes + 1;
      if (notes > 14) {
        //FastLED.clear(); // end of show
        next = IDLE;
        break;
      }
      state = COLOR;
      Serial.printf("Switched colors: time = %f, freq = %f \n", millis() - start, freq);
      break;
  }
}

void light_show_2() {
  FastLED.show();
  switch (state) {
    case IDLE:
      state = COLOR;
      start = millis();
      break;
    case COLOR:
      freq = freqs[notes - 1];  // len(freq) = len(beats-1)
      leds[dot] = CHSV(freq,255,127); // change single dot (starts at 0)
      leds[dot+1] = CHSV(freq,255,127); 
      state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < times[notes]) {
        FastLED.show(); 
      }
      notes = notes + 1;
      dot = dot + 2;
      if (notes > 14) {
        //FastLED.clear(); // end of show
        next = IDLE;
        break;
      }
      state = COLOR;
      Serial.printf("Switched colors: time = %f, freq = %f \n", millis() - start, freq);
      break;
  }
}



void loop() {
  //light_show_1(); // all led's change HUE together, on beat (very basic)
  light_show_2(); // led's change HUE sequentially (one led at a time (can customize increments of course)), on beat
  
}
