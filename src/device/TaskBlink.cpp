#include "TaskBlink.h"

#define DHT11_LED_PIN 48

void TaskBlink(void *pvParameters){
    
    pinMode(DHT11_LED_PIN,OUTPUT);

    while (true){

       if(xSemaphoreTake(tempLowSem, 0)){
            vTaskDelay(pdMS_TO_TICKS(2000));
       }

        if(xSemaphoreTake(tempMidSem, 0)){
            vTaskDelay(pdMS_TO_TICKS(1000));
       }

        if(xSemaphoreTake(tempHighSem, 0)){
            vTaskDelay(pdMS_TO_TICKS(200));
       }

       digitalWrite(DHT11_LED_PIN,!digitalRead(DHT11_LED_PIN));
    }

}