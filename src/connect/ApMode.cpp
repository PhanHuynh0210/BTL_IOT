#include "ApMode.h"

#define BOOT_PIN 0

#define AP_SSID "ESP32_Config"
#define AP_PASS "12345678"
#define AP_TIMEOUT_MS (15 * 60 * 1000)

Preferences wifiPrefs;
WiFiServer apServer(80);
TimerHandle_t apTimeoutTimer;
bool apRunning = false;

void saveWiFi(const String &ssid, const String &pass) {
    wifiPrefs.begin("wifi", false);
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("pass", pass);
    wifiPrefs.end();
}

void loadWiFi(String &ssid, String &pass) {
    wifiPrefs.begin("wifi", true);
    ssid = wifiPrefs.getString("ssid", "");
    pass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();
}


void apTimeoutCallback(TimerHandle_t xTimer) {
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

    apTimeoutTimer = xTimerCreate("apTimeout", pdMS_TO_TICKS(AP_TIMEOUT_MS),pdFALSE,NULL,apTimeoutCallback);
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

void handleAPClient(WiFiClient &client) {
  String req = "";
  unsigned long timeout = millis() + 1000;
  while (client.connected() && millis() < timeout) {
    while (client.available()) {
      char c = client.read();
      req += c;
    }
  }
  

  if (req.indexOf("GET /scan") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    int n = WiFi.scanNetworks();
    client.print("[");
    for (int i = 0; i < n; i++) {
      if (i) client.print(",");
      client.printf("{\"ssid\":\"%s\",\"rssi\":%d}", 
                    WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
    client.print("]");
    client.stop();
    return;
  }

// ===== /wifi =====
if (req.indexOf("GET /wifi?ssid=") >= 0) {
  int s1 = req.indexOf("ssid=") + 5;
  int s2 = req.indexOf("&pass=");
  int s3 = req.indexOf("HTTP");
  String ssid = req.substring(s1, s2);
  String pass = req.substring(s2 + 6, s3 - 1);
  ssid.replace("%20", " ");
  
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Content-Length: 2\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print("OK");
  client.flush();
  delay(50);
  client.stop();
  
  saveWiFi(ssid, pass);
  system_event evt = EVT_WIFI_SAVED;
  xQueueSend(stateQueue, &evt, 0);
  
  return;
}

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 WiFi Setup</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    
    .container {
      background: white;
      border-radius: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      max-width: 500px;
      width: 100%;
      overflow: hidden;
    }
    
    .header {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 30px;
      text-align: center;
    }
    
    .header h1 {
      font-size: 28px;
      font-weight: 600;
      margin-bottom: 8px;
    }
    
    .header p {
      opacity: 0.9;
      font-size: 14px;
    }
    
    .content {
      padding: 30px;
    }
    
    .scan-section {
      margin-bottom: 25px;
    }
    
    .btn {
      width: 100%;
      padding: 14px 20px;
      border: none;
      border-radius: 10px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
    }
    
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    
    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
    }
    
    .btn-primary:active {
      transform: translateY(0);
    }
    
    .btn-secondary {
      background: #f0f0f0;
      color: #333;
      margin-top: 10px;
    }
    
    .btn-secondary:hover {
      background: #e0e0e0;
    }
    
    .btn:disabled {
      opacity: 0.6;
      cursor: not-allowed;
      transform: none !important;
    }
    
    .form-group {
      margin-bottom: 20px;
    }
    
    label {
      display: block;
      margin-bottom: 8px;
      color: #333;
      font-weight: 500;
      font-size: 14px;
    }
    
    input, select {
      width: 100%;
      padding: 12px 15px;
      border: 2px solid #e0e0e0;
      border-radius: 10px;
      font-size: 15px;
      transition: all 0.3s ease;
      background: #fafafa;
    }
    
    input:focus, select:focus {
      outline: none;
      border-color: #667eea;
      background: white;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    
    select {
      cursor: pointer;
      appearance: none;
      background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 12 12'%3E%3Cpath fill='%23333' d='M6 9L1 4h10z'/%3E%3C/svg%3E");
      background-repeat: no-repeat;
      background-position: right 15px center;
      padding-right: 40px;
    }
    
    .network-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 5px 0;
    }
    
    .signal-strength {
      font-size: 12px;
      color: #666;
    }
    
    .loading {
      display: none;
      text-align: center;
      color: #667eea;
      font-size: 14px;
      margin-top: 10px;
    }
    
    .loading.active {
      display: block;
    }
    
    .spinner {
      border: 3px solid #f3f3f3;
      border-top: 3px solid #667eea;
      border-radius: 50%;
      width: 30px;
      height: 30px;
      animation: spin 1s linear infinite;
      margin: 10px auto;
    }
    
    @keyframes spin {
      0% { transform: rotate(0deg); }
      100% { transform: rotate(360deg); }
    }
    
    .success-message {
      background: #d4edda;
      color: #155724;
      padding: 15px;
      border-radius: 10px;
      margin-top: 20px;
      display: none;
      border-left: 4px solid #28a745;
    }
    
    .success-message.active {
      display: block;
    }
    
    @media (max-width: 480px) {
      .header h1 {
        font-size: 24px;
      }
      
      .content {
        padding: 20px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>ESP32 WiFi Setup</h1>
      <p>Configure your wireless connection</p>
    </div>
    
    <div class="content">
      <div class="scan-section">
        <button class="btn btn-primary" onclick="scanWiFi()" id="scanBtn">
          <span></span>
          <span>Scan for Networks</span>
        </button>
        <div class="loading" id="loading">
          <div class="spinner"></div>
          <p>Scanning networks...</p>
        </div>
      </div>
      
      <form onsubmit="saveWiFi(event)">
        <div class="form-group">
          <label for="ssid">Network Name (SSID)</label>
          <select id="ssid" required>
            <option value="">Select a network...</option>
          </select>
        </div>
        
        <div class="form-group">
          <label for="pass">Password</label>
          <input type="password" id="pass" placeholder="Enter WiFi password" required>
        </div>
        
        <button type="submit" class="btn btn-primary" id="saveBtn">
          <span></span>
          <span>Save & Connect</span>
        </button>
      </form>
      
      <div class="success-message" id="successMsg">
        WiFi saved successfully!
      </div>
    </div>
  </div>

  <script>
    function scanWiFi() {
      document.getElementById('loading').classList.add('active');
      document.getElementById('scanBtn').disabled = true;
      
      fetch('/scan')
        .then(r => r.json())
        .then(data => {
          const select = document.getElementById('ssid');
          select.innerHTML = '<option value="">Select a network...</option>';
          
          data.forEach(net => {
            const option = document.createElement('option');
            option.value = net.ssid;
            option.textContent = `${net.ssid} (${net.rssi} dBm)`;
            select.appendChild(option);
          });
          
          document.getElementById('loading').classList.remove('active');
          document.getElementById('scanBtn').disabled = false;
        })
        .catch(() => {
          document.getElementById('loading').classList.remove('active');
          document.getElementById('scanBtn').disabled = false;
          alert('Scan failed. Please try again.');
        });
    }
    
    function saveWiFi(e) {
      e.preventDefault();
      const ssid = document.getElementById('ssid').value;
      const pass = document.getElementById('pass').value;
      
      if (!ssid) {
        alert('Please select a network');
        return;
      }
      
      document.getElementById('saveBtn').disabled = true;
      document.getElementById('saveBtn').innerHTML = '<span></span><span>Saving...</span>';
      
      fetch(`/wifi?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`)
        .then(r => r.text())
        .then(() => {
          document.getElementById('successMsg').classList.add('active');
          setTimeout(() => {
            location.reload();
          }, 2000);
        })
        .catch(() => {
          alert('Failed to save. Please try again.');
          document.getElementById('saveBtn').disabled = false;
          document.getElementById('saveBtn').innerHTML = '<span></span><span>Save & Connect</span>';
        });
    }
    
    // Auto-scan on load
    window.onload = () => scanWiFi();
  </script>
</body>
</html>
)rawliteral");
  
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
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}
