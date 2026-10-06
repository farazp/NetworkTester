#pragma once

#include <Arduino.h>

class ButtonManager
{
public:
    explicit ButtonManager(uint8_t pin);

    void begin();
    void update();

    bool wasShortPressed();
    bool wasLongPressed();

private:
    uint8_t pin_;

    bool rawLast_ = HIGH;
    bool stableState_ = HIGH;

    bool pressActive_ = false;
    bool longReported_ = false;

    uint32_t lastRawChangeMs_ = 0;
    uint32_t pressedAtMs_ = 0;

    bool shortPressed_ = false;
    bool longPressed_ = false;
};