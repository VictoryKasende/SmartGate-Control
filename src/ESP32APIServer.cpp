#include "ESP32APIServer.h"
#include "ESP32Config.h"

ESP32APIServer::ESP32APIServer(int port) 
    : server(port), distanceSensor(nullptr), servoController(nullptr), 
      camClient(nullptr), autoPhotoEnabled(false), lastAutoPhoto(0) {
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
    // Interface ultra-minimaliste pour économiser la mémoire
    String html = "<html><head><title>SmartGate</title></head><body>";
    html += "<h1>SmartGate Control</h1>";
    html += "<p>Distance: <span id='d'>--</span> cm | Gate: <span id='g'>--</span></p>";
    html += "<button onclick=\"fetch('/api/gate?action=open',{method:'POST'})\">OPEN</button> ";
    html += "<button onclick=\"fetch('/api/gate?action=close',{method:'POST'})\">CLOSE</button> ";
    html += "<button onclick=\"fetch('/api/photo',{method:'POST'})\">PHOTO</button> ";
    html += "<button onclick=\"r()\">REFRESH</button>";
    html += "<script>function r(){fetch('/api/status').then(a=>a.json()).then(d=>{document.getElementById('d').innerHTML=d.distance.toFixed(1);document.getElementById('g').innerHTML=d.gate?'OPEN':'CLOSED'})}</script>";
    html += "</body></html>";
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
    // Auto photo detection disabled to prevent request failures
    // Uncomment to re-enable automatic photo capture
    /*
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
    */
}
