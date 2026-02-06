#include "ApMode.h"

#define BOOT_PIN 0

#define AP_SSID "ESP32_Config"
#define AP_PASS "12345678"
#define AP_TIMEOUT_MS (15 * 60 * 1000)

Preferences wifiPrefs;

static void saveWiFi(const String &ssid, const String &pass) {
    wifiPrefs.begin("wifi", false);
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("pass", pass);
    wifiPrefs.end();
}

static void loadWiFi(String &ssid, String &pass) {
    wifiPrefs.begin("wifi", true);
    ssid = wifiPrefs.getString("ssid", "");
    pass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();
}

static WiFiServer apServer(80);
static TimerHandle_t apTimeoutTimer;
static bool apRunning = false;

static void apTimeoutCallback(TimerHandle_t xTimer) {
    system_event evt = EVT_BOOT_TIMEOUT;
    xQueueSendFromISR(stateQueue, &evt, NULL);
}

 
void taskBootButton(void *pvParameters){
    while(true){
        const TickType_t HOLD_TIME = pdMS_TO_TICKS(3000);

        static bool wasPressed = false;
        static TickType_t pressStart = 0;
        static bool eventSent = false;

        pinMode(BOOT_PIN, INPUT_PULLUP);

        if (digitalRead(BOOT_PIN) == LOW) {

            if (!wasPressed) {
                xTimerStop(bootTimeoutTimer, 0);
                wasPressed = true;
                pressStart = xTaskGetTickCount();
                eventSent = false;
            }

            if (!eventSent &&
                (xTaskGetTickCount() - pressStart >= HOLD_TIME)) {
                Serial.println("[BOOT] LONG PRESS -> EVT_BOOT_BUTTON_LONG");
                system_event evt = EVT_BOOT_BUTTON_LONG;
                xQueueSend(stateQueue, &evt, 0);

                eventSent = true; 
            }

        } else {
            if (wasPressed && !eventSent) {
                xTimerStart(bootTimeoutTimer, 0);
                Serial.println("[BOOT] Released → resume boot timer");
            }
            wasPressed = false;
            eventSent = false;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}


void initAP(void) {
    if (apRunning) return;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    apServer.begin();

    apTimeoutTimer = xTimerCreate(
        "apTimeout",
        pdMS_TO_TICKS(AP_TIMEOUT_MS),
        pdFALSE,
        NULL,
        apTimeoutCallback
    );
    xTimerStart(apTimeoutTimer, 0);

    apRunning = true;
}

void stopAP(void) {
    if (!apRunning) return;

    apServer.stop();
    WiFi.softAPdisconnect(true);
    xTimerStop(apTimeoutTimer, 0);

    apRunning = false;
}

static void handleAPClient(WiFiClient &client) {
    String req = client.readStringUntil('\r');

    // ===== Scan WiFi =====
    if (req.indexOf("GET /scan") >= 0) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json\r\n");

        int n = WiFi.scanNetworks();
        client.print("[");
        for (int i = 0; i < n; i++) {
            if (i) client.print(",");
            client.printf("{\"ssid\":\"%s\",\"rssi\":%d}",
                          WiFi.SSID(i).c_str(),
                          WiFi.RSSI(i));
        }
        client.print("]");
        client.stop();
        return;
    }

    // ===== Save WiFi =====
    if (req.indexOf("GET /wifi?ssid=") >= 0) {
        int s1 = req.indexOf("ssid=") + 5;
        int s2 = req.indexOf("&pass=");
        int s3 = req.indexOf("HTTP");

        String ssid = req.substring(s1, s2);
        String pass = req.substring(s2 + 6, s3 - 1);
        ssid.replace("%20", " ");

        saveWiFi(ssid, pass);   // hàm bạn đã có

        system_event evt = EVT_WIFI_SAVED;
        xQueueSend(stateQueue, &evt, 0);

        client.println("HTTP/1.1 200 OK\r\n\r\nSAVED");
        client.stop();
        return;
    }

    // ===== Default =====
    client.println("HTTP/1.1 200 OK\r\n\r\nESP32 WiFi Config");
    client.stop();
}
void taskAccessPoint(void *pvParameters) {
    WiFiClient client;

    while (true) {
        if (apRunning) {
            client = apServer.available();
            if (client) {
                handleAPClient(client);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // nhường CPU
    }
}
