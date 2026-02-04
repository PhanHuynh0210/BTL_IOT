#include "TaskBlink.h"

#define DHT11_LED_PIN 48

typedef enum {
    TEMP_LOW,
    TEMP_MID,
    TEMP_HIGH
} temp_state_t;

temp_state_t currentState = TEMP_MID;
TickType_t blinkDelay = pdMS_TO_TICKS(1000);

void TaskBlink(void *pvParameters){

    pinMode(DHT11_LED_PIN, OUTPUT);

    while (1){

        if (xSemaphoreTake(tempLowSem, 0)){
            currentState = TEMP_LOW;
            blinkDelay = pdMS_TO_TICKS(2000);
        }
        else if (xSemaphoreTake(tempMidSem, 0)){
            currentState = TEMP_MID;
            blinkDelay = pdMS_TO_TICKS(1000);
        }
        else if (xSemaphoreTake(tempHighSem, 0)){
            currentState = TEMP_HIGH;
            blinkDelay = pdMS_TO_TICKS(200);
        }

        digitalWrite(DHT11_LED_PIN, !digitalRead(DHT11_LED_PIN));
        vTaskDelay(blinkDelay);
    }
}
