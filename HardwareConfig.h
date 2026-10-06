#pragma once

#include <Arduino.h>

// ============================================================
// ESP8266 NodeMCU / ESP-12E / ESP-12F
//
// W5500:
// SCK  -> D5 / GPIO14
// MISO -> D6 / GPIO12
// MOSI -> D7 / GPIO13
// CS   -> D0 / GPIO16
// RST  -> D1 / GPIO5
// INT  -> D2 / GPIO4 (optional)
//
// OLED:
// SDA  -> D3 / GPIO0
// SCL  -> D4 / GPIO2
//
// Button:
// GPIO3 / RX -> button -> GND
//
// IMPORTANT:
// GPIO0 and GPIO2 are boot-strap pins. OLED pull-ups must not
// pull them low at boot. GPIO3 shares UART RX.
// ============================================================

#if defined(ESP8266)

static constexpr uint8_t PIN_W5500_CS = 5;
static constexpr uint8_t PIN_W5500_RST = 6;
static constexpr int8_t  PIN_W5500_INT = 4;

static constexpr uint8_t W5500_SPI_SCK  = 18;
static constexpr uint8_t W5500_SPI_MISO = 19;
static constexpr uint8_t W5500_SPI_MOSI = 23;

static constexpr uint8_t PIN_I2C_SDA = 0;
static constexpr uint8_t PIN_I2C_SCL = 2;

static constexpr uint8_t PIN_BUTTON = 3;

#endif

// ============================================================
// ESP32-C3 SuperMini
//
// Board variants differ. Verify your board silkscreen and USB/
// boot wiring before deploying.
//
// W5500:
// SCK  -> GPIO4
// MISO -> GPIO5
// MOSI -> GPIO6
// CS   -> GPIO7
// RST  -> GPIO3
// INT  -> GPIO10 optional
//
// OLED:
// SDA -> GPIO8
// SCL -> GPIO9
//
// Button:
// GPIO2 -> button -> GND
// ============================================================

#if defined(ARDUINO_ARCH_ESP32)

#if defined(CONFIG_IDF_TARGET_ESP32S2)

// ESP32-S2 defaults used by the known-good generic ESP32/W5500 wiring.
// If your board uses a different PCB mapping, override these before including this header.
#ifndef PIN_W5500_CS
#define PIN_W5500_CS 5
#endif

#ifndef PIN_W5500_RST
#define PIN_W5500_RST 4
#endif

#ifndef PIN_W5500_INT
#define PIN_W5500_INT 6
#endif

#ifndef W5500_SPI_SCK
#define W5500_SPI_SCK 18
#endif

#ifndef W5500_SPI_MISO
#define W5500_SPI_MISO 19
#endif

#ifndef W5500_SPI_MOSI
#define W5500_SPI_MOSI 23
#endif

#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA 21
#endif

#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL 22
#endif

#ifndef PIN_BUTTON
#define PIN_BUTTON 12
#endif

#elif defined(CONFIG_IDF_TARGET_ESP32C3)

// ESP32-C3 SuperMini common mapping.
#ifndef PIN_W5500_CS
#define PIN_W5500_CS 7
#endif

#ifndef PIN_W5500_RST
#define PIN_W5500_RST 3
#endif

#ifndef PIN_W5500_INT
#define PIN_W5500_INT 10
#endif

#ifndef W5500_SPI_SCK
#define W5500_SPI_SCK 4
#endif

#ifndef W5500_SPI_MISO
#define W5500_SPI_MISO 5
#endif

#ifndef W5500_SPI_MOSI
#define W5500_SPI_MOSI 6
#endif

#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA 8
#endif

#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL 9
#endif

#ifndef PIN_BUTTON
#define PIN_BUTTON 2
#endif

#else

// Generic ESP32 fallback (for boards that are not using the S2/C3 target selection).
#ifndef PIN_W5500_CS
#define PIN_W5500_CS 5
#endif

#ifndef PIN_W5500_RST
#define PIN_W5500_RST 4
#endif

#ifndef PIN_W5500_INT
#define PIN_W5500_INT 6
#endif

#ifndef W5500_SPI_SCK
#define W5500_SPI_SCK 18
#endif

#ifndef W5500_SPI_MISO
#define W5500_SPI_MISO 19
#endif

#ifndef W5500_SPI_MOSI
#define W5500_SPI_MOSI 23
#endif

#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA 21
#endif

#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL 22
#endif

#ifndef PIN_BUTTON
#define PIN_BUTTON 12
#endif

#endif

#endif