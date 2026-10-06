#include <Arduino.h>

// Override W5500 pins to match your wiring (must be before HardwareConfig.h)
#define PIN_W5500_CS 5
#define PIN_W5500_RST 4
#define W5500_SPI_SCK 18
#define W5500_SPI_MISO 19
#define W5500_SPI_MOSI 23

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
    //For Test
        //#include "Esp32C3W5500Backend.h"
        #include "Esp32S2W5500Backend.h"
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
        //Esp32C3W5500Backend networkBackend;
        Esp32S2W5500Backend networkBackend;
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