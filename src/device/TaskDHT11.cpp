#include "TaskDHT11.h"

#define DHT11_PIN 10   

DHTesp dht;

void TaskDHT11(void *pvParameters)
{
    Sensordata data;
    dht.setup(DHT11_PIN, DHTesp::DHT11);

    while (true)
    {
        TempAndHumidity values = dht.getTempAndHumidity();

        if (dht.getStatus() == DHTesp::ERROR_NONE)
        {
            data.temp = values.temperature;
            data.humi = values.humidity;

            Serial.print("DHT11 OK: ");
            Serial.print(data.temp);
            Serial.print(" C | ");
            Serial.print(data.humi);
            Serial.println(" %");

            if (data.temp < 20)
                xSemaphoreGive(tempLowSem);
            else if (data.temp < 35)
                xSemaphoreGive(tempMidSem);
            else
                xSemaphoreGive(tempHighSem);

            if (data.humi < 30)
                xSemaphoreGive(humLowSem);
            else if (data.humi < 35)
                xSemaphoreGive(humMidSem);
            else
                xSemaphoreGive(humHighSem);

            xQueueOverwrite(lcdQueue, &data);
            xQueueOverwrite(coreIOTQueue, &data);
            xQueueOverwrite(MLTinyQueue, &data);
            xQueueOverwrite(GGSheetQueue, &data);

        }

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
