#include "TaskDHT11.h"

#define DHT11_PIN 21
#define DHT11_LED_PIN 48

DHT11 dht11(DHT11_PIN);

void TaskDHT11(void *pvParameters)
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

        if (dht11.readTemperatureHumidity(data.temp, data.humi))
        {

            // Task 1
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
