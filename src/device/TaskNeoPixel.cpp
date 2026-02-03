#include "TaskNeoPixel.h"

#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 45
#define NEOPIXEL_COUNT 1

static SemaphoreHandle_t neoPixelSemaphore = nullptr;
static volatile uint8_t humidityLevel = 0;

static Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void NeoPixelSetHumidityLevel(uint8_t level)
{
    humidityLevel = level;
    if (neoPixelSemaphore != nullptr)
    {
        xSemaphoreGive(neoPixelSemaphore);
    }
}

void TaskNeoPixel(void *pvParameters)
{
    (void)pvParameters;

    pixels.begin();
    pixels.clear();
    pixels.show();

    neoPixelSemaphore = xSemaphoreCreateBinary();

    while (true)
    {
        if (xSemaphoreTake(neoPixelSemaphore, portMAX_DELAY) == pdTRUE)
        {
            uint32_t color;

            if (humidityLevel == 0)
            {
                color = pixels.Color(0, 0, 255);
            }
            else if (humidityLevel == 1)
            {
                color = pixels.Color(0, 255, 0);
            }
            else
            {
                color = pixels.Color(255, 0, 0);
            }

            pixels.setPixelColor(0, color);
            pixels.show();
        }
    }
}
