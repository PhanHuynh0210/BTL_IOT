#include "TaskDHT11.h"

#include <DHT11.h>

#define DHT11_PIN 21
#define DHT11_LED_PIN 48

QueueHandle_t delayQueueDHT11;

DHT11 dht11(DHT11_PIN);

void TaskBlinkLED11(void *pvParameters){
    TickType_t delayTime = pdMS_TO_TICKS(1000);

    while (true){
        xQueueReceive(delayQueueDHT11, &delayTime, 0);
        digitalWrite(DHT11_LED_PIN, !digitalRead(DHT11_LED_PIN));
        vTaskDelay(delayTime);
    }
    
    
}

void TaskDHT11(void *pvParameters)
{
    (void)pvParameters;
    pinMode(DHT11_LED_PIN, OUTPUT);

    delayQueueDHT11 = xQueueCreate(1, sizeof(TickType_t));

    xTaskCreate(TaskBlinkLED11, "TaskBlinkLED11", 4096, NULL, 1, NULL);

    while (true)
    {
        int temperature = 0;
        int humidity = 0;
        int result = dht11.readTemperatureHumidity(temperature, humidity);

        if (result == 0)
        {
            TickType_t delayValue;

            if (temperature < 30)
            {
                delayValue = pdMS_TO_TICKS(100);
            }
            else if (temperature < 35)
            {
                delayValue = pdMS_TO_TICKS(1000);
            }
            else
            {
                delayValue = pdMS_TO_TICKS(200);
            }

            xQueueOverwrite(delayQueueDHT11, &delayValue);

            uint8_t level;
            if (humidity < 40)
            {
                level = 0;
            }
            else if (humidity < 60)
            {
                level = 1;
            }
            else
            {
                level = 2;
            }
            NeoPixelSetHumidityLevel(level);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
