#include "ESP32CAMClient.h"
#include "DebugHelper.h"

ESP32CAMClient::ESP32CAMClient(const String& ip) 
    : esp32camIP(ip), lastPhotoRequest(0) {
}

bool ESP32CAMClient::init() {
    Serial.printf("ESP32-CAM Client initialized for IP: %s\n", esp32camIP.c_str());
    return true;
}

bool ESP32CAMClient::requestPhoto() {
    DebugHelper::logCriticalOperation("ESP32CAM Photo Request START");
    unsigned long currentTime = millis();
    
    // Éviter les requêtes trop rapprochées
    if (currentTime - lastPhotoRequest < 3000) {
        Serial.println("⏭️  Photo request too soon, skipping...");
        return false;
    }
    
    DebugHelper::feedWatchdog();
    
    Serial.printf("📸 Requesting photo from %s...\n", esp32camIP.c_str());
    httpClient.begin("http://" + esp32camIP + "/capture");
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.setTimeout(3000); // Réduire timeout à 3 secondes
    
    DebugHelper::feedWatchdog();
    
    int httpResponseCode = httpClient.POST("");
    lastPhotoRequest = currentTime;
    
    DebugHelper::feedWatchdog();
    
    if (httpResponseCode == 200) {
        String response = httpClient.getString();
        Serial.println("✅ Photo OK");
        httpClient.end();
        DebugHelper::logCriticalOperation("ESP32CAM Photo Request SUCCESS");
        return true;
    } else {
        Serial.printf("❌ Photo failed: HTTP %d\n", httpResponseCode);
        httpClient.end();
        DebugHelper::logCriticalOperation("ESP32CAM Photo Request FAILED");
        return false;
    }
}

String ESP32CAMClient::getStatus() {
    httpClient.begin("http://" + esp32camIP + "/status");
    httpClient.setTimeout(2000); // Réduire timeout à 2 secondes
    
    int httpResponseCode = httpClient.GET();
    
    if (httpResponseCode == 200) {
        String response = httpClient.getString();
        httpClient.end();
        yield(); // Donner du temps au watchdog
        return response;
    } else {
        httpClient.end();
        yield(); // Donner du temps au watchdog
        return "{\"error\":\"CAM offline\"}";
    }
}

bool ESP32CAMClient::isReachable() {
    httpClient.begin("http://" + esp32camIP + "/status");
    httpClient.setTimeout(1500); // Timeout très court
    
    int httpResponseCode = httpClient.GET();
    httpClient.end();
    yield(); // Donner du temps au watchdog
    
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
    // FONCTION DÉSACTIVÉE - Cause des crashes mémoire
    // Les images de 50KB+ font crash l'ESP32 principal
    Serial.println("❌ requestPhotoData() disabled to prevent crashes");
    return "";
}
