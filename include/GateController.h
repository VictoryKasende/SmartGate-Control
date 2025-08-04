#ifndef GATE_CONTROLLER_H
#define GATE_CONTROLLER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class GateController {
private:
    Servo gateServo;
    bool servoAttached;
    int currentPosition;
    
    // Ultrasonic sensor
    unsigned long lastMeasurement;
    float lastDistance;
    
    float measureDistance();
    
public:
    GateController();
    bool init();
    void openGate();    // 90 degrees
    void closeGate();   // 0 degrees
    void setGatePosition(int angle);
    int getGatePosition();
    bool isGateOpen();
    
    // Ultrasonic sensor methods
    float getDistance();
    bool isObjectDetected();
    void updateDistance();
};

#endif // GATE_CONTROLLER_H
