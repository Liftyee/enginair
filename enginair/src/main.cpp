// enginair main.cpp
// Read Particulate matter (PM), CO2, Temperature and humidity from SEN50 and SCD40 sensors.
// Display on an SSD1306 128x32 OLED.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "symbols.h"
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_ADDRESS 0x3C
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1); // -1: no reset pin
#include <Fonts/FreeSans9pt7b.h> // TODO: Convert Meshtastic font ArialMT_Plain_10 to Adafruit GFX font
// Meshtastic FONT_MEDIUM = ArialMT_Plain_16, FONT_SMALL = ArialMT_Plain_10

SensirionI2CSen5x pmSens;
SensirionI2CScd4x co2Sens;

// TODO: figure out how to pass in a TwoWire pointer
void initSEN50();
void initSCD40();
void printSensirionError(String message, uint16_t error);

#define PROJECT_NAME "enginAIR"
enum message_t {
    DEBUG,
    NAME,
    INFO,
    WARN,
    ERR
};

bool initDisplay();
void showMessage(String message, message_t level);
void showPMValues(float pm1p0, float pm2p5, float pm4p0, float pm10p0);
void showCO2Values(uint16_t co2, float temp, float humi);
void showValues_LargeText(float pm2p5, uint16_t co2, float temp, float humi);

void setup() {
    Wire.setSCL(17);
    Wire.setSDA(16);
    Wire.begin();

    initDisplay(); // OLED display init early, so we can show a message
    Serial.begin(115200);

    showMessage("Waiting for serial", NAME);
    while (!Serial) {
        delay(100);
        break; // don't wait 
    }
    showMessage("Connected", NAME);

    initSEN50(); // PM sensor init
    initSCD40(); // CO2 sensor init

    showMessage("Init complete", NAME);
    Serial.println("Starting main loop");
}

int seconds = 0;
void loop() {
    uint16_t error;
    char errorMessage[256];
    float pm1p0, pm2p5, pm4p0, pm10p0, sen_temp, nox, voc, sen_humi;

    delay(1000);
    seconds++;

    error = pmSens.readMeasuredValues(pm1p0, pm2p5, pm4p0, pm10p0, sen_humi, sen_temp, voc, nox);
    if (error) {
        printSensirionError("Error reading measured values from SEN50: ", error);
    } else { 
        // showPMValues(pm1p0, pm2p5, pm4p0, pm10p0);

        // SEN50 has no VOC, NOx, humidity or temperature sensor.
    }

    // The SCD40 CO2 sensor only produces a new measurement every 5 seconds, 
    // and clears the buffer after reading, so check if data is ready
    uint16_t co2;
    float temp, humi;
    bool dataReady = false;
    error = co2Sens.getDataReadyFlag(dataReady);
    if (error) {
        printSensirionError("Couldn't get SCD40 data ready flag: ", error);
        return; // Don't do anything after this if an error occurred at this stage
    }
    
    // Serial.print("Data ready: ");
    // Serial.println(dataReady);
    if (dataReady) {
        error = co2Sens.readMeasurement(co2, temp, humi);
        if (error) {
            printSensirionError("Error reading measurement from SCD40: ", error);
        } else {
            // showCO2Values(co2, temp, humi);
        }
    }

    // // Swap between the CO2 and PM values every 5 seconds
    // if (seconds > 5) {
    //     showCO2Values(co2, temp, humi);
    // } else {
    //     showPMValues(pm1p0, pm2p5, pm4p0, pm10p0);
    // }
    showValues_LargeText(pm2p5, co2, temp, humi);

    if (seconds >= 10) {
        seconds = 0;
    }
}

void initSEN50() {
    pmSens.begin(Wire);

    uint16_t error;
    char message[256];
    error = pmSens.deviceReset();
    if (error) {
        printSensirionError("Error resetting SEN50: ", error);
    } 

    error = pmSens.startMeasurement();
    if (error) {
        printSensirionError("Error starting SEN50 measurement: ", error);
    } else {
        Serial.println("SEN50 measurement started successfully");
    }
}

void initSCD40() {
    co2Sens.begin(Wire);

    // A measurement might be running from a previous startup
    uint16_t error;
    char message[256];
    error = co2Sens.stopPeriodicMeasurement();
    if (error) {
        printSensirionError("Error stopping SCD40 measurement: ", error);
    }

    error = co2Sens.startPeriodicMeasurement();
    if (error) {
        printSensirionError("Error starting SCD40 measurement: ", error);
    } else {
        Serial.println("SCD40 measurement started successfully");
    }
}

// Print an error message and a decoded Sensirion error code
void printSensirionError(String message, uint16_t error) {
    char errorMessage[256];
    errorToString(error, errorMessage, 256);
    Serial.print(message);
    Serial.println(errorMessage);
}

// Initialise the SSD1306 OLED display settings and display a small message
bool initDisplay() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println(F("Couldn't initialise SSD1306"));
        return false;
    }

    display.setTextSize(1);
    // TODO: Uncomment when font fixed
    //display.setFont(&FreeSans9pt7b);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // foreground, background
    display.setCursor(0, 20);
    display.println("display init...");
    display.display();

    return true;
}

// Display a short message on the OLED display (and Serial) with a "log level"
void showMessage(String message, message_t level) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    
    switch (level) {
        case DEBUG:
            display.println("DEBUG: ");
            Serial.print("DEBUG: ");
            break;
        case INFO:
            display.println("INFO: ");
            Serial.print("INFO: ");
            break;
        case WARN:
            display.println("WARN: ");
            Serial.print("WARN: ");
            break;
        case ERR:
            display.println("ERROR: ");
            Serial.print("ERROR: ");
            break;
        case NAME:
            display.print(PROJECT_NAME);
            display.println(": ");
            Serial.print(PROJECT_NAME);
            Serial.print(": ");
            break;
        default:
            display.println("(no msg type): ");
    }

    display.println(message);
    display.display();

    // print to serial, for good measure
    Serial.println(message);
}

#define BOTLINE_Y 29
#define TOPLINE_Y 14
#define RIGHTHALF_X 64
void showValues_LargeText(float pm2p5, uint16_t co2, float temp, float humi) {
    int x, y; // temp vars
    display.clearDisplay();
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
    display.setCursor(0,TOPLINE_Y);
    display.print(String(pm2p5, 1));

    display.setFont(); // reset to default font
    x = display.getCursorX();
    display.drawBitmap(x+2, 0, icon_ugm3, 16, 16, SSD1306_WHITE);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(RIGHTHALF_X, TOPLINE_Y);
    display.print(co2);
    display.setFont();
    display.setCursor(display.getCursorX()+1, display.getCursorY());
    display.print("ppm");

    display.setFont(&FreeSans9pt7b);
    display.setCursor(0, BOTLINE_Y);
    display.print(String(temp, 1));
    x = display.getCursorX();
    y = display.getCursorY();
    display.drawBitmap(x+1, y-10, icon_degC, 8, 7, SSD1306_WHITE);
    display.setCursor(RIGHTHALF_X, BOTLINE_Y);
    display.print(String(humi, 1));
    display.print("%");
    display.display();
}

// Show the PM values on the OLED and serial monitor
void showPMValues(float pm1p0, float pm2p5, float pm4p0, float pm10p0) {
    Serial.print("PM1.0: ");
    Serial.print(pm1p0);
    Serial.print("\t PM2.5: ");
    Serial.print(pm2p5);
    Serial.print("\t PM4.0: ");
    Serial.print(pm4p0);
    Serial.print("\t PM10.0: ");
    Serial.print(pm10p0);
    Serial.println();

    // Only show PM2.5 and PM10 for brevity
    display.clearDisplay();
    display.setCursor(16,0);
    display.drawBitmap(0, 0, icon_pm25, 16, 7, SSD1306_WHITE);
    display.print(pm2p5);
    display.println(" ug/m3");
    display.setCursor(16, 10);
    display.drawBitmap(0, 10, icon_pm10, 16, 7, SSD1306_WHITE);
    display.print(pm10p0);
    display.print(" ug/m3");
    display.display();
}

// Show the CO2 ppm, temperature and humidity on the OLED and serial monitor
void showCO2Values(uint16_t co2, float temp, float humi) {
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print("\t Temperature: ");
    Serial.print(temp);
    Serial.print("\t Humidity: ");
    Serial.print(humi);
    Serial.println();

    display.clearDisplay();
    display.setCursor(0,0);
    display.print("CO2: ");
    display.print(co2);
    display.print(" ppm");
    display.setCursor(0,10);
    display.print("Temp: ");
    display.print(temp);
    display.print("C");
    display.setCursor(0,20);
    display.print("Humidity: ");
    display.print(humi);
    display.print("%");
    display.display();
}