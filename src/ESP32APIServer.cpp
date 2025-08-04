#include "ESP32APIServer.h"
#include "ESP32Config.h"

ESP32APIServer::ESP32APIServer(int port) 
    : server(port), distanceSensor(nullptr), servoController(nullptr), 
      camClient(nullptr), autoPhotoEnabled(true), lastAutoPhoto(0) {
}

bool ESP32APIServer::init(DistanceSensor* sensor, ServoController* servo, ESP32CAMClient* cam) {
    distanceSensor = sensor;
    servoController = servo;
    camClient = cam;
    
    // Connecter WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
        setupRoutes();
        return true;
    } else {
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

void ESP32APIServer::setupRoutes() {
    // Page d'accueil
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", generateWebInterface());
    });
    
    // API Status général
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["distance"] = distanceSensor->getLastDistance();
        doc["gate"] = servoController->isGateOpen();
        doc["auto_photo"] = autoPhotoEnabled;
        doc["esp32cam_ip"] = camClient->getIP();
        doc["esp32cam_reachable"] = camClient->isReachable();
        doc["free_heap"] = ESP.getFreeHeap();
        doc["uptime"] = millis();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API Distance
    server.on("/api/distance", HTTP_GET, [this](AsyncWebServerRequest *request) {
        float distance = distanceSensor->readDistance();
        
        JsonDocument doc;
        doc["distance"] = distance;
        doc["detected"] = distanceSensor->isObjectDetected(DETECTION_DISTANCE_CM);
        doc["threshold"] = DETECTION_DISTANCE_CM;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API Gate Status
    server.on("/api/gate", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["gate"] = servoController->isGateOpen();
        doc["position"] = servoController->getCurrentAngle();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API Gate Control
    server.on("/api/gate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("action")) {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing action parameter\"}");
            return;
        }
        
        String action = request->getParam("action")->value();
        bool success = false;
        
        if (action == "open" || action == "on") {
            success = servoController->openGate();
        } else if (action == "close" || action == "off") {
            success = servoController->closeGate();
        } else {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid action (use: open/close)\"}");
            return;
        }
        
        JsonDocument doc;
        doc["status"] = success ? "success" : "error";
        doc["gate"] = servoController->isGateOpen();
        doc["position"] = servoController->getCurrentAngle();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API Photo - Capture et retourne l'image
    server.on("/api/photo", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // Demander l'image directement depuis l'ESP32-CAM
        String imageData = camClient->requestPhotoData();
        
        if (imageData.length() > 0) {
            request->send(200, "image/jpeg", imageData);
        } else {
            JsonDocument doc;
            doc["status"] = "error";
            doc["message"] = "Photo capture failed";
            doc["esp32cam_ip"] = camClient->getIP();
            
            String response;
            serializeJson(doc, response);
            request->send(500, "application/json", response);
        }
    });
    
    // API Photo JSON - Juste capturer (sans retourner l'image)
    server.on("/api/photo/capture", HTTP_POST, [this](AsyncWebServerRequest *request) {
        bool success = camClient->requestPhoto();
        
        JsonDocument doc;
        doc["status"] = success ? "success" : "error";
        doc["message"] = success ? "Photo captured and saved to SD" : "Photo capture failed";
        doc["esp32cam_ip"] = camClient->getIP();
        
        String response;
        serializeJson(doc, response);
        request->send(success ? 200 : 500, "application/json", response);
    });
    
    // API Auto Photo Toggle
    server.on("/api/auto", HTTP_POST, [this](AsyncWebServerRequest *request) {
        autoPhotoEnabled = !autoPhotoEnabled;
        
        JsonDocument doc;
        doc["status"] = "success";
        doc["auto_photo"] = autoPhotoEnabled;
        doc["message"] = autoPhotoEnabled ? "Auto photo enabled" : "Auto photo disabled";
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API ESP32-CAM Status
    server.on("/api/esp32cam", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String camStatus = camClient->getStatus();
        request->send(200, "application/json", camStatus);
    });
    
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Endpoint not found\"}");
    });
}

String ESP32APIServer::generateWebInterface() {
    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "    <title>SmartGate Control API</title>\n";
    html += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "    <meta charset='UTF-8'>\n";
    html += "    <style>\n";
    html += "        body { font-family: Arial; margin: 20px; background: #f0f0f0; }\n";
    html += "        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n";
    html += "        h1 { color: #333; text-align: center; }\n";
    html += "        .status { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 10px 0; }\n";
    html += "        .controls { background: #f8f8ff; padding: 15px; border-radius: 5px; margin: 10px 0; }\n";
    html += "        button { background: #007bff; color: white; border: none; padding: 10px 20px; margin: 5px; border-radius: 5px; cursor: pointer; }\n";
    html += "        button:hover { background: #0056b3; }\n";
    html += "        button.danger { background: #dc3545; }\n";
    html += "        button.danger:hover { background: #c82333; }\n";
    html += "        .result { background: #fff3cd; padding: 10px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #ffc107; }\n";
    html += "        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }\n";
    html += "        @media (max-width: 600px) { .grid { grid-template-columns: 1fr; } }\n";
    html += "    </style>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "    <div class=\"container\">\n";
    html += "        <h1>🚪 SmartGate Control API</h1>\n";
    html += "        <div class=\"status\">\n";
    html += "            <h2>📊 System Status</h2>\n";
    html += "            <p><strong>Distance:</strong> <span id=\"distance\">--</span> cm</p>\n";
    html += "            <p><strong>Gate:</strong> <span id=\"gate\">--</span></p>\n";
    html += "            <p><strong>Auto Photo:</strong> <span id=\"auto\">--</span></p>\n";
    html += "            <p><strong>ESP32-CAM:</strong> <span id=\"esp32cam\">--</span></p>\n";
    html += "            <p><strong>Free Memory:</strong> <span id=\"memory\">--</span> bytes</p>\n";
    html += "        </div>\n";
    html += "        <div class=\"controls\">\n";
    html += "            <h2>🎮 Controls</h2>\n";
    html += "            <div class=\"grid\">\n";
    html += "                <div>\n";
    html += "                    <h3>Gate Control</h3>\n";
    html += "                    <button onclick=\"openGate()\">🔓 Open Gate</button>\n";
    html += "                    <button onclick=\"closeGate()\" class=\"danger\">🔒 Close Gate</button>\n";
    html += "                </div>\n";
    html += "                <div>\n";
    html += "                    <h3>Photo Control</h3>\n";
    html += "                    <button onclick=\"takePhoto()\">📸 Capture Photo</button>\n";
    html += "                    <button onclick=\"downloadPhoto()\">📥 Download Photo</button>\n";
    html += "                    <button onclick=\"toggleAuto()\">🔄 Toggle Auto</button>\n";
    html += "                </div>\n";
    html += "            </div>\n";
    html += "            <div style=\"text-align: center; margin-top: 15px;\">\n";
    html += "                <button onclick=\"refresh()\">🔄 Refresh Status</button>\n";
    html += "            </div>\n";
    html += "        </div>\n";
    html += "        <div id=\"result\" class=\"result\" style=\"display: none;\"></div>\n";
    html += "    </div>\n";
    html += "    <script>\n";
    html += "        function refresh() {\n";
    html += "            fetch('/api/status')\n";
    html += "                .then(r => r.json())\n";
    html += "                .then(d => {\n";
    html += "                    document.getElementById('distance').textContent = d.distance.toFixed(1);\n";
    html += "                    document.getElementById('gate').textContent = d.gate ? 'OPEN' : 'CLOSED';\n";
    html += "                    document.getElementById('auto').textContent = d.auto_photo ? 'ON' : 'OFF';\n";
    html += "                    document.getElementById('esp32cam').textContent = d.esp32cam_reachable ? 'Connected' : 'Disconnected';\n";
    html += "                    document.getElementById('memory').textContent = d.free_heap.toLocaleString();\n";
    html += "                })\n";
    html += "                .catch(e => showResult('Error: ' + e.message));\n";
    html += "        }\n";
    html += "        function showResult(message) {\n";
    html += "            const result = document.getElementById('result');\n";
    html += "            result.innerHTML = typeof message === 'object' ? JSON.stringify(message, null, 2) : message;\n";
    html += "            result.style.display = 'block';\n";
    html += "            setTimeout(() => result.style.display = 'none', 5000);\n";
    html += "        }\n";
    html += "        function openGate() {\n";
    html += "            fetch('/api/gate?action=open', {method: 'POST'})\n";
    html += "                .then(r => r.json())\n";
    html += "                .then(d => { showResult(d); refresh(); });\n";
    html += "        }\n";
    html += "        function closeGate() {\n";
    html += "            fetch('/api/gate?action=close', {method: 'POST'})\n";
    html += "                .then(r => r.json())\n";
    html += "                .then(d => { showResult(d); refresh(); });\n";
    html += "        }\n";
    html += "        function takePhoto() {\n";
    html += "            fetch('/api/photo/capture', {method: 'POST'})\n";
    html += "                .then(r => r.json())\n";
    html += "                .then(d => showResult(d));\n";
    html += "        }\n";
    html += "        function downloadPhoto() {\n";
    html += "            fetch('/api/photo', {method: 'POST'})\n";
    html += "                .then(response => {\n";
    html += "                    if (response.ok) {\n";
    html += "                        return response.blob();\n";
    html += "                    }\n";
    html += "                    throw new Error('Photo capture failed');\n";
    html += "                })\n";
    html += "                .then(blob => {\n";
    html += "                    const url = window.URL.createObjectURL(blob);\n";
    html += "                    const a = document.createElement('a');\n";
    html += "                    a.href = url;\n";
    html += "                    a.download = 'smartgate_photo_' + new Date().getTime() + '.jpg';\n";
    html += "                    document.body.appendChild(a);\n";
    html += "                    a.click();\n";
    html += "                    window.URL.revokeObjectURL(url);\n";
    html += "                    document.body.removeChild(a);\n";
    html += "                    showResult('Photo downloaded successfully');\n";
    html += "                })\n";
    html += "                .catch(e => showResult('Error: ' + e.message));\n";
    html += "        }\n";
    html += "        function toggleAuto() {\n";
    html += "            fetch('/api/auto', {method: 'POST'})\n";
    html += "                .then(r => r.json())\n";
    html += "                .then(d => { showResult(d); refresh(); });\n";
    html += "        }\n";
    html += "        setInterval(refresh, 3000);\n";
    html += "        refresh();\n";
    html += "    </script>\n";
    html += "</body>\n";
    html += "</html>\n";
    return html;
}


void ESP32APIServer::begin() {
    server.begin();
    Serial.println("=== API Server Ready ===");
    Serial.printf("Access: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("Endpoints:");
    Serial.println("  GET  /              - Web interface");
    Serial.println("  GET  /api/status    - System status");
    Serial.println("  GET  /api/distance  - Distance sensor");
    Serial.println("  GET  /api/gate      - Gate status");
    Serial.println("  POST /api/gate      - Gate control");
    Serial.println("  POST /api/photo     - Take photo (returns image)");
    Serial.println("  POST /api/photo/capture - Capture only (returns JSON)");
    Serial.println("  POST /api/auto      - Toggle auto photo");
    Serial.println("  GET  /api/esp32cam  - ESP32-CAM status");
}

String ESP32APIServer::getIPAddress() {
    return WiFi.localIP().toString();
}

bool ESP32APIServer::isAutoPhotoEnabled() const {
    return autoPhotoEnabled;
}

void ESP32APIServer::setAutoPhoto(bool enabled) {
    autoPhotoEnabled = enabled;
}

void ESP32APIServer::handleAutoPhoto() {
    if (!autoPhotoEnabled || !distanceSensor || !camClient) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    if (distanceSensor->isObjectDetected(DETECTION_DISTANCE_CM) && 
        currentTime - lastAutoPhoto >= AUTO_PHOTO_INTERVAL_MS) {
        
        Serial.println("Object detected! Requesting auto photo...");
        
        if (camClient->requestPhoto()) {
            Serial.println("Auto photo requested successfully");
        } else {
            Serial.println("Auto photo request failed");
        }
        
        lastAutoPhoto = currentTime;
    }
}
