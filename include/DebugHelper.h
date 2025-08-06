#ifndef DEBUG_HELPER_H
#define DEBUG_HELPER_H

#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

class DebugHelper {
private:
    static unsigned long lastHeapCheck;
    static uint32_t minHeapSeen;
    static uint32_t bootCount;
    
public:
    static void init();
    static void printResetReason();
    static void checkMemory();
    static void printSystemInfo();
    static void feedWatchdog();
    static void logCriticalOperation(const char* operation);
    static String getResetReasonString(esp_reset_reason_t reason);
    static void printStackHighWaterMark();
};

#endif
