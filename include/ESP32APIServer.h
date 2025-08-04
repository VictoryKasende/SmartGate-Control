#ifndef ESP32_API_SERVER_H
#define ESP32_API_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "DistanceSensor.h"
#include "ServoController.h"
#include "ESP32CAMClient.h"

class ESP32APIServer {
private:
    AsyncWebServer server;
    DistanceSensor* distanceSensor;
    ServoController* servoController;
    ESP32CAMClient* camClient;
    bool autoPhotoEnabled;
    unsigned long lastAutoPhoto;
    
    void setupRoutes();
    String generateWebInterface();
    
public:
    ESP32APIServer(int port = 80);
    bool init(DistanceSensor* sensor, ServoController* servo, ESP32CAMClient* cam);
    void begin();
    String getIPAddress();
    bool isAutoPhotoEnabled() const;
    void setAutoPhoto(bool enabled);
    void handleAutoPhoto();
};

#endif
