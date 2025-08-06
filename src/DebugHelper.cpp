#include "DebugHelper.h"
#include "ESP32Config.h"

unsigned long DebugHelper::lastHeapCheck = 0;
uint32_t DebugHelper::minHeapSeen = UINT32_MAX;
uint32_t DebugHelper::bootCount = 0;

void DebugHelper::init() {
    bootCount++;
    Serial.begin(115200);
    delay(2000); // Attendre que le Serial soit prêt
    
    Serial.println("\n==================================================");
    Serial.printf("🔄 BOOT #%d - DEBUG MODE ENABLED\n", bootCount);
    Serial.println("==================================================");
    
    printResetReason();
    printSystemInfo();
    
    // Configurer le watchdog avec un délai plus long
    esp_task_wdt_init(30, true); // 30 secondes timeout
    esp_task_wdt_add(NULL);
    
    Serial.println("✅ Debug Helper initialized");
}

void DebugHelper::printResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.printf("🔍 Reset Reason: %s\n", getResetReasonString(reason).c_str());
    
    // Informations détaillées selon la cause
    switch(reason) {
        case ESP_RST_POWERON:
            Serial.println("   → Normal power-on reset");
            break;
        case ESP_RST_EXT:
            Serial.println("   → External reset (reset button)");
            break;
        case ESP_RST_SW:
            Serial.println("   → Software reset (restart())");
            break;
        case ESP_RST_PANIC:
            Serial.println("   ⚠️  PANIC RESET - Code crashed!");
            break;
        case ESP_RST_INT_WDT:
            Serial.println("   ⚠️  WATCHDOG TIMEOUT - Main task blocked!");
            break;
        case ESP_RST_TASK_WDT:
            Serial.println("   ⚠️  TASK WATCHDOG - Specific task blocked!");
            break;
        case ESP_RST_WDT:
            Serial.println("   ⚠️  HARDWARE WATCHDOG - System hung!");
            break;
        case ESP_RST_DEEPSLEEP:
            Serial.println("   → Wake up from deep sleep");
            break;
        case ESP_RST_BROWNOUT:
            Serial.println("   ⚠️  BROWNOUT - Power supply issue!");
            break;
        default:
            Serial.println("   → Unknown reset reason");
            break;
    }
}

String DebugHelper::getResetReasonString(esp_reset_reason_t reason) {
    switch(reason) {
        case ESP_RST_POWERON: return "POWERON";
        case ESP_RST_EXT: return "EXTERNAL";
        case ESP_RST_SW: return "SOFTWARE";
        case ESP_RST_PANIC: return "PANIC";
        case ESP_RST_INT_WDT: return "INT_WATCHDOG";
        case ESP_RST_TASK_WDT: return "TASK_WATCHDOG";
        case ESP_RST_WDT: return "HW_WATCHDOG";
        case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
        case ESP_RST_BROWNOUT: return "BROWNOUT";
        default: return "UNKNOWN";
    }
}

void DebugHelper::printSystemInfo() {
    Serial.printf("📊 Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("📊 Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("📊 CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("📊 Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("📊 Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("📊 Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("📊 PSRAM Size: %d bytes\n", ESP.getPsramSize());
    Serial.printf("📊 SDK Version: %s\n", ESP.getSdkVersion());
    
    printStackHighWaterMark();
}

void DebugHelper::checkMemory() {
    unsigned long currentTime = millis();
    if (currentTime - lastHeapCheck < 5000) return; // Check every 5 seconds
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    
    if (freeHeap < minHeapSeen) {
        minHeapSeen = freeHeap;
    }
    
    if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        Serial.printf("⚠️  LOW MEMORY WARNING: %d bytes free (min seen: %d)\n", 
                     freeHeap, minFreeHeap);
    }
    
    if (DEBUG_MEMORY && (currentTime - lastHeapCheck >= 10000)) { // Print every 10s
        Serial.printf("💾 Memory: Free=%d, Min=%d, MinSeen=%d\n", 
                     freeHeap, minFreeHeap, minHeapSeen);
    }
    
    lastHeapCheck = currentTime;
}

void DebugHelper::feedWatchdog() {
    if (DEBUG_WATCHDOG) {
        esp_task_wdt_reset();
    }
}

void DebugHelper::logCriticalOperation(const char* operation) {
    Serial.printf("🔧 Critical Operation: %s (Heap: %d)\n", operation, ESP.getFreeHeap());
    feedWatchdog();
}

void DebugHelper::printStackHighWaterMark() {
    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("📊 Stack High Water Mark: %d bytes\n", stackHighWaterMark);
    
    if (stackHighWaterMark < 1024) {
        Serial.println("⚠️  WARNING: Stack usage is high!");
    }
}
