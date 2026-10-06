#include <Arduino.h>

#include "Config.h"
#include "HardwareConfig.h"
#include "PlatformCompat.h"

#include "ButtonManager.h"
#include "DisplayManager.h"
#include "NetworkTesterEngine.h"

#if defined(ESP8266)
    #include "Esp8266W5500Backend.h"
#elif defined(ARDUINO_ARCH_ESP32)
    #if defined(CONFIG_IDF_TARGET_ESP32S2)
        #include "Esp32S2W5500Backend.h"
    #else
        #include "Esp32C3W5500Backend.h"
    #endif
#endif

DisplayManager displayManager;
ButtonManager buttonManager(PIN_BUTTON);

#if defined(ESP8266)
    Esp8266W5500Backend networkBackend;
#elif defined(ARDUINO_ARCH_ESP32)
    #if defined(CONFIG_IDF_TARGET_ESP32S2)
        Esp32S2W5500Backend networkBackend;
    #else
        Esp32C3W5500Backend networkBackend;
    #endif
#endif

NetworkTesterEngine testerEngine(
    networkBackend,
    displayManager,
    buttonManager
);

void setup()
{
#if ENABLE_SERIAL_DEBUG
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println(F("NetworkTester booting..."));
#endif

    testerEngine.begin();
}

void loop()
{
    testerEngine.update();
}