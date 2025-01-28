#pragma once

typedef unsigned char uint8_t;
#define PROGMEM

// 16 width 7 height
const uint8_t icon_pm25[] PROGMEM = {
    0b00000000, 0b00000000,
    0b11000001, 0b11000111,
    0b11001100, 0b01000100,
    0b00001101, 0b11000111,
    0b01100001, 0b00000001,
    0b01100001, 0b11010111,
    0b00000000, 0b00000000,
};

const uint8_t icon_pm10[] PROGMEM = {
    0b01000000, 0b01001110,
    0b11101000, 0b11001010,
    0b01011100, 0b01001010,
    0b00101000, 0b01001010,
    0b01110000, 0b11101110,
    0b00100000, 0b00000000,
    0b00000000, 0b00000000,
};

const uint8_t icon_degC[] PROGMEM = {
    0b11100111,
    0b10101001,
    0b11101000,
    0b00001000,
    0b00001000,
    0b00001001,
    0b00000110,
};

// following icons created with https://javl.github.io/image2cpp/

// 'ugm3', 16x16px
const uint8_t icon_ugm3[] PROGMEM = {
0x93, 0x80, 0x94, 0x80, 0x94, 0x80, 0xe3, 0x80, 0x80, 0x80, 0x84, 0x80, 0x83, 0x07, 0x00, 0x78, 
0x0f, 0x80, 0x00, 0x07, 0x00, 0x01, 0x01, 0xa7, 0x01, 0x51, 0x01, 0x57, 0x01, 0x50, 0x00, 0x00
};

const uint8_t icon_cubed[] PROGMEM = {
    0b11100111, 0b10011110};

//const uint8_t icon_ugm3[] PROGMEM = {
