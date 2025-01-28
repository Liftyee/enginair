// enginair main.cpp
// Read Particulate matter (PM), CO2, Temperature and humidity from SEN50 and SCD40 sensors.
// Display on an SSD1306 128x32 OLED.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_ADDRESS 0x3C
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1); // -1: no reset pin
// #include <Fonts/FreeSans9pt7b.h> // TODO: Convert Meshtastic font ArialMT_Plain_10 to Adafruit GFX font
// Meshtastic FONT_MEDIUM = ArialMT_Plain_16, FONT_SMALL = ArialMT_Plain_10

SensirionI2CSen5x pmSens;
SensirionI2CScd4x co2Sens;

// TODO: figure out how to pass in a TwoWire pointer
void initSEN50();
void initSCD40();

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

void setup() {
    Wire.setSCL(17);
    Wire.setSDA(16);
    Wire.begin();

    initDisplay(); // OLED display init early, so we can show a message
    Serial.begin(115200);

    showMessage("Waiting for serial", NAME);
    while (!Serial) {
        delay(100);
    }
    showMessage("Connected", NAME);

    initSEN50(); // PM sensor init
    initSCD40(); // CO2 sensor init

    showMessage("Init complete", NAME);
    Serial.println("Starting main loop");
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

    // The SCD40 CO2 sensor only produces a new measurement every 5 seconds, 
    // and clears the buffer after reading, so check if data is ready
    uint16_t co2 = 0;
    bool dataReady = false;
    error = co2Sens.getDataReadyFlag(dataReady);
    if (error) {
        Serial.print("Couldn't get SCD40 data ready flag: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return; // Don't do anything after this if an error occurred at this stage
    }
    
    if (dataReady) {
        error = co2Sens.readMeasurement(co2, temp, humi);
        if (error) {
            Serial.print("Error reading SCD40 measurement: ");
            errorToString(error, errorMessage, 256);
            Serial.println(errorMessage);
        } else {
            Serial.print("CO2: ");
            Serial.print(co2);
            Serial.print("\t Temperature: ");
            Serial.print(temp);
            Serial.print("\t Humidity: ");
            Serial.print(humi);
            Serial.println();
        }
    }
}

void initSEN50() {
    pmSens.begin(Wire);

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

void initSCD40() {
    co2Sens.begin(Wire);

    // A measurement might be running from a previous startup
    uint16_t error;
    char message[256];
    error = co2Sens.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error stopping SCD40 measurement: ");
        errorToString(error, message, 256);
        Serial.println(message);
    }

    error = co2Sens.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error starting SCD40 measurement: ");
        errorToString(error, message, 256);
        Serial.println(message);
    } else {
        Serial.println("SCD40 measurement started successfully");
    }
}

bool initDisplay() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println(F("Couldn't initialise SSD1306"));
        return false;
    }

    display.setTextSize(1);
    // TODO: Uncomment when font fixed
    //display.setFont(&FreeSans9pt7b);
    display.setTextColor(SSD1306_WHITE);
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
