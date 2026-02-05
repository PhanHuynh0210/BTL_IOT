#include "TaskCOREIOT.h"

Preferences mqttPrefs;
const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "gtf8cv81mqtv4zv799mj";   
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

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Parse JSON
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
            xSemaphoreGive(otaSem);
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
    Serial.println("[CoreIOT] Waiting for WiFi...");
    xSemaphoreTake(CoreIOTSem, portMAX_DELAY);

    Serial.println("[CoreIOT] WiFi ready!");

    client.setServer(coreIOT_Server, mqttPort);
    client.setCallback(callback);
}
void coreiot_task(void *pvParameters){

    setup_coreiot();
    loadMqttToken();

    Sensordata data;

    while(true){

        if (xSemaphoreTake(mqttUpdateSem, 0) == pdTRUE) {
            Serial.println("[MQTT] Token updated, reconnecting...");
            loadMqttToken();         
            client.disconnect();
            vTaskDelay(pdMS_TO_TICKS(300)); 
        }

        if (WiFi.status() != WL_CONNECTED) {
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

        if(xQueueReceive(coreIOTQueue, &data, portMAX_DELAY)){
            if (WiFi.status() != WL_CONNECTED){
                xSemaphoreTake(CoreIOTSem, portMAX_DELAY);
            }


            StaticJsonDocument<128> doc;
            doc["temperature"] = data.temp;
            doc["humidity"]    = data.humi;

            char buffer[128];
            serializeJson(doc, buffer);

            client.publish("v1/devices/me/telemetry", buffer);

            Serial.print("[CoreIOT] Published: ");
            Serial.println(buffer);
            
        }
    }

}