#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

// Configuration WiFi
#define WIFI_SSID "WINS"
#define WIFI_PASSWORD "WINNER20"
#define WEB_SERVER_PORT 80

// Configuration ESP32-CAM
#define ESP32CAM_IP "10.253.254.144"

// Configuration matérielle ESP32
#define SERVO_PIN 18
#define TRIG_PIN 2
#define ECHO_PIN 4
#define LED_PIN 2

// Configuration système
#define DETECTION_DISTANCE_CM 20
#define UPDATE_INTERVAL_MS 1000
#define AUTO_PHOTO_INTERVAL_MS 5000

#endif
