#include "TaskNeoPixel.h"

#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 45
#define NEOPIXEL_COUNT 1


static Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void TaskNeoPixel(void *pvParameters){
    pixels.begin();
    pixels.clear();
    pixels.show();

    while (true) {

        if (xSemaphoreTake(humLowSem, portMAX_DELAY)) {
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
        }

        if (xSemaphoreTake(humMidSem, 0)) {
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); 
        }

        if (xSemaphoreTake(humHighSem, 0)) {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); 
        }

        pixels.show();
    }
}
