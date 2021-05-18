#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include "FastLED.h"
#include <string.h>


// fields to change if necessary -------------------------------------------------
char USER[] = "sunchoi"; // change username here
const int QUERY_FREQ = 250;
const char NETWORK[] = "MIT GUEST";
const char PASSWORD[] = "";
//--------------------------------------------------------------------------------


const uint8_t IDLE = 0;


// song control variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const uint8_t PLAYING = 1;
const uint8_t PAUSED = 2;
uint8_t state;

const uint8_t NO_CHANGE = 0;
const uint8_t PAUSE = 1;
const uint8_t PLAY = 2;
const uint8_t START = 3;

double timestamps[1000];
double used_times[1000];
int frequencies[1000];

bool song_done;
double paused_point;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// led control variables ---------------------------------------------------------
#define NUM_LEDS 60
#define DATA_PIN 19 //
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
float switch_time;
float show_time;

const uint8_t BLUE = 1;
const uint8_t GREEN = 2;
const uint8_t RED = 3;
const uint8_t COLOR = 4;
const uint8_t RETAIN = 5;
uint8_t play_state;
float start;
uint8_t next;
uint16_t notes; // "index" into times[]; notes=n --> we are on nth note
float freq = 0;
float last_freq = 0;
int dot;
uint16_t time_size;
uint16_t freq_size;
uint8_t palette_idx;
CHSV endclr;
CHSV midclr;
int timer;
uint8_t led_mode;
// -------------------------------------------------------------------------------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DEFINE_GRADIENT_PALETTE( heatmap_gp ) {
  0,     0,  0,  0,   //black
  128,   255,  0,  0,   //red
  224,   255, 255,  0,  //bright yellow
  255,   255, 255, 255
}; //full white

DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,     0,  0,  0,
  128, 128, 0, 128,
  200, 0, 0, 255, //blue
  255,   255, 255, 255
}; //full white
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// wifi/server interaction variables ---------------------------------------------
WiFiClientSecure client;
WiFiClient client2;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 500; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];
char response_buffer[OUT_BUFFER_SIZE];
//--------------------------------------------------------------------------------


void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  
  // wifi setup -----------------------------------------------------------------------------------------------------------------------
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  WiFi.begin(NETWORK, PASSWORD);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  //----------------------------------------------------------------------------------------------------------------------------------


  // initializing variables/timers for state machine and led control ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  song_done = true;
  notes = 1;
  dot = 0;
  palette_idx = 0;
  show_time = millis();
  switch_time = millis();
  state = IDLE;
  play_state = IDLE;
  next = IDLE;
  timer = millis();
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}


void loop() {
  if (state == PLAYING && song_done == false) {
    MAIN_PLAY_LIGHTSHOW(used_times, frequencies, led_mode);
  }
  if (millis() - timer > QUERY_FREQ) {
    
    // sending request and processing ----------------------------------------------------------------------------
    send_request(USER, request_buffer, response_buffer);
    char* current = strtok(response_buffer, ", ");
    int indicator = atoi(current);
    Serial.println(indicator);
    if (indicator == START) {
      current = strtok(NULL, ", ");
      led_mode = atoi(current);
      Serial.println(led_mode);
      current = strtok(NULL, ", ");
      time_size = atoi(current);
      Serial.println(time_size);
      current = strtok(NULL, ", ");
      for (int i = 0; i < time_size; i++) {
        timestamps[i] = 1000 * atof(current);
        current = strtok(NULL, ", ");
      }
      freq_size = atoi(current);
      current = strtok(NULL, ", ");
      for (int i = 0; i < freq_size; i++) {
        frequencies[i] = atoi(current);
        current = strtok(NULL, ", ");
      }
      song_done = false;
    } else if (indicator == PLAY) {
      current = strtok(NULL, ", ");
      paused_point = 1000 * atof(current);
    }
    //------------------------------------------------------------------------------------------------------------

    
    switch (state) {
      case IDLE:
        if (indicator == START) {
          song_done = false;
          notes = 1;
          show_time = millis();
          switch_time = millis();
          play_state = IDLE;
          next = IDLE;
          adjust_times(timestamps, used_times, &notes, 0.0);
          state = PLAYING;
        }
        break;
      case PLAYING:
        if (song_done == true) {
          state = IDLE;
          notes = 1;
          show_time = millis();
          switch_time = millis();
          play_state = IDLE;
          next = IDLE;
        } else if (indicator == PAUSE) {
          FastLED.clear();
          FastLED.show();
          state = PAUSED;
        } else if (indicator == START) {
          FastLED.clear();
          FastLED.show();
          song_done = false;
          notes = 1;
          show_time = millis();
          switch_time = millis();
          play_state = IDLE;
          next = IDLE;
          adjust_times(timestamps, used_times, &notes, 0.0);
          state = PLAYING;
        }
        break;
      case PAUSED:
        if (indicator == PLAY) {
          adjust_times(timestamps, used_times, &notes, paused_point);
          state = PLAYING;
        } else if (indicator == START) {
          FastLED.clear();
          FastLED.show();
          song_done = false;
          notes = 1;
          show_time = millis();
          switch_time = millis();
          play_state = IDLE;
          next = IDLE;
          adjust_times(timestamps, used_times, &notes, 0.0);
          state = PLAYING;
        }
        break;
    }
    
    timer = millis();
  }
}

void MAIN_PLAY_LIGHTSHOW(double* times, int* freqs, uint8_t modes) {
  if (modes == 0) {
    WAVES(times, freqs, 1); // fast
  }
  else if (modes == 1) {
    WAVES(times, freqs, 0); // slow
  }
  else if (modes == 2) {
    GRADIENTS(times, heatmap_gp); // sun
  }
  else if (modes == 3) {
    GRADIENTS(times, purple_gp); // moon
  }
  else if (modes == 4) {
    SPARKLES(times, freqs);
  }
  else if (modes == 5) {
    PULSES(times, freqs);
  }
}

void GRADIENTS(double* timestamp, CRGBPalette16 palette) {
  float scale = 5.0;  // speed up gradient change
  uint8_t num_leds = 50;
  switch (play_state) {
    case IDLE:
      play_state = COLOR;
      start = millis();
      break;
    case COLOR:
      fill_palette(leds, num_leds, palette_idx, 255 / num_leds, palette, 255, LINEARBLEND);
      play_state = RETAIN;
      break;
    case RETAIN:
      if (millis()- start > timestamp[notes] / scale){
        notes++;
        palette_idx++;
      }
      if (notes > time_size) { // reset notes
        notes = 1;
        song_done = true;
        next = IDLE;
        start = millis();
      }
      play_state = COLOR;
      FastLED.show();
      break;
  }
}

void PULSES(double* timestamp, int* frequencies) {
  uint8_t speed = beatsin8(100, 0, 255);
  FastLED.show();
  switch (play_state) {
    case IDLE:
      play_state = COLOR;
      start = millis();
      break;
    case COLOR:
      last_freq = freq;
      freq = frequencies[notes - 1];  // len(freq) = len(beats-1)
      endclr = blend(CHSV(last_freq, 250, 127), CHSV(freq, 250, 127), speed);
      midclr = blend(CHSV(freq, 250, 127), CHSV(last_freq, 250, 127), speed);
      play_state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes]) {
        cycle();
      }
      notes = notes + 1;
      if (notes > time_size) {
        FastLED.clear(); // end of show
        next = IDLE;
        song_done = true;
        notes = 1;
        break;
      }
      play_state = COLOR;
      Serial.printf("Switched colors: time = %f, freq = %f \n", millis() - start, freq);
      break;
  }
}
void cycle() {
  fill_gradient_RGB(leds, 0, endclr, NUM_LEDS / 2, midclr);
  fill_gradient_RGB(leds, NUM_LEDS / 2 + 1, midclr, NUM_LEDS, endclr);
}


void WAVES(double* timestamp, int* frequencies, uint8_t speedy) {
  uint8_t sinBeat; uint8_t sinBeat2; uint8_t sinBeat3;
  if (speedy == 0) { // slow
    sinBeat = beatsin8(2, 0, NUM_LEDS - 1, 0, 0);
    sinBeat2 = beatsin8(2, 0, NUM_LEDS - 1, 0, 128);
    sinBeat3 = beatsin8(2, 0, NUM_LEDS - 1, 0, 255);
  }
  else {
    sinBeat = beatsin8(10, 0, NUM_LEDS - 1, 0, 0);
    sinBeat2 = beatsin8(10, 0, NUM_LEDS - 1, 0, 128);
    sinBeat3 = beatsin8(10, 0, NUM_LEDS - 1, 0, 255);
  }
  switch (play_state) {
    case IDLE:
      FastLED.show();
      start = millis();
      play_state = COLOR;
      break;
    case COLOR: // beats-invariant
      if (millis() - start > timestamp[notes]) {
        notes++;
        Serial.printf("Switched colors: time = %f, freq = %f \n", millis() - start, freq);
      }
      if (notes > time_size) {
        play_state = IDLE;
        next = IDLE;
        song_done = true;
        notes = 1;
        FastLED.clear();
        FastLED.clear();
        break;
      }
      freq = frequencies[notes - 1];
      fill_gradient_RGB(leds, 0,  CHSV(100, 0, 127), NUM_LEDS,  CHSV(100, 0, 50)); // white background
      leds[sinBeat] = CHSV(freq, 250, 127);
      leds[sinBeat2] = CHSV(freq, 250, 127);
      leds[sinBeat3] = CHSV(freq, 250, 127);
      fadeToBlackBy(leds, NUM_LEDS, 2);
      FastLED.show();
      break;
  }
}

void SPARKLES(double* timestamp, int* frequencies) {
  CRGBPalette16 palette = heatmap_gp;  // define palette
  switch (play_state) {
    case IDLE:
      play_state = COLOR;
      start = millis();
      break;
    case COLOR:
      //Switch on an LED at random, with the current frequency
      freq = frequencies[notes - 1];
      leds[random8(0, NUM_LEDS - 1)] = CHSV(freq, 250, 127);
      play_state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes]) {
        ;
      }
      notes = notes++;
      palette_idx++;
      play_state = COLOR;
      if (notes > time_size) { // reset notes
        notes = 1;
      }
      break;
  }
  // Fade all LEDs down by 1 in brightness each time this is called
  fadeToBlackBy(leds, NUM_LEDS, 1);
  FastLED.show();
}

//void play_song(double* timestamp) {
//  FastLED.show();
//  switch (play_state) {
//    case IDLE:
//      play_state = BLUE;
//      start = millis();
//      break;
//    case BLUE:
//      for (int dot = 0; dot < 10; dot++) {
//        leds[dot] = CRGB::Blue;
//      }
//      play_state = RETAIN;
//      next = GREEN;
//      break;
//    case GREEN:
//      for (int dot = 0; dot < 10; dot++) {
//        leds[dot] = CRGB::Red;
//      }
//      play_state = RETAIN;
//      next = RED;
//      break;
//    case RED:
//      for (int dot = 0; dot < 10; dot++) {
//        leds[dot] = CRGB::Green;
//      }
//      play_state = RETAIN;
//      next = BLUE;
//      break;
//    case RETAIN:
//      while (millis() - start < timestamp[notes]) {
//        FastLED.show();
//      }
//      notes++;
//      if (notes > time_size) {
//        FastLED.clear();
//        next = IDLE;
//        song_done = true;
//        break;
//      }
//      play_state = next;
//      Serial.printf("Switched colors: time = %f\n", millis() - start);
//      break;
//  }
//}

void send_request(char* username, char* request, char* response) {
  memset(request, 0, sizeof(request));
  strcpy(request, "GET http://608dev-2.net/sandbox/sc/team00/final/comm.py?reason=espquery&user=");
  strcat(request, username);
  strcat(request, " HTTP/1.1\r\n");
  strcat(request, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);  
}

void adjust_times(double* all_times, double* new_times, uint16_t* pos, double pause) {
  for (int i = 0; i < time_size; i++) {
    new_times[i] = all_times[i] - pause;
    if (new_times[i] < 0) {
      *pos = i + 1;
    }
  }
}
