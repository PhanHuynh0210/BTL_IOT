#include "TaskCOREIOT.h"

Preferences mqttPrefs;
const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "V7owrjT8WmrHOt1tPS2f";   
const int   mqttPort = 1883;

char mqttToken[64];

void loadMqttToken()
{
    mqttPrefs.begin("mqtt", true);
    String token = mqttPrefs.getString("token", "");
    mqttPrefs.end();

    if (token.length() > 0) {
        token.toCharArray(mqttToken, sizeof(mqttToken));
        Serial.println(mqttToken);
    } else {
        strcpy(mqttToken, coreIOT_Token);
    }
}


int mqttFailCount = 0;
const int MQTT_MAX_FAIL = 5;
bool mqttReconnect()
{
    if (client.connected())
        return true;

    Serial.print("[MQTT] Connecting...");
    if (client.connect("ESP32Client", mqttToken, NULL))
    {
        Serial.println("OK");
        mqttFailCount = 0;
        client.subscribe("v1/devices/me/rpc/request/+");
        client.subscribe("v1/devices/me/attributes");
        return true;
    }

    mqttFailCount++;
    Serial.print("Fail, rc=");
    Serial.println(client.state());
    return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("\n========== CÓ TÍN HIỆU MQTT ĐẾN ==========");
    Serial.print("[MQTT] Topic: ");
    Serial.println(topic);

    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.print("[MQTT] Payload: ");
    Serial.println(message);
    Serial.println("=============================================\n");

    String topicStr = String(topic);
    String reqId = "";
    int reqIndex = topicStr.lastIndexOf('/');
    if (reqIndex != -1) {
        reqId = topicStr.substring(reqIndex + 1);
    }

    OTA_SYS ota;

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    if (doc.containsKey("method")) {
        const char* method = doc["method"];

        if (strcmp(method, "checkFirmware") == 0) {
            String responseTopic = "v1/devices/me/rpc/response/" + reqId;
            client.publish(responseTopic.c_str(), "{\"status\":\"app_is_checking_github\"}");
            return;
        }

        if (strcmp(method, "updateFirmware") == 0) {
            String responseTopic = "v1/devices/me/rpc/response/" + reqId;
            client.publish(responseTopic.c_str(), "{\"status\":\"updating_started\"}");

            if (doc["params"].containsKey("version")) {
                const char* v = doc["params"]["version"];
                VersionNew = String(v);
                
                Serial.print("[OTA] Nhận lệnh cài đặt phiên bản: ");
                Serial.println(VersionNew);

                ota = OTA_UPDATE;
                xQueueSend(otaQueue, &ota, 0);
            } else {
                Serial.println("[OTA] Lỗi: App không gửi kèm thông tin version!");
            }
            return;
        }
    }

    if (doc.containsKey("updatemqtt")) {
        const char* token = doc["updatemqtt"];
        Serial.println(token);
        mqttPrefs.begin("mqtt", false);   
        mqttPrefs.putString("token", token);
        mqttPrefs.end();
        xSemaphoreGive(mqttUpdateSem);
    }
}



void setup_coreiot()
{
    xSemaphoreTake(CoreIOTSem, portMAX_DELAY);
    client.setServer(coreIOT_Server, mqttPort);
    client.setCallback(callback);
}
void coreiot_task(void *pvParameters){

    setup_coreiot();
    loadMqttToken();

    Sensordata data;
    bool checkwifi = false;

    while(true){
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);

        if (checkwifi && !wifiConnected) {
            system_event evt = EVT_WIFI_FAIL;
            xQueueSend(stateQueue, &evt, 0);
        }
        checkwifi = wifiConnected;

        if (xSemaphoreTake(mqttUpdateSem, 0) == pdTRUE) {
            Serial.println("[MQTT] Token updated, reconnecting...");
            loadMqttToken();         
            client.disconnect();
            vTaskDelay(pdMS_TO_TICKS(300)); 
        }

        if (!wifiConnected) {
            xSemaphoreTake(CoreIOTSem, portMAX_DELAY);
        }

        if (!client.connected()) {
            if (!mqttReconnect()) {

                if (mqttFailCount >= 5) {

                    mqttPrefs.begin("mqtt", false);
                    mqttPrefs.remove("token");
                    mqttPrefs.end();

                    strcpy(mqttToken, coreIOT_Token);

                    mqttFailCount = 0;
                    client.disconnect();
                    vTaskDelay(pdMS_TO_TICKS(500));
                }   
            }
        }
        client.loop(); 

        if(xQueueReceive(coreIOTQueue, &data, pdMS_TO_TICKS(1000))){
            if (WiFi.status() != WL_CONNECTED){
                xSemaphoreTake(CoreIOTSem, portMAX_DELAY);
            }

            if (!client.connected()) {
                // Sẽ thử reconnect ở vòng lặp kế tiếp.
                continue;
            }

            // sensorId = 0 hoặc > MAX_SENSORS: coi là local/unknown, publish theo API single-device.
            if (data.sensorId == 0 || data.sensorId > MAX_SENSORS) {
                StaticJsonDocument<128> doc;
                doc["temperature"] = data.temp;
                doc["humidity"]    = data.humi;

                char buffer[128];
                size_t len = serializeJson(doc, buffer);
                client.publish("v1/devices/me/telemetry", buffer, len);
            } else {
                // Gateway mode: mỗi sensorId là 1 device con trên CoreIoT.
                // Cache trạng thái "đã connect" để chỉ gửi v1/gateway/connect 1 lần/device.
                static bool connectedSensor[MAX_SENSORS + 1] = {false};

                const char* devName = sensorName(data.sensorId);

                if (!connectedSensor[data.sensorId]) {
                    StaticJsonDocument<64> c;
                    c["device"] = devName;
                    char cbuf[96];
                    size_t cn = serializeJson(c, cbuf);
                    if (client.publish("v1/gateway/connect", cbuf, cn)) {
                        connectedSensor[data.sensorId] = true;
                        Serial.printf("[GW] connect %s\n", devName);
                    } else {
                        Serial.printf("[GW] connect %s FAIL\n", devName);
                    }
                }

                // Payload format phẳng cho CoreIoT Gateway:
                // { "<devName>": [ { "ts": <ms>, "temperature": .., "humidity": .. } ] }
                // CoreIoT không unwrap "values" nên phải đặt key ngang hàng với "ts".
                StaticJsonDocument<192> doc;
                JsonArray arr = doc.createNestedArray(devName);
                JsonObject item = arr.createNestedObject();

                // Lấy epoch ms nếu đã sync NTP (TaskGGsheet gọi configTime).
                time_t now = time(nullptr);
                if (now > 1700000000) { // ~2023-11, coi như đã có time hợp lệ
                    item["ts"] = (uint64_t)now * 1000ULL;
                }
                item["temperature"] = data.temp;
                item["humidity"]    = data.humi;

                char buffer[192];
                size_t len = serializeJson(doc, buffer);
                if (!client.publish("v1/gateway/telemetry", buffer, len)) {
                    Serial.printf("[GW] telemetry %s FAIL\n", devName);
                    // Nếu publish fail, reset trạng thái để lần sau connect lại.
                    connectedSensor[data.sensorId] = false;
                } else {
                    Serial.printf("[GW] telemetry %s OK: %s\n", devName, buffer);
                }
            }
        }
    }

}