#include "TaskNeoPixel.h"

#define NEOPIXEL_PIN 45
#define NEOPIXEL_COUNT 1


Adafruit_NeoPixel statusNeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


void TaskNeoPixel(void *pvParameters){
    statusNeoPixel.begin();
    statusNeoPixel.clear();
    statusNeoPixel.show();

    while (true) {

        if (xSemaphoreTake(humLowSem, 0)) {
            statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(0, 0, 255));
        }

        else if (xSemaphoreTake(humMidSem, 0)) {
            statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(0, 255, 0)); 
        }

        else if (xSemaphoreTake(humHighSem, 0)) {
            statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(255, 0, 0)); 
        }

        statusNeoPixel.show();
    }
}
