#include <SPI.h>

const int UPDATE_PERIOD = 20;
const uint8_t PIN_1 = 5; //button 1
const uint8_t PIN_2 = 0; //button 2
uint32_t primary_timer;
float sample_rate = 2000; //Hz
float sample_period = (int)(1e6 / sample_rate);

int render_counter;
const uint8_t CORRELATION_SIZE = 5;
uint16_t raw_reading;
float readings[CORRELATION_SIZE] = {0};
uint8_t r_index = 0;
float scaled_reading;
float max_c = 0;
float max_x = 0;
float min_x = 0;
bool waiting = true;

float song[100] = {0.7600250244140625, 0.2997711181640625, -0.0601104736328125, -0.6448379516601562, 0.15111083984375, 0.4025611877441406, 0.2918609619140625, -0.054456329345703124, -0.10596923828125, -0.23829116821289062, -0.1533660888671875, 0.04453582763671875, 0.0903045654296875, -0.031222534179687504, 0.025921630859375003, -0.10894393920898438, 0.0610992431640625, 0.017116546630859375, 0.06913299560546876, 0.19110565185546874, 0.1349639892578125, 0.06181182861328125, -0.143487548828125, -0.06424789428710938, -0.00206298828125, -0.00124359130859375, -0.07711944580078126, -0.03532333374023437, -0.044549560546875, -0.19733734130859376, -0.15547943115234375, -0.151898193359375, 0.05438537597656251, 0.20585708618164061, -0.014532470703125008, 0.053722381591796875, -0.10164794921875, -0.10778350830078125, -0.15329437255859374, 0.025157165527343747, 0.0474090576171875, 0.11090316772460937, 0.03000640869140625, 0.11139755249023438, -0.1267547607421875, -0.049831390380859375, -0.1122100830078125, 0.053997802734375, 0.079034423828125, 0.08487319946289062, 0.10469818115234375, 0.04984893798828125, -0.0514404296875, -0.15439376831054688, -0.021484375, 0.031036376953125, 0.1045166015625, 0.07505264282226562, -0.0184295654296875, 0.00905609130859375, -0.1688385009765625, -0.12885513305664062, -0.09060821533203126, -0.09439926147460938, 0.0754180908203125, 0.1578826904296875, 0.04230499267578125, -0.0124114990234375, -0.046856689453125, -0.12806930541992187, -0.037384033203125, -0.1704498291015625, -0.103857421875, 0.03780899047851563, 0.0764312744140625, 0.10989761352539062, 0.1290374755859375, -0.03388519287109375, -0.07750091552734376, -0.03637924194335937, -0.140533447265625, -0.036229705810546874, -0.0629425048828125, 0.026236724853515626, 0.0680938720703125, 0.16880416870117188, 0.061004638671875,
                   0.10985641479492188, -0.0452667236328125, -0.023757171630859376, -0.1255645751953125, -0.045467376708984375, 0.0363372802734375, 0.09856414794921875, 0.006770324707031251, 0.025753021240234375, -0.0946868896484375, -0.07562484741210937, -0.05287628173828125, -0.09205322265624999
                  };
void offset_and_normalize(float *x, float*out, int index = 0) {
  float sum = 0;
  for (int i = 0; i < CORRELATION_SIZE; i++) {
    sum += x[i];
  }
  float x_bar = sum / CORRELATION_SIZE;
  sum = 0;
  for (int i = 0; i < CORRELATION_SIZE; i++) {
    sum += pow((x[i] - x_bar), 2);
  }
  float norm_x = sqrt(sum);
  for (int i = 0; i < CORRELATION_SIZE; i++) {
    out[i] = (x[(i + index) % CORRELATION_SIZE] - x_bar) / norm_x;
  }
}
//def offset_and_normalize(x):
//    x_bar = sum(x)/len(x)
//    x = [elt-x_bar for elt in x]
//    norm_x = math.sqrt(sum(elt**2 for elt in x))
//    x = [elt/norm_x for elt in x]
//    return x
float correlation(float*x, float *y, int index = 0) {
  float norm_x[CORRELATION_SIZE] = {0};
  float norm_y[CORRELATION_SIZE] = {0};
  offset_and_normalize(x, norm_x, index = index);
  offset_and_normalize(y, norm_y);
  float sum = 0;
  for (int i = 0; i < CORRELATION_SIZE; i++) {
    sum += norm_x[i] * norm_y[i];
  }
  return sum;
}
//def correlation(x,y):
//    x = offset_and_normalize(x)
//    y = offset_and_normalize(y)
//    return sum(elt_x*elt_y for elt_x, elt_y in zip(x,y))


void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  primary_timer = micros();
  render_counter = 0;

}

void loop() {
  while (!digitalRead(PIN_1)) {
    render_counter++;
    if (render_counter % UPDATE_PERIOD == 0) {
      raw_reading = analogRead(A0);
      scaled_reading = raw_reading * 3.3 / 4096 - 1.18;
      readings[r_index] = scaled_reading;
      //    float c = correlation(readings, song, r_index);

      //max
      if ((readings[r_index] < readings[(r_index - 1) % CORRELATION_SIZE]) && (readings[(r_index - 2) % CORRELATION_SIZE] < readings[(r_index - 1) % CORRELATION_SIZE])) {
        max_x = readings[(r_index - 1) % CORRELATION_SIZE];
        if (((max_x - min_x) > 0.6)&& waiting) {
          waiting = false;
          Serial.printf("scaled reading: %f, %f\n", max_x - min_x);
        }
      }
      //min
      if ((readings[r_index] > readings[(r_index - 1) % CORRELATION_SIZE]) && (readings[(r_index - 2) % CORRELATION_SIZE] > readings[(r_index - 1) % CORRELATION_SIZE])) {
        min_x = readings[(r_index - 1) % CORRELATION_SIZE];
        if (((max_x - min_x) > 0.6)&& waiting) {
          Serial.printf("scaled reading: %f, %f\n", max_x - min_x);
          waiting = false;
        }
      }

      r_index = (r_index + 1) % CORRELATION_SIZE;
    }
    while (micros() > primary_timer && micros() - primary_timer < sample_period); //prevents rollover glitch every 71 minutes...not super needed
    primary_timer = micros();
    waiting = true;
  }
}
