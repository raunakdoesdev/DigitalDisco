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
CRGB leds[NUM_LEDS];
float switch_time;
float show_time;

const uint8_t BLUE = 1;
const uint8_t GREEN = 2;
const uint8_t RETAIN = 3;
const uint8_t RED = 4;
uint8_t play_state;
float start;
uint8_t next;
int notes;
int time_size;
int freq_size;
int timer;
int led_mode;
// -------------------------------------------------------------------------------


// wifi/server interaction variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WiFiClientSecure client;
WiFiClient client2;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 500; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1500; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];
char response_buffer[OUT_BUFFER_SIZE];
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  
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
  show_time = millis();
  switch_time = millis();
  play_state = IDLE;
  next = IDLE;
  timer = millis();
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}


void loop() {
  if (state == PLAYING && song_done == false) {
    play_song(used_times);
  }
  if (millis() - timer > QUERY_FREQ) {
    
    // sending request and processing ----------------------------------------------------------------------------
    send_request(USER, request_buffer, response_buffer);
    char* current = strtok(response_buffer, ",");
    int indicator = atoi(current);
    if (indicator == START) {
      current = strtok(NULL, ",");
      led_mode = atoi(current);
      current = strtok(NULL, ",");
      time_size = atoi(current);
      for (int i = 0; i < time_size; i++) {
        timestamps[i] = 1000 * atof(current);
        current = strtok(NULL, ",");
      }
      // freq_size = atoi (current);
      // current = strtok(NULL, ",");
      // for (int i = 0; i < freq_size; i++) {
      //   frequencies[i] = atoi(current);
      //   current = strtok(NULL, ",");
      // }
      song_done = false;
    } else if (indicator == PLAY) {
      current = strtok(NULL, ",");
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

void play_song(double* timestamp) {
  FastLED.show();
  Serial.println("FLASHING THINGSSS!!!!!");
  switch (play_state) {
    case IDLE:
      play_state = BLUE;
      start = millis();
      break;
    case BLUE:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Blue;
      }
      play_state = RETAIN;
      next = GREEN;
      break;
    case GREEN:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Red;
      }
      play_state = RETAIN;
      next = RED;
      break;
    case RED:
      for (int dot = 0; dot < 10; dot++) {
        leds[dot] = CRGB::Green;
      }
      play_state = RETAIN;
      next = BLUE;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes]) {
        FastLED.show();
      }
      notes++;
      if (notes > time_size) {
        FastLED.clear();
        next = IDLE;
        song_done = true;
        break;
      }
      play_state = next;
      Serial.printf("Switched colors: time = %f\n", millis() - start);
      break;
  }
}

void send_request(char* username, char* request, char* response) {
  memset(request, 0, sizeof(request));
  strcpy(request, "GET http://608dev-2.net/sandbox/sc/team00/final/comm.py?reason=espquery&user=");
  strcat(request, username);
  strcat(request, " HTTP/1.1\r\n");
  strcat(request, "Host: 608dev-2.net\r\n\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);  
}

void adjust_times(double* all_times, double* new_times, int* pos, double pause) {
  for (int i = 0; i < time_size; i++) {
    new_times[i] = all_times[i] - pause;
    if (new_times[i] < 0) {
      *pos = i + 1;
    }
  }
}
