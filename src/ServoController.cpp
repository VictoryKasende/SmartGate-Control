#include "ServoController.h"

ServoController::ServoController(int pin, int openPos, int closedPos) 
    : servoPin(pin), isOpen(false), openAngle(openPos), closedAngle(closedPos) {
}

bool ServoController::init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.setPeriodHertz(50); // fréquence standard 50Hz pour servos
    servo.attach(servoPin, 500, 2400); 
    delay(100);
    
    // Position initiale fermée
    servo.write(closedAngle);
    isOpen = false;
    delay(500);
    
    Serial.printf("Servo controller initialized on pin %d (closed position: %d°)\n", 
                  servoPin, closedAngle);
    return true;
}

bool ServoController::openGate() {
    servo.write(openAngle);
    isOpen = true;
    delay(500); // Laisser le temps au servo de bouger
    
    Serial.printf("Gate OPENED (servo: %d°)\n", openAngle);
    return true;
}

bool ServoController::closeGate() {
    servo.write(closedAngle);
    isOpen = false;
    delay(500); // Laisser le temps au servo de bouger
    
    Serial.printf("Gate CLOSED (servo: %d°)\n", closedAngle);
    return true;
}

bool ServoController::setPosition(int angle) {
    if (angle < 0 || angle > 180) {
        Serial.println("Invalid servo angle");
        return false;
    }
    
    servo.write(angle);
    isOpen = (angle > (closedAngle + openAngle) / 2);
    delay(500);
    
    Serial.printf("Servo position set to %d°\n", angle);
    return true;
}

bool ServoController::isGateOpen() const {
    return isOpen;
}

int ServoController::getCurrentAngle() const {
    return isOpen ? openAngle : closedAngle;
}
