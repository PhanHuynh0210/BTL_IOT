#include "TaskNeoPixel.h"

#define NEOPIXEL_PIN 8
#define NEOPIXEL_COUNT 4


Adafruit_NeoPixel NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setAll(uint32_t color) {
    for (int i = 0; i < NEOPIXEL_COUNT; i++) {
        NeoPixel.setPixelColor(i, color);
    }
}

void TaskNeoPixel(void *pvParameters){
    NeoPixel.begin();
    NeoPixel.clear();
    NeoPixel.show();

    while (true) {

        if (xSemaphoreTake(humLowSem, 0)) {
            setAll(NeoPixel.Color(0, 0, 255));   // xanh dương
        }

        else if (xSemaphoreTake(humMidSem, 0)) {
            setAll(NeoPixel.Color(0, 255, 0));   // xanh lá
        }

        else if (xSemaphoreTake(humHighSem, 0)) {
            setAll(NeoPixel.Color(255, 0, 0));   // đỏ
        }

        NeoPixel.show();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
