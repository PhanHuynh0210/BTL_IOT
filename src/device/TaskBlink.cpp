#include "TaskBlink.h"

#define DHT11_LED_PIN_1 21
#define DHT11_LED_PIN_2 18


TickType_t blinkDelay = pdMS_TO_TICKS(1000);

// void TaskBlink(void *pvParameters){

//     pinMode(DHT11_LED_PIN_1, OUTPUT);
//     pinMode(DHT11_LED_PIN_2, OUTPUT);


//     while (1){

//         if (xSemaphoreTake(tempLowSem, 0)){
//             blinkDelay = pdMS_TO_TICKS(2000);
//         }
//         else if (xSemaphoreTake(tempMidSem, 0)){
//             blinkDelay = pdMS_TO_TICKS(1000);
//         }
//         else if (xSemaphoreTake(tempHighSem, 0)){
//             blinkDelay = pdMS_TO_TICKS(200);
//         }

//         digitalWrite(DHT11_LED_PIN, !digitalRead(DHT11_LED_PIN));
//         vTaskDelay(blinkDelay);
//     }
// }


void TaskBlink(void *pvParameters){

    pinMode(DHT11_LED_PIN_1, OUTPUT);
    pinMode(DHT11_LED_PIN_2, OUTPUT);


    while (1){

        if (xSemaphoreTake(tempLowSem, 0)){

            digitalWrite(DHT11_LED_PIN_1, 1);
            digitalWrite(DHT11_LED_PIN_2, 0);
        }
        else if (xSemaphoreTake(tempMidSem, 0)){

            digitalWrite(DHT11_LED_PIN_1, 0);
            digitalWrite(DHT11_LED_PIN_2, 1);        
        }
        else if (xSemaphoreTake(tempHighSem, 0)){

            digitalWrite(DHT11_LED_PIN_1, 1);
            digitalWrite(DHT11_LED_PIN_2, 1);
        }
        vTaskDelay(blinkDelay);
    }
}