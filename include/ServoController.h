#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
private:
    Servo servo;
    int servoPin;
    bool isOpen;
    int openAngle;
    int closedAngle;
    
public:
    ServoController(int pin, int openPos = 0, int closedPos = 95);
    bool init();
    bool openGate();
    bool closeGate();
    bool setPosition(int angle);
    bool isGateOpen() const;
    int getCurrentAngle() const;
};

#endif