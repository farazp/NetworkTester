#pragma once

#include <Arduino.h>

#if defined(ESP8266)
    #define PLATFORM_ESP8266 1
#else
    #define PLATFORM_ESP8266 0
#endif

#if defined(ARDUINO_ARCH_ESP32)
    #define PLATFORM_ESP32 1
#else
    #define PLATFORM_ESP32 0
#endif

#if PLATFORM_ESP32 && defined(CONFIG_IDF_TARGET_ESP32S2)
    #define PLATFORM_ESP32_S2 1
    #define PLATFORM_ESP32_C3 0
#elif PLATFORM_ESP32 && defined(CONFIG_IDF_TARGET_ESP32C3)
    #define PLATFORM_ESP32_S2 0
    #define PLATFORM_ESP32_C3 1
#else
    #define PLATFORM_ESP32_S2 0
    #define PLATFORM_ESP32_C3 0
#endif

#if !PLATFORM_ESP8266 && !PLATFORM_ESP32
    #error "This project supports ESP8266, ESP32-C3, and ESP32-S2 only."
#endif

inline void platformYield()
{
#if PLATFORM_ESP8266
    yield();
#elif PLATFORM_ESP32
    delay(0);
#endif
}

inline const char* platformName()
{
#if PLATFORM_ESP8266
    return "ESP8266";
#elif PLATFORM_ESP32_S2
    return "ESP32-S2";
#elif PLATFORM_ESP32_C3
    return "ESP32-C3";
#elif PLATFORM_ESP32
    return "ESP32";
#else
    return "UNKNOWN";
#endif
}