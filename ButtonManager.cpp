#include "ButtonManager.h"
#include "Config.h"

ButtonManager::ButtonManager(uint8_t pin)
    : pin_(pin)
{
}

void ButtonManager::begin()
{
    pinMode(pin_, INPUT_PULLUP);

    rawLast_ = digitalRead(pin_);
    stableState_ = rawLast_;

    lastRawChangeMs_ = millis();
}

void ButtonManager::update()
{
    const bool raw = digitalRead(pin_);
    const uint32_t now = millis();

    if (raw != rawLast_)
    {
        rawLast_ = raw;
        lastRawChangeMs_ = now;
    }

    if ((now - lastRawChangeMs_) < BUTTON_DEBOUNCE_MS)
    {
        return;
    }

    if (raw != stableState_)
    {
        stableState_ = raw;

        if (stableState_ == LOW)
        {
            pressActive_ = true;
            longReported_ = false;
            pressedAtMs_ = now;
        }
        else
        {
            if (pressActive_ && !longReported_)
            {
                shortPressed_ = true;
            }

            pressActive_ = false;
        }
    }

    if (pressActive_ &&
        !longReported_ &&
        (now - pressedAtMs_) >= BUTTON_LONG_PRESS_MS)
    {
        longReported_ = true;
        longPressed_ = true;
    }
}

bool ButtonManager::wasShortPressed()
{
    const bool result = shortPressed_;
    shortPressed_ = false;
    return result;
}

bool ButtonManager::wasLongPressed()
{
    const bool result = longPressed_;
    longPressed_ = false;
    return result;
}