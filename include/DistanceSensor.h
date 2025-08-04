#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include <Arduino.h>

class DistanceSensor {
private:
    int trigPin;
    int echoPin;
    float lastDistance;
    unsigned long lastReadTime;
    
public:
    DistanceSensor(int trig, int echo);
    bool init();
    float readDistance();
    float getLastDistance() const;
    bool isObjectDetected(float thresholdCm = 20.0);
    void update();
};

#endif
