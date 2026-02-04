#ifndef GLOBALS_H
#define GLOBALS_H

#define delay_time 10000

// include libraries
#include <Wire.h>
#include <WiFi.h>
#include <DHT20.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include "DHTesp.h"

///
#include "../src/connect/TaskWifi.h"
#include "../src/connect/TaskMQTT.h"
#include "../src/device/TaskDHT20.h"
#include "../src/device/TaskDHT11.h"
#include "../src/device/TaskNeoPixel.h"
#include "../src/device/TaskBlink.h"
#include "../src/device/TaskLCD.h"


extern SemaphoreHandle_t tempLowSem;
extern SemaphoreHandle_t tempMidSem;
extern SemaphoreHandle_t tempHighSem;

extern SemaphoreHandle_t humLowSem;
extern SemaphoreHandle_t humMidSem;
extern SemaphoreHandle_t humHighSem;

extern QueueHandle_t lcdQueue;


typedef struct{
    int temp;
    int humi;
} Sensordata;

#define MY_SCL 11
#define MY_SDA 12


#endif