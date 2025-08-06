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
    
    // Timeout très court pour éviter les blocages
    long duration = pulseIn(echoPin, HIGH, 15000); // Timeout 15ms au lieu de 30ms
    
    if (duration == 0) {
        // En cas d'erreur, garder la dernière valeur valide
        return lastDistance > 0 ? lastDistance : 999.0;
    }
    
    float distance = duration * 0.034 / 2;
    
    // Filtrer les valeurs aberrantes
    if (distance > 2 && distance < 400) {
        lastDistance = distance;
        lastReadTime = millis();
        return distance;
    }
    
    // Retourner la dernière valeur valide si mesure aberrante
    return lastDistance > 0 ? lastDistance : 999.0;
}

float DistanceSensor::getLastDistance() const {
    return lastDistance;
}

bool DistanceSensor::isObjectDetected(float thresholdCm) {
    return (lastDistance > 0 && lastDistance < thresholdCm);
}

void DistanceSensor::update() {
    unsigned long currentTime = millis();
    // Réduire la fréquence de lecture pour économiser CPU
    if (currentTime - lastReadTime >= 500) { // Lecture toutes les 500ms au lieu de 100ms
        readDistance();
        yield(); // Donner du temps au watchdog
    }
}
