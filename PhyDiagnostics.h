#pragma once

#include <Arduino.h>
#include "TestResult.h"

namespace PhyDiagnostics
{
    static constexpr uint16_t W5500_PHYCFGR = 0x005E;
    static constexpr uint8_t W5500_COMMON_REG_BLOCK = 0x00;
    static constexpr uint8_t W5500_READ_VDM = 0x00;

    bool readRegister(uint8_t csPin, uint16_t addr, uint8_t& value);
    bool readPhyConfig(uint8_t csPin, uint8_t& phycfgr);
    void decodePhyConfig(uint8_t phycfgr, PhyStatus& result);

    const char* phyModeText(uint8_t phycfgr);
}