#include "TaskDHT20.h"

#define LED_PIN 48


DHT20 dht20;


void TaskDHT20(void *pvParameters)
{
   Sensordata data;
    
    tempLowSem  = xSemaphoreCreateBinary();
    tempMidSem  = xSemaphoreCreateBinary();
    tempHighSem = xSemaphoreCreateBinary();

    humLowSem  = xSemaphoreCreateBinary();
    humMidSem  = xSemaphoreCreateBinary();
    humHighSem = xSemaphoreCreateBinary();

    lcdQueue = xQueueCreate(1, sizeof(Sensordata));

    while (true)
    {
        if (dht20.read() == DHT20_OK)
        {
            data.temp = dht20.getTemperature();
            data.humi = dht20.getHumidity();

            Serial.print("DHT20 OK: ");
            Serial.print(data.temp);
            Serial.print("C ");
            Serial.print(data.humi);
            Serial.println("%");

            if (data.temp < 30){
                xSemaphoreGive(tempLowSem);
            }
            else if (data.temp < 35){
                xSemaphoreGive(tempMidSem);
            }
            else{
                xSemaphoreGive(tempHighSem);
            }

            // task2
            if (data.humi < 30){
                xSemaphoreGive(humLowSem);
            }
            else if (data.humi < 35){
                xSemaphoreGive(humMidSem);
            }
            else{
                xSemaphoreGive(humHighSem);
            }

            // Task 3
            xQueueOverwrite(lcdQueue, &data);
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); 
    }
}



