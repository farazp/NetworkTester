#include "PhyDiagnostics.h"

#include <SPI.h>
#include "Config.h"

namespace
{
    uint8_t makeControlByte(uint8_t blockSelect, bool read)
    {
        const uint8_t rwBit = read ? 0x04 : 0x00;
        const uint8_t omBits = 0x00;
        return static_cast<uint8_t>((blockSelect << 3) | rwBit | omBits);
    }
}

bool PhyDiagnostics::readPhyConfig(uint8_t csPin, uint8_t& phycfgr)
{
    SPI.beginTransaction(SPISettings(W5500_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));

    digitalWrite(csPin, LOW);

    SPI.transfer(static_cast<uint8_t>(W5500_PHYCFGR >> 8));
    SPI.transfer(static_cast<uint8_t>(W5500_PHYCFGR & 0xFF));
    SPI.transfer(makeControlByte(W5500_COMMON_REG_BLOCK, true));

    phycfgr = SPI.transfer(0x00);

    digitalWrite(csPin, HIGH);

    SPI.endTransaction();

    return true;
}

const char* PhyDiagnostics::phyModeText(uint8_t phycfgr)
{
    const bool softwareMode = (phycfgr & 0x40) != 0;

    if (!softwareMode)
    {
        return "HW CFG";
    }

    const uint8_t opmdc = static_cast<uint8_t>((phycfgr >> 3) & 0x07);

    switch (opmdc)
    {
        case 0x00: return "AUTO";
        case 0x01: return "100F";
        case 0x02: return "100H";
        case 0x03: return "10F";
        case 0x04: return "10H";
        case 0x05: return "POWERDOWN";
        case 0x06: return "AUTO";
        case 0x07: return "AUTO";
        default:   return "N/A";
    }
}

void PhyDiagnostics::decodePhyConfig(uint8_t phycfgr, PhyStatus& result)
{
    result.valid = true;

    result.linkUp = (phycfgr & 0x01) != 0;

    result.speedValid = result.linkUp;
    result.speedMbps = (phycfgr & 0x02) ? 100 : 10;

    result.duplexValid = result.linkUp;
    result.fullDuplex = (phycfgr & 0x04) != 0;

    result.modeValid = true;

    const char* text = phyModeText(phycfgr);

    strncpy(result.mode, text, sizeof(result.mode) - 1);
    result.mode[sizeof(result.mode) - 1] = '\0';
}