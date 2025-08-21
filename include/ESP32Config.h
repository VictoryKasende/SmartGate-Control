#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

// Configuration WiFi
#define WIFI_SSID "WINS"
#define WIFI_PASSWORD "WINNER20"
#define WEB_SERVER_PORT 80

// Configuration ESP32-CAM
#define ESP32CAM_IP "10.244.250.144"

// Configuration matérielle ESP32
#define SERVO_PIN 13
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_PIN 12

// Configuration système
#define DETECTION_DISTANCE_CM 20
#define UPDATE_INTERVAL_MS 2000  // Augmenter à 2000ms pour réduire la charge
#define AUTO_PHOTO_INTERVAL_MS 5000  // Disabled in code to prevent failures

// Configuration debug
#define DEBUG_WATCHDOG true
#define DEBUG_MEMORY true
#define DEBUG_RESET_REASON true
#define MEMORY_WARNING_THRESHOLD 50000  // Alerter si heap < 50KB

#endif
