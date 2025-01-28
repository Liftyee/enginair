// enginair main.cpp
// Read PM, CO2, Temperature and humidity from SEN50 and SCD40 sensors.
// Display on an SSD1306 128x32 OLED.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>

SensirionI2CSen5x pmSens;
SensirionI2CScd4x co2Sens;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    Wire.setSCL(17);
    Wire.setSDA(16);
    Wire.begin();

    delay(100);
    pmSens.begin(Wire);
    delay(100);

    uint16_t error;
    char message[256];
    error = pmSens.deviceReset();
    if (error) {
        Serial.print("Error resetting SEN50: ");
        errorToString(error, message, 256);
        Serial.println(message);
    } 

    error = pmSens.startMeasurement();
    if (error) {
        Serial.print("Error starting SEN50 measurement: ");
        errorToString(error, message, 256);
        Serial.println(message);
    } else {
        Serial.println("SEN50 measurement started successfully");
    }
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    float pm1p0, pm2p5, pm4p0, pm10p0, temp, nox, voc, humi;

    delay(1000);

    error = pmSens.readMeasuredValues(pm1p0, pm2p5, pm4p0, pm10p0, humi, temp, voc, nox);
    if (error) {
        Serial.print("Error reading measured values from SEN50: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else { 
        Serial.print("PM1.0: ");
        Serial.print(pm1p0);
        Serial.print("\t PM2.5: ");
        Serial.print(pm2p5);
        Serial.print("\t PM4.0: ");
        Serial.print(pm4p0);
        Serial.print("\t PM10.0: ");
        Serial.print(pm10p0);
        Serial.println();

        // SEN50 has no VOC, NOx, humidity or temperature sensor.
    }
}