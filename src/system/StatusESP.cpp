#include "StatusESP.h"


void setStatus(system_status st) {
  xQueueSend(ledQueue, &st, portMAX_DELAY);
}

TimerHandle_t bootTimeoutTimer;

void bootTimeoutCallback(TimerHandle_t xTimer) {
    system_event evt = EVT_BOOT_TIMEOUT;
    xQueueSendFromISR(stateQueue, &evt, NULL);
}

void taskStateManager(void *pvParameters) {
  system_status current = STATUS_BOOTING;
  system_event evt;

  setStatus(STATUS_BOOTING);
  xTimerStart(bootTimeoutTimer, 0);


  while (1) {
    if (xQueueReceive(stateQueue, &evt, portMAX_DELAY)) {

      switch (current) {

        case STATUS_BOOTING:
          if (evt == EVT_BOOT_BUTTON_LONG) {
            current = STATUS_AP_MODE;
            xTimerStop(bootTimeoutTimer, 0);
            setStatus(current);
                            Serial.println("vao ap");

            initAP();
          }
          else if (evt == EVT_BOOT_TIMEOUT) {
            current = STATUS_CONNECTING;
            xTimerStop(bootTimeoutTimer, 0);
            setStatus(current);
            
            evt = EVT_WIFI_START;
            xQueueSend(wifiQueue, &evt, 0);
          }
          break;

        case STATUS_AP_MODE:
          if (evt == EVT_WIFI_SAVED) {   
            current = STATUS_CONNECTING;
            setStatus(current);
            stopAP();
            
            system_event evt = EVT_WIFI_START;
            xQueueSend(wifiQueue, &evt, 0);
          }
          break;

        case STATUS_CONNECTING:
          if (evt == EVT_WIFI_OK){
            current = STATUS_NORMAL;
            setStatus(current);
          }

          if (evt == EVT_WIFI_FAIL) {
            current = STATUS_ERROR;
            setStatus(current);
          }
          break;

        case STATUS_NORMAL:
          if (evt == EVT_OTA_START) {
            current = STATUS_OTA_UPDATE;
            setStatus(current);
          }
          break;

        case STATUS_ERROR:
          // current = STATUS_CONNECTING;
          // setStatus(current);
          break;

        case STATUS_OTA_UPDATE:
          if (evt == EVT_OTA_DONE) {
            ESP.restart();
          }
          break;

        default:
          break;
      }
    }
  }
}
