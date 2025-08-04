#ifndef ESP32CAM_CLIENT_H
#define ESP32CAM_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class ESP32CAMClient {
private:
    String esp32camIP;
    HTTPClient httpClient;
    unsigned long lastPhotoRequest;
    
public:
    ESP32CAMClient(const String& ip);
    bool init();
    bool requestPhoto();
    String requestPhotoData(); // Nouvelle méthode pour récupérer l'image
    String getStatus();
    bool isReachable();
    void setIP(const String& ip);
    String getIP() const;
};

#endif
