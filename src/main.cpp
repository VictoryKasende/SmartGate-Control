#include <Arduino.h>
#include "ESP32Config.h"
#include "DistanceSensor.h"
#include "ServoController.h"
#include "ESP32CAMClient.h"
#include "ESP32APIServer.h"
#include "DebugHelper.h"

// Global instances
DistanceSensor distanceSensor(TRIG_PIN, ECHO_PIN);
ServoController servoController(SERVO_PIN);
ESP32CAMClient esp32camClient(ESP32CAM_IP);
ESP32APIServer apiServer(WEB_SERVER_PORT);

// Timing variables
unsigned long lastUpdate = 0;

void setup() {
    // Initialiser le système de debug en premier
    DebugHelper::init();
    
    Serial.println("\n=== ESP32 SmartGate API Server Starting ===");
    
    // Initialize Distance Sensor
    DebugHelper::logCriticalOperation("Initializing Distance Sensor");
    if (!distanceSensor.init()) {
        Serial.println("❌ Failed to initialize Distance Sensor!");
        return;
    }
    Serial.println("✅ Distance Sensor initialized");
    DebugHelper::feedWatchdog();
    
    // Initialize Servo Controller
    DebugHelper::logCriticalOperation("Initializing Servo Controller");
    if (!servoController.init()) {
        Serial.println("❌ Failed to initialize Servo Controller!");
        return;
    }
    Serial.println("✅ Servo Controller initialized");
    DebugHelper::feedWatchdog();
    
    // Initialize ESP32-CAM Client
    DebugHelper::logCriticalOperation("Initializing ESP32-CAM Client");
    if (!esp32camClient.init()) {
        Serial.println("❌ Failed to initialize ESP32-CAM Client!");
        return;
    }
    Serial.println("✅ ESP32-CAM Client initialized");
    DebugHelper::feedWatchdog();
    
    // Initialize API Server (includes WiFi connection)
    DebugHelper::logCriticalOperation("Initializing API Server (WiFi + HTTP)");
    if (!apiServer.init(&distanceSensor, &servoController, &esp32camClient)) {
        Serial.println("❌ Failed to initialize API Server!");
        return;
    }
    DebugHelper::feedWatchdog();
    
    apiServer.begin();
    Serial.println("✅ API Server initialized");
    
    Serial.println("🎉 === System Ready ===");
    Serial.printf("🌐 Access the web interface at: http://%s\n", apiServer.getIPAddress().c_str());
    Serial.printf("📷 Communicating with ESP32-CAM at: %s\n", esp32camClient.getIP().c_str());
    
    DebugHelper::feedWatchdog();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Surveillance mémoire et watchdog en continu
    DebugHelper::checkMemory();
    DebugHelper::feedWatchdog();
    
    // Update distance sensor moins fréquemment
    DebugHelper::logCriticalOperation("Distance sensor update");
    distanceSensor.update();
    DebugHelper::feedWatchdog();
    
    // Auto photo detection disabled to prevent failures
    // apiServer.handleAutoPhoto();
    
    // Monitoring moins fréquent pour économiser CPU
    if (currentTime - lastUpdate >= UPDATE_INTERVAL_MS * 15) { // Every 30 seconds au lieu de 10
        DebugHelper::logCriticalOperation("Status monitoring");
        float distance = distanceSensor.getLastDistance();
        Serial.printf("📊 Status - Distance: %.1f cm | Gate: %s | Heap: %d\n",
                     distance,
                     servoController.isGateOpen() ? "OPEN" : "CLOSED",
                     ESP.getFreeHeap());
        
        lastUpdate = currentTime;
        DebugHelper::feedWatchdog();
    }
    
    // Délai avec watchdog feeding
    delay(200);
    DebugHelper::feedWatchdog();
}
