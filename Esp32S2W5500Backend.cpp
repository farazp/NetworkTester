#if defined(ARDUINO_ARCH_ESP32)

#include "Esp32S2W5500Backend.h"

#include <SPI.h>
#include <Ethernet.h>

#include "Config.h"
#include "HardwareConfig.h"
#include "PhyDiagnostics.h"

Esp32S2W5500Backend::Esp32S2W5500Backend()
{
}

bool Esp32S2W5500Backend::begin()
{
    pinMode(PIN_W5500_CS, OUTPUT);
    digitalWrite(PIN_W5500_CS, HIGH);

    pinMode(PIN_W5500_RST, OUTPUT);
    digitalWrite(PIN_W5500_RST, LOW);
    delay(50);
    digitalWrite(PIN_W5500_RST, HIGH);
    delay(150);

#if ENABLE_SERIAL_DEBUG
    Serial.printf(
        "[W5500] SPI init: SCK=%d MISO=%d MOSI=%d CS=%d RST=%d\n",
        W5500_SPI_SCK,
        W5500_SPI_MISO,
        W5500_SPI_MOSI,
        PIN_W5500_CS,
        PIN_W5500_RST
    );
#endif

    SPI.begin(
        W5500_SPI_SCK,
        W5500_SPI_MISO,
        W5500_SPI_MOSI,
        PIN_W5500_CS
    );

    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    Ethernet.init(PIN_W5500_CS);
    delay(50);

    initialized_ = true;

    const EthernetHardwareStatus status = Ethernet.hardwareStatus();

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[W5500] hardware status: "));
    if (status == EthernetW5500)
    {
        Serial.println(F("W5500"));
    }
    else if (status == EthernetNoHardware)
    {
        Serial.println(F("NO_HARDWARE"));
        Serial.println(F("[W5500] Check wiring: SPI pins, CS pin, and reset line must match the actual board."));
    }
    else
    {
        Serial.println(F("UNKNOWN"));
    }
#endif

    return status == EthernetW5500;
}

void Esp32S2W5500Backend::update()
{
    if (!dhcpActive_)
    {
        return;
    }

    if (hasNonZeroIp(localIP()))
    {
        dhcpActive_ = false;
        dhcpComplete_ = true;
        dhcpSuccess_ = true;
        return;
    }

    if ((millis() - dhcpStartedAtMs_) >= dhcpTimeoutMs_)
    {
        dhcpActive_ = false;
        dhcpComplete_ = true;
        dhcpSuccess_ = false;
    }
}

bool Esp32S2W5500Backend::isChipDetected()
{
    if (!initialized_)
    {
        return false;
    }

    const EthernetHardwareStatus status = Ethernet.hardwareStatus();

    return status == EthernetW5500;
}

bool Esp32S2W5500Backend::isLinkUp()
{
    return Ethernet.linkStatus() == LinkON;
}

bool Esp32S2W5500Backend::readPhyStatus(PhyStatus& phy)
{
    uint8_t phycfgr = 0;

    if (!PhyDiagnostics::readPhyConfig(PIN_W5500_CS, phycfgr))
    {
        return false;
    }

    PhyDiagnostics::decodePhyConfig(phycfgr, phy);

    return true;
}

bool Esp32S2W5500Backend::beginDhcp(uint32_t timeoutMs)
{
    dhcpTimeoutMs_ = timeoutMs;
    dhcpStartedAtMs_ = millis();

    dhcpComplete_ = false;
    dhcpSuccess_ = false;
    dhcpActive_ = true;

    Ethernet.begin(
        const_cast<uint8_t*>(DEVICE_MAC),
        timeoutMs,
        1000UL
    );

    return true;
}

bool Esp32S2W5500Backend::isDhcpComplete()
{
    return dhcpComplete_;
}

bool Esp32S2W5500Backend::isDhcpSuccess()
{
    return dhcpSuccess_;
}

bool Esp32S2W5500Backend::configureStatic(
    IPAddress ip,
    IPAddress gateway,
    IPAddress subnet,
    IPAddress dns
)
{
    Ethernet.begin(
        const_cast<uint8_t*>(DEVICE_MAC),
        ip,
        dns,
        gateway,
        subnet
    );

    return hasNonZeroIp(localIP());
}

IPAddress Esp32S2W5500Backend::localIP()
{
    return Ethernet.localIP();
}

IPAddress Esp32S2W5500Backend::subnetMask()
{
    return Ethernet.subnetMask();
}

IPAddress Esp32S2W5500Backend::gatewayIP()
{
    return Ethernet.gatewayIP();
}

IPAddress Esp32S2W5500Backend::dnsIP()
{
    return Ethernet.dnsServerIP();
}

bool Esp32S2W5500Backend::resolveDns(
    const char* hostname,
    IPAddress& result,
    uint32_t timeoutMs
)
{
    result = IPAddress();

    EthernetClient client;

    const uint32_t started = millis();

    const bool connected = client.connect(hostname, 80);

    const uint32_t elapsed = millis() - started;

    if (connected)
    {
        result = client.remoteIP();
        client.stop();
        return true;
    }

    if (elapsed > timeoutMs)
    {
        if (client.connected())
        {
            client.stop();
        }

        return false;
    }

    return false;
}

bool Esp32S2W5500Backend::tcpConnect(
    const char* host,
    uint16_t port,
    uint32_t timeoutMs,
    uint32_t& elapsedMs
)
{
    EthernetClient client;

    const uint32_t started = millis();

    const int result = client.connect(host, port);

    elapsedMs = millis() - started;

    if (elapsedMs > timeoutMs)
    {
        if (client.connected())
        {
            client.stop();
        }

        return false;
    }

    if (client.connected())
    {
        client.stop();
    }

    return result == 1;
}

bool Esp32S2W5500Backend::tcpConnect(
    IPAddress ip,
    uint16_t port,
    uint32_t timeoutMs,
    uint32_t& elapsedMs
)
{
    EthernetClient client;

    const uint32_t started = millis();

    const int result = client.connect(ip, port);

    elapsedMs = millis() - started;

    if (elapsedMs > timeoutMs)
    {
        if (client.connected())
        {
            client.stop();
        }

        return false;
    }

    if (client.connected())
    {
        client.stop();
    }

    return result == 1;
}

BackendCapabilities Esp32S2W5500Backend::capabilities() const
{
    BackendCapabilities result;

    result.dhcpSupported = true;
    result.dnsSupported = true;
    result.tcpSupported = true;

    result.rawEthernetSupported = false;
    result.arpCacheReadable = false;
    result.icmpPingSupported = false;

    result.directPhyReadSupported = true;

    return result;
}

const char* Esp32S2W5500Backend::backendName() const
{
    return "ESP32-S2 Ethernet W5500";
}

bool Esp32S2W5500Backend::hasNonZeroIp(const IPAddress& ip) const
{
    return ip[0] != 0 ||
           ip[1] != 0 ||
           ip[2] != 0 ||
           ip[3] != 0;
}

#endif
