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
#include "HTTPUpdate.h"
#include <TensorFlowLite_ESP32.h>

///
#include "../src/connect/TaskWifi.h"
#include "../src/device/TaskDHT20.h"
#include "../src/device/TaskDHT11.h"
#include "../src/device/TaskNeoPixel.h"
#include "../src/device/TaskBlink.h"
#include "../src/device/TaskLCD.h"
#include "../src/connect/TaskCOREIOT.h"
#include "../src/connect/ApMode.h"
#include "../src/device/TaskOTA.h"
#include "../src/system/StatusESP.h"
#include "../src/system/LedStatus.h"
#include "../src/device/TaskML.h"


#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"


extern SemaphoreHandle_t tempLowSem;
extern SemaphoreHandle_t tempMidSem;
extern SemaphoreHandle_t tempHighSem;

extern SemaphoreHandle_t humLowSem;
extern SemaphoreHandle_t humMidSem;
extern SemaphoreHandle_t humHighSem;

extern SemaphoreHandle_t CoreIOTSem;

extern QueueHandle_t otaQueue;


extern QueueHandle_t lcdQueue;
extern QueueHandle_t coreIOTQueue;
extern QueueHandle_t MLTinyQueue;


extern SemaphoreHandle_t mqttUpdateSem;

extern QueueHandle_t stateQueue;
extern QueueHandle_t ledQueue;

extern QueueHandle_t wifiQueue;


extern TimerHandle_t bootTimeoutTimer;
 

extern PubSubClient client;


typedef struct{
    int temp;
    int humi;
} Sensordata;

typedef enum {
  STATUS_BOOTING,
  STATUS_AP_MODE,
  STATUS_CONNECTING,
  STATUS_NORMAL,
  STATUS_ERROR,
  STATUS_OTA_UPDATE
} system_status;

typedef enum {
  OTA_CHECK,
  OTA_UPDATE
} OTA_SYS;

typedef enum {
  EVT_WIFI_START,
  EVT_WIFI_OK,
  EVT_WIFI_FAIL,

  
  EVT_WIFI_SAVED,
  EVT_BOOT_TIMEOUT,
  EVT_BOOT_BUTTON_LONG,
  EVT_MQTT_OK,
  EVT_MQTT_FAIL,
  EVT_OTA_START,
  EVT_OTA_DONE
} system_event;


#define MY_SCL 11
#define MY_SDA 12


#endif