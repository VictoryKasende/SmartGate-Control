#include <Arduino.h>
#include "ESP32Config.h"
#include "DistanceSensor.h"
#include "ServoController.h"
#include "ESP32CAMClient.h"
#include "ESP32APIServer.h"

// Global instances
DistanceSensor distanceSensor(TRIG_PIN, ECHO_PIN);
ServoController servoController(SERVO_PIN);
ESP32CAMClient esp32camClient(ESP32CAM_IP);
ESP32APIServer apiServer(WEB_SERVER_PORT);

// Timing variables
unsigned long lastUpdate = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32 SmartGate API Server Starting ===");
    
    // Initialize Distance Sensor
    if (!distanceSensor.init()) {
        Serial.println("Failed to initialize Distance Sensor!");
        return;
    }
    Serial.println("Distance Sensor initialized");
    
    // Initialize Servo Controller
    if (!servoController.init()) {
        Serial.println("Failed to initialize Servo Controller!");
        return;
    }
    Serial.println("Servo Controller initialized");
    
    // Initialize ESP32-CAM Client
    if (!esp32camClient.init()) {
        Serial.println("Failed to initialize ESP32-CAM Client!");
        return;
    }
    Serial.println("ESP32-CAM Client initialized");
    
    // Initialize API Server (includes WiFi connection)
    if (!apiServer.init(&distanceSensor, &servoController, &esp32camClient)) {
        Serial.println("Failed to initialize API Server!");
        return;
    }
    apiServer.begin();
    Serial.println("API Server initialized");
    
    Serial.println("=== System Ready ===");
    Serial.printf("Access the web interface at: http://%s\n", apiServer.getIPAddress().c_str());
    Serial.printf("Communicating with ESP32-CAM at: %s\n", esp32camClient.getIP().c_str());
}

void loop() {
    unsigned long currentTime = millis();
    
    // Update distance sensor regularly
    distanceSensor.update();
    
    // Handle auto photo detection
    apiServer.handleAutoPhoto();
    
    // Periodic status monitoring
    if (currentTime - lastUpdate >= UPDATE_INTERVAL_MS * 10) { // Every 10 seconds
        float distance = distanceSensor.getLastDistance();
        Serial.printf("Status - Distance: %.1f cm | Gate: %s | Auto Photo: %s | Free Heap: %d\n",
                     distance,
                     servoController.isGateOpen() ? "OPEN" : "CLOSED",
                     apiServer.isAutoPhotoEnabled() ? "ON" : "OFF",
                     ESP.getFreeHeap());
        
        lastUpdate = currentTime;
    }
    
    delay(100); // Main loop delay
}
