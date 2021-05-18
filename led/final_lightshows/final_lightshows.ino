#include "FastLED.h"
#include <string.h>

// how many leds in your strip?
#define NUM_LEDS 60// actually >60 but to prevent power outage
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
float start;
uint8_t next;
int notes;  // "index" into times[]; notes=n --> we are on nth note
float freq = 0;
float last_freq = 0;
int dot;
int time_size;
int freq_size;
uint8_t palette_idx;
CHSV endclr;
CHSV midclr;
double timestamps[5000];
int frequencies[5000];

bool song_play;
bool processed;

//-----------------------------------------------------
const char NETWORK[] = "MIT GUEST";
const char PASSWORD[] = "";

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];
char response_buffer[OUT_BUFFER_SIZE];
//----------------------------------------------------------
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
//-----------------------------------------------------


void setup() {
  Serial.begin(115200);               // Set up serial port
  FastLED.addLeds<WS2811, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  song_play = false;
  processed = false;

  switch_time = millis();
  show_time = millis();
  state = IDLE;
  notes = 1;
  dot = 0;
  palette_idx = 0;
  show_time = millis();
  strcpy(response_buffer, "533, 0.0000, 0.5921, 1.5441, 2.0666, 2.6006, 3.1231, 3.6571, 4.1796, 4.7137, 5.2477, 5.7818, 6.3042, 6.8383, 7.0821, 7.3607, 7.8948, 8.4172, 8.6727, 8.9513, 9.2067, 9.4621, 10.0078, 10.5302, 11.0759, 11.5984, 12.1324, 12.6549, 13.1889, 13.7114, 14.2454, 14.7679, 15.3136, 15.5574, 15.8244, 16.3701, 16.8925, 17.4150, 17.6820, 17.9606, 18.4831, 18.7385, 18.9939, 19.2726, 19.5396, 19.7950, 20.0620, 20.3291, 20.5961, 20.8515, 21.1185, 21.6526, 22.1751, 22.7207, 22.9761, 23.2316, 23.4870, 23.7772, 24.0210, 24.2997, 24.5435, 24.8337, 25.3562, 25.6116, 25.8902, 26.1457, 26.4127, 26.6797, 26.9468, 27.2022, 27.4692, 27.7246, 28.0033, 28.2587, 28.5257, 28.7927, 29.0714, 29.3152, 29.5822, 29.8493, 30.1279, 30.6503, 31.1844, 31.4398, 31.7068, 32.2409, 32.4963, 32.7634, 33.2974, 33.5644, 33.8199, 34.0985, 34.3539, 34.5977, 34.8880, 35.4220, 35.9445, 36.4785, 36.7224, 37.0010, 37.5351, 37.7905, 38.0575, 38.3245, 38.5916, 39.1140, 39.3694, 39.6597, 39.9035, 40.1705, 40.7162, 40.9600, 41.2270, 41.4941, 41.7611, 42.0165, 42.2951, 42.5506, 42.8292, 43.3400, 43.6071, 43.8857, 44.4082, 44.6636, 44.9422, 45.4647, 45.7201, 45.9987, 46.5212, 46.7766, 47.0552, 47.5893, 48.1234, 48.3904, 48.6458, 48.9012, 49.1799, 49.4237, 49.7023, 49.9810, 50.2364, 50.7588, 51.0142, 51.2929, 51.5483, 51.8037, 52.0940, 52.3494, 52.8718, 53.1505, 53.4059, 53.6613, 53.9400, 54.1838, 54.4740, 54.7410, 54.9965, 55.2287, 55.5305, 56.0530, 56.5870, 56.8308, 57.1095, 57.3649, 57.6435, 57.8757, 58.1660, 58.7000, 58.9555, 59.2225, 59.4895, 59.7566, 60.0236, 60.2906, 60.8247, 61.0685, 61.3471, 61.8812, 62.1366, 62.4036, 62.9377, 63.1815, 63.4601, 63.7272, 63.9942, 64.5283, 65.0623, 65.3061, 65.5732, 66.1188, 66.3626, 66.6413, 67.1753, 67.4191, 67.6862, 68.2318, 68.4873, 68.7543, 69.2883, 69.8108, 70.0778, 70.3449, 70.6003, 70.8673, 71.4014, 71.6568, 71.9238, 72.1908, 72.4695, 72.9919, 73.5260, 73.7814, 74.0484, 74.5825, 74.8379, 75.1049, 75.6390, 76.1615, 76.6955, 77.2180, 77.7636, 78.2861, 78.8201, 79.0639, 79.3310, 79.6096, 79.8766, 80.1321, 80.3991, 80.9332, 81.4556, 82.0013, 82.2451, 82.5121, 82.7791, 83.0462, 83.3016, 83.5686, 84.1027, 84.3581, 84.6367, 84.8922, 85.1708, 85.4262, 85.6816, 85.9603, 86.2273, 86.7498, 87.0168, 87.2838, 87.8063, 88.0733, 88.3403, 88.8744, 89.1530, 89.3968, 89.9193, 90.4649, 90.7204, 90.9874, 91.5215, 91.7769, 92.0439, 92.3341, 92.5780, 93.1004, 93.6345, 93.8899, 94.1569, 94.4472, 94.6910, 95.2134, 95.4921, 95.7475, 96.2699, 96.5370, 96.8156, 97.3264, 97.5935, 97.8721, 98.1159, 98.3829, 98.9286, 99.1840, 99.4511, 99.7297, 99.9851, 100.2289, 100.5076, 101.0416, 101.3087, 101.5641, 101.8195, 102.0981, 102.3536, 102.6090, 102.8876, 103.1546, 103.6887, 103.9441, 104.2228, 104.7336, 105.0006, 105.2793, 105.8017, 106.0688, 106.3358, 106.8466, 107.1369, 107.3923, 107.6477, 107.9263, 108.1818, 108.4604, 108.7158, 108.9712, 109.2615, 109.5053, 110.0394, 110.5734, 110.8288, 111.0843, 111.3745, 111.6299, 112.1524, 112.4310, 112.6864, 113.2089, 113.4643, 113.7429, 114.2770, 114.5208, 114.7995, 115.3219, 115.8676, 116.1230, 116.3900, 116.6571, 116.9241, 117.1679, 117.4465, 117.7252, 117.9806, 118.2360, 118.5030, 118.7585, 119.0371, 119.2925, 119.5712, 120.0936, 120.3490, 120.6277, 121.1617, 121.4055, 121.6726, 121.9396, 122.2182, 122.7407, 123.2747, 123.7972, 124.3312, 124.5867, 124.8537, 125.3994, 125.6432, 125.9102, 126.1772, 126.4443, 126.9783, 127.5124, 128.0348, 128.2902, 128.5805, 128.8243, 129.0913, 129.3468, 129.6254, 129.8808, 130.1478, 130.4149, 130.6819, 131.2160, 131.7500, 132.2725, 132.8065, 133.0620, 133.3290, 133.5844, 133.8746, 134.1068, 134.3855, 134.6409, 134.9195, 135.1750, 135.4420, 135.6974, 135.9761, 136.2315, 136.5101, 136.7655, 137.0326, 137.5550, 138.0891, 138.6115, 139.1456, 139.6680, 140.2021, 140.7361, 141.2586, 141.7810, 142.3267, 142.8492, 143.3832, 143.9057, 144.4397, 144.9622, 145.4962, 146.0187, 146.5527, 147.0868, 147.6209, 148.1433, 148.6774, 149.1998, 149.7339, 150.2563, 150.7904, 151.3128, 151.8469, 152.3693, 152.9034, 153.4375, 153.9715, 154.4940, 155.0280, 155.5505, 156.0845, 156.6070, 157.1410, 157.6751, 158.2092, 158.7316, 159.2657, 159.7881, 160.3222, 160.5660, 160.8446, 161.1000, 161.3787, 161.9011, 162.1566, 162.4352, 162.9576, 163.5033, 164.0141, 164.5598, 165.0823, 165.3377, 165.6163, 166.1272, 166.6728, 167.1953, 167.7293, 168.2518, 168.7859, 169.3083, 169.8424, 170.3648, 170.8989, 171.4329, 171.7000, 171.9670, 172.4894, 173.0235, 173.5459, 173.7898, 174.0800, 174.6024, 175.1365, 175.6590, 176.1930, 176.7155, 177.2611, 177.7720, 178.3176, 178.5615, 178.8401, 179.3625, 179.8966, 180.4307, 180.9531, 181.4872, 182.0096, 182.5437, 183.0777, 183.6118, 184.1342, 184.6683, 185.1907, 185.7248, 186.2473, 186.7813, 187.3038, 187.8378, 188.3603, 188.6389, 188.8943, 189.4168, 189.6838, 189.9624, 190.4849, 190.7287, 191.0190, 191.5414, 192.0755, 192.5979, 193.1320, 193.6544, 194.1885, 194.7109, 194.9780, 195.2566, 195.7790, 196.3131, 196.8356, 197.3812, 197.8921, 198.4261, 198.9602, 199.4942, 200.0167, 200.5507, 201.0732, 211.5454,532, 248, 54, 151, 255, 139, 139, 139, 224, 127, 78, 224, 212, 121, 139, 97, 139, 139, 170, 97, 242, 255, 151, 139, 139, 121, 139, 151, 127, 127, 151, 151, 139, 139, 139, 170, 170, 242, 151, 248, 224, 151, 109, 194, 248, 151, 85, 121, 212, 139, 139, 97, 97, 12, 36, 182, 194, 200, 109, 151, 139, 212, 145, 182, 212, 182, 182, 182, 182, 151, 109, 48, 151, 151, 170, 212, 212, 255, 109, 212, 139, 30, 242, 42, 109, 242, 242, 242, 255, 255, 255, 255, 218, 218, 224, 218, 224, 224, 182, 194, 139, 182, 182, 200, 255, 255, 255, 97, 0, 109, 224, 121, 127, 42, 200, 212, 78, 236, 242, 151, 127, 248, 194, 212, 212, 121, 194, 236, 242, 242, 218, 127, 200, 200, 121, 218, 206, 236, 139, 24, 236, 255, 218, 127, 151, 200, 200, 151, 139, 212, 182, 182, 121, 127, 242, 242, 121, 224, 224, 212, 242, 242, 103, 224, 224, 224, 242, 236, 242, 242, 255, 242, 151, 242, 242, 255, 133, 170, 224, 212, 212, 194, 182, 188, 139, 24, 182, 182, 255, 151, 255, 218, 151, 212, 236, 139, 78, 255, 255, 170, 127, 151, 182, 255, 66, 121, 109, 182, 139, 139, 242, 255, 236, 127, 224, 242, 242, 176, 255, 248, 242, 242, 242, 255, 224, 236, 236, 255, 218, 255, 255, 97, 97, 157, 188, 224, 255, 200, 157, 236, 78, 255, 255, 242, 151, 242, 248, 194, 212, 212, 206, 194, 236, 242, 224, 224, 182, 200, 200, 218, 206, 242, 139, 24, 236, 242, 97, 151, 212, 200, 151, 127, 212, 236, 109, 139, 242, 139, 224, 236, 224, 212, 242, 242, 151, 224, 224, 176, 236, 236, 242, 236, 255, 242, 151, 182, 127, 248, 194, 212, 212, 139, 194, 236, 242, 242, 218, 182, 182, 200, 200, 139, 218, 206, 236, 139, 24, 236, 255, 127, 127, 212, 242, 151, 151, 212, 236, 139, 127, 242, 127, 224, 236, 212, 242, 242, 103, 224, 224, 224, 139, 236, 236, 236, 242, 255, 242, 127, 242, 97, 97, 103, 103, 103, 97, 97, 218, 176, 176, 24, 97, 97, 97, 97, 97, 103, 72, 218, 176, 194, 157, 24, 91, 182, 224, 248, 182, 127, 248, 248, 218, 176, 176, 24, 97, 151, 224, 248, 218, 224, 248, 78, 170, 151, 127, 176, 176, 230, 12, 6, 255, 200, 224, 212, 194, 188, 188, 188, 182, 255, 255, 218, 212, 236, 236, 255, 255, 200, 182, 255, 121, 182, 182, 188, 255, 236, 224, 139, 218, 151, 255, 255, 218, 224, 224, 236, 182, 200, 255, 255, 97, 139, 224, 255, 200, 212, 236, 236, 242, 151, 224, 248, 212, 218, 194, 236, 242, 242, 182, 127, 200, 218, 236, 139, 139, 236, 151, 200, 200, 139, 212, 236, 139, 242, 139, 224, 224, 212, 242, 224, 224, 236, 236, 236, 127, 127, 206, 248, 212, 212, 206, 236, 242, 188, 182, 200, 218, 236, 121, 236, 242, 151, 212, 224, 194, 212, 236, 127, 255, 255, 224, 224, 224, 212, 242, 224, 224, 236, 242, 236, 218, 127, 182, 248, 212, 212, 206, 236, 242, 194, 127, 200, 0");
}


void loop() {
  //process and parse timestamps string
  if (processed == false) {
    char* current = strtok(response_buffer, ",");
    time_size = atoi(current);
    current = strtok(NULL, ",");
    for (int i = 0; i < time_size; i++) {
      timestamps[i] = 1000 * atof(current);
      current = strtok(NULL, ",");
    }
    freq_size = atoi(current);
    current = strtok(NULL, ",");
    for (int i = 0; i < freq_size; i++) {
      frequencies[i] = atoi(current);
      current = strtok(NULL, ",");
    }
    processed = true;
    Serial.println("Starting light show in 3...");
    delay(1000);
    Serial.println("2...");
    delay(1000);
    Serial.println("1...");
    delay(1000);
    Serial.println("go!");
  }

  MAIN_PLAY_LIGHTSHOW(4); // SUNMEE PASS IN THE MODES VALUE HERE
}

void MAIN_PLAY_LIGHTSHOW(uint8_t modes) {
  if (modes == 0) {
    WAVES(timestamps, frequencies, 1); // fast
  }
  else if (modes == 1) {
    WAVES(timestamps, frequencies, 0); // slow
  }
  else if (modes == 2) {
    GRADIENTS(timestamps, heatmap_gp); // sun
  }
  else if (modes == 3) {
    GRADIENTS(timestamps, purple_gp); // moon
  }
  else if (modes == 4) {
    SPARKLES(timestamps, frequencies);
  }
  else if (modes == 5) {
    PULSES(timestamps, frequencies);
  }
}

void GRADIENTS(double* timestamp, CRGBPalette16 palette) {
  uint8_t scale = 4;  // speed up gradient change
  uint8_t num_leds = 50;

  switch (state) {
    case IDLE:
      state = COLOR;
      start = millis();
      break;
    case COLOR:
      fill_palette(leds, num_leds, palette_idx, 255 / num_leds, palette, 255, LINEARBLEND);
      state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes] / scale) {
        FastLED.show();
      }
      notes = notes++;
      palette_idx++;
      state = COLOR;
      if (notes > time_size * scale) { // reset notes
        notes = 1;
      }
      break;
  }
}


void PULSES(double* timestamp, int* frequencies) {
  uint8_t speed = beatsin8(100, 0, 255);
  FastLED.show();
  switch (state) {
    case IDLE:
      state = COLOR;
      start = millis();
      break;
    case COLOR:
      last_freq = freq;
      freq = frequencies[notes - 1];  // len(freq) = len(beats-1)
      endclr = blend(CHSV(last_freq, 250, 127), CHSV(freq, 250, 127), speed);
      midclr = blend(CHSV(freq, 250, 127), CHSV(last_freq, 250, 127), speed);
      state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes]) {
        cycle();
      }
      notes = notes + 1;
      if (notes > time_size) {
        FastLED.clear(); // end of show
        next = IDLE;
        break;
      }
      state = COLOR;
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
  switch (state) {
    case IDLE:
      FastLED.show();
      start = millis();
      state = COLOR;
      break;
    case COLOR: // beats-invariant
      if (millis() - start > timestamp[notes]) {
        notes++;
        Serial.printf("Switched colors: time = %f, freq = %f \n", millis() - start, freq);
      }
      if (notes > time_size) {
        state = IDLE;
        FastLED.clear();
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
  switch (state) {
    case IDLE:
      state = COLOR;
      start = millis();
      break;
    case COLOR:
      //Switch on an LED at random, with the current frequency
      freq = frequencies[notes - 1];
      leds[random8(0, NUM_LEDS - 1)] = CHSV(freq, 250, 127);
      state = RETAIN;
      break;
    case RETAIN:
      while (millis() - start < timestamp[notes]) {
        ;
      }
      notes = notes++;
      state = COLOR;
      if (notes > time_size) { // reset notes
        notes = 1;
      }
      break;
  }
  // Fade all LEDs down by 1 in brightness each time this is called
  fadeToBlackBy(leds, NUM_LEDS, 1);
  FastLED.show();
}
