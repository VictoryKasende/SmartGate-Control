#include "ESP32CAMClient.h"

ESP32CAMClient::ESP32CAMClient(const String& ip) 
    : esp32camIP(ip), lastPhotoRequest(0) {
}

bool ESP32CAMClient::init() {
    Serial.printf("ESP32-CAM Client initialized for IP: %s\n", esp32camIP.c_str());
    return true;
}

bool ESP32CAMClient::requestPhoto() {
    unsigned long currentTime = millis();
    
    // Éviter les requêtes trop rapprochées
    if (currentTime - lastPhotoRequest < 2000) {
        Serial.println("Photo request too soon, skipping...");
        return false;
    }
    
    httpClient.begin("http://" + esp32camIP + "/capture");
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.setTimeout(10000); // 10 secondes timeout
    
    int httpResponseCode = httpClient.POST("");
    lastPhotoRequest = currentTime;
    
    if (httpResponseCode == 200) {
        String response = httpClient.getString();
        Serial.println("Photo capture response: " + response);
        httpClient.end();
        return true;
    } else {
        Serial.printf("Photo capture failed: HTTP %d\n", httpResponseCode);
        httpClient.end();
        return false;
    }
}

String ESP32CAMClient::getStatus() {
    httpClient.begin("http://" + esp32camIP + "/status");
    httpClient.setTimeout(5000); // 5 secondes timeout
    
    int httpResponseCode = httpClient.GET();
    
    if (httpResponseCode == 200) {
        String response = httpClient.getString();
        httpClient.end();
        return response;
    } else {
        httpClient.end();
        return "{\"error\":\"ESP32-CAM not reachable\",\"code\":" + String(httpResponseCode) + "}";
    }
}

bool ESP32CAMClient::isReachable() {
    httpClient.begin("http://" + esp32camIP + "/status");
    httpClient.setTimeout(3000); // 3 secondes timeout court
    
    int httpResponseCode = httpClient.GET();
    httpClient.end();
    
    return (httpResponseCode == 200);
}

void ESP32CAMClient::setIP(const String& ip) {
    esp32camIP = ip;
    Serial.printf("ESP32-CAM IP updated to: %s\n", ip.c_str());
}

String ESP32CAMClient::getIP() const {
    return esp32camIP;
}

String ESP32CAMClient::requestPhotoData() {
    unsigned long currentTime = millis();
    
    // Éviter les requêtes trop rapprochées
    if (currentTime - lastPhotoRequest < 2000) {
        Serial.println("Photo request too soon, skipping...");
        return "";
    }
    
    httpClient.begin("http://" + esp32camIP + "/stream");
    httpClient.setTimeout(15000); // 15 secondes timeout pour image
    
    int httpResponseCode = httpClient.GET();
    lastPhotoRequest = currentTime;
    
    if (httpResponseCode == 200) {
        String imageData = httpClient.getString();
        Serial.printf("Image data received: %d bytes\n", imageData.length());
        httpClient.end();
        return imageData;
    } else {
        Serial.printf("Image request failed: HTTP %d\n", httpResponseCode);
        httpClient.end();
        return "";
    }
}
