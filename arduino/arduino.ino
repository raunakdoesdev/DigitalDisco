#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

uint32_t last = 0;

char network[] = "RaunakDorm";  //SSID for 6.08 Lab
char password[] = "Raunak123!"; //Password for 6.08 Lab
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

void setup() {
  Serial.begin(115200); // Set up serial port
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font

  setup_internet();
  
  tft.setCursor(0, 0, 2); //set cursor, font size 1
  tft.printf("Hello World");
}


void get_server_state(){
    char request_buffer[200];
    sprintf(request_buffer, "GET /sandbox/sc/raunakc/final/comm.py HTTP/1.1\r\n");
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

void loop() {
  if(millis() - last > 1000){
    get_server_state();
    
    tft.fillScreen(TFT_WHITE); //fill background
    tft.setCursor(0, 0, 2); //set cursor, font size 1
    tft.printf("%s", response);
    last = millis();
  }
}
