//
// Autonomous Cockroach
//
// A Lego and Arduino robotic cockroach that runs away from light
// Developed with [embedXcode](http://embedXcode.weebly.com)
//
// Author 		Ates Goral
// 				Magnetiq
//
// Date			2015-02-08 11:01 AM
// Version		<#version#>
//
// Copyright	© Ates Goral, 2015
// Licence		<#license#>
//
// See         ReadMe.txt for references
//


// Core library for code-sense - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(SPARK) // Spark specific
#include "application.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

const uint8_t ledPin = 13;

const uint8_t controlPin1 = 3;
const uint8_t controlPin2 = 9;
const uint8_t controlPin3 = 10;
const uint8_t controlPin4 = 11;

const uint8_t lightSensorPin1 = A0;
const uint8_t lightSensorPin2 = A1;

void steerCenter() {
    digitalWrite(controlPin1, LOW);
    digitalWrite(controlPin2, LOW);
}

void steerRight(double amount) {
    analogWrite(controlPin1, amount * 255.0 + 0.5);
    digitalWrite(controlPin2, LOW);
}

void steerLeft(double amount) {
    digitalWrite(controlPin1, LOW);
    analogWrite(controlPin2, amount * 255.0 + 0.5);
}

void steer(double amount) {
    if (amount >= 0) {
        steerRight(amount);
    } else {
        steerLeft(-amount);
    }
}

void stop() {
    digitalWrite(controlPin3, LOW);
    digitalWrite(controlPin4, LOW);
}

void forward(double amount) {
    digitalWrite(controlPin3, LOW);
    analogWrite(controlPin4, amount * 255.0 + 0.5);
}

void back(double amount) {
    analogWrite(controlPin3, amount * 255.0 + 0.5);
    digitalWrite(controlPin4, LOW);
}

void move(double amount) {
    if (amount >= 0) {
        forward(amount);
    } else {
        back(-amount);
    }
}

void ledOn() {
    digitalWrite(ledPin, HIGH);
}

void ledOff() {
    digitalWrite(ledPin, LOW);
}

class LightSensor {
    int pin;
    float baselineSampleSum;
    int baselineSampleCount;

    int readRaw() {
        int raw = analogRead(this->pin);
        delay(5);
        return raw;
    }
public:
    LightSensor(int pin) {
        this->pin = pin;
        this->baselineSampleSum = 0.0;
        this->baselineSampleCount = 0;
    }

    void sampleBaseline() {
        this->baselineSampleSum += this->readRaw();
        this->baselineSampleCount++;
    }

    float read() {
        float baseline = this->baselineSampleSum / this->baselineSampleCount;
        float raw = this->readRaw();

        if (raw < baseline) {
            raw = baseline;
        }

        return (raw - baseline) / 1023;
    }
};

LightSensor rightLightSensor(lightSensorPin1);
LightSensor leftLightSensor(lightSensorPin2);

void calibrate() {
    ledOn();

    while (millis() < 5000) {
        rightLightSensor.sampleBaseline();
        leftLightSensor.sampleBaseline();
    }

    ledOff();
}

void setup() {
    Serial.begin(9600);

    pinMode(ledPin, OUTPUT);

    pinMode(controlPin1, OUTPUT);
    pinMode(controlPin2, OUTPUT);
    pinMode(controlPin3, OUTPUT);
    pinMode(controlPin4, OUTPUT);

    stop();
    steerCenter();
    ledOff();

    calibrate();
    //forward(1.0);
}

void loop() {
    float rightLight = rightLightSensor.read();
    float leftLight = leftLightSensor.read();

    float maxLight = rightLight > leftLight ? rightLight : leftLight;
    float lightDelta = rightLight - leftLight;
    float absLightDelta = lightDelta > 0 ? lightDelta : -lightDelta;

    Serial.print("Max: ");
    Serial.print(maxLight);
    Serial.print(" Delta: ");
    Serial.print(lightDelta);
    Serial.print(" Abs Delta: ");
    Serial.print(absLightDelta);
    Serial.println();

    if (maxLight < 0.05) {
        // Happily in the dark. Stop where you are.
        steerCenter();
        stop();
    } else if (absLightDelta < 0.05) {
        // Not enough directional light difference to steer away, run backwards.
        steerCenter();
        back(1.0);
    } else {
        // Run towards dark.
        if (lightDelta > 0) {
            steerLeft(1.0);
        } else {
            steerRight(1.0);
        }
        forward(1.0);
    }
}
