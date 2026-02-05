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
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h> 

///
#include "../src/connect/TaskWifi.h"
#include "../src/device/TaskDHT20.h"
#include "../src/device/TaskDHT11.h"
#include "../src/device/TaskNeoPixel.h"
#include "../src/device/TaskBlink.h"
#include "../src/device/TaskLCD.h"
#include "../src/connect/TaskCOREIOT.h"
#include "../src/device/TaskOTA.h"




extern SemaphoreHandle_t tempLowSem;
extern SemaphoreHandle_t tempMidSem;
extern SemaphoreHandle_t tempHighSem;

extern SemaphoreHandle_t humLowSem;
extern SemaphoreHandle_t humMidSem;
extern SemaphoreHandle_t humHighSem;

extern SemaphoreHandle_t CoreIOTSem;

extern SemaphoreHandle_t otaSem;


extern QueueHandle_t lcdQueue;
extern QueueHandle_t coreIOTQueue;

extern QueueHandle_t mqttUpdateSem;



extern PubSubClient client;


typedef struct{
    int temp;
    int humi;
} Sensordata;

#define MY_SCL 11
#define MY_SDA 12


#endif