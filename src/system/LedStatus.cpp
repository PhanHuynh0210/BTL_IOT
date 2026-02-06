#include "LedStatus.h"

#define NEOPIXEL_PIN 45
#define NEOPIXEL_COUNT 1


Adafruit_NeoPixel statusNeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


void LedStatus(void *pvParameters){
    statusNeoPixel.begin();
    statusNeoPixel.clear();
    statusNeoPixel.show();

    system_status st;

    while (1) {
        if (xQueueReceive(ledQueue, &st, portMAX_DELAY)) {
            switch (st) {

                case STATUS_BOOTING:      
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(255, 150, 0));
                    break;

                case STATUS_AP_MODE:      
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(0, 0, 255));
                    break;

                case STATUS_CONNECTING:   
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(150, 0, 150));
                    break;

                case STATUS_NORMAL:       
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(0, 255, 0));
                    break;

                case STATUS_ERROR:         
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(255, 0, 0));
                    break;

                case STATUS_OTA_UPDATE:   
                    statusNeoPixel.setPixelColor(0, statusNeoPixel.Color(255, 255, 255));
                    break;
            }
                statusNeoPixel.show();
            }
    }
}
