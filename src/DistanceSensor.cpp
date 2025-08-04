#include "DistanceSensor.h"

DistanceSensor::DistanceSensor(int trig, int echo) 
    : trigPin(trig), echoPin(echo), lastDistance(0.0), lastReadTime(0) {
}

bool DistanceSensor::init() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    // Test initial
    digitalWrite(trigPin, LOW);
    delay(10);
    
    Serial.println("Distance sensor initialized");
    return true;
}

float DistanceSensor::readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 30000); // Timeout 30ms
    
    if (duration == 0) {
        return -1; // Erreur de lecture
    }
    
    float distance = duration * 0.034 / 2;
    
    // Filtrer les valeurs aberrantes
    if (distance > 0 && distance < 400) {
        lastDistance = distance;
        lastReadTime = millis();
    }
    
    return lastDistance;
}

float DistanceSensor::getLastDistance() const {
    return lastDistance;
}

bool DistanceSensor::isObjectDetected(float thresholdCm) {
    return (lastDistance > 0 && lastDistance < thresholdCm);
}

void DistanceSensor::update() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= 100) { // Lecture toutes les 100ms max
        readDistance();
    }
}
