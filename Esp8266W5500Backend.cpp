#if defined(ESP8266)

#include "Esp8266W5500Backend.h"

#include <SPI.h>
#include <ESP8266WiFi.h>
#include <W5500lwIP.h>

#include "Config.h"
#include "HardwareConfig.h"
#include "PhyDiagnostics.h"
#include "PlatformCompat.h"

namespace
{
    Wiznet5500lwIP eth(PIN_W5500_CS);
}

Esp8266W5500Backend::Esp8266W5500Backend()
{
}

bool Esp8266W5500Backend::begin()
{
    pinMode(PIN_W5500_CS, OUTPUT);
    digitalWrite(PIN_W5500_CS, HIGH);

    pinMode(PIN_W5500_RST, OUTPUT);
    digitalWrite(PIN_W5500_RST, LOW);
    delay(5);
    digitalWrite(PIN_W5500_RST, HIGH);
    delay(50);

    SPI.begin();

    eth.setDefault();

    initialized_ = true;

    return isChipDetected();
}

void Esp8266W5500Backend::update()
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

bool Esp8266W5500Backend::isChipDetected()
{
    if (!initialized_)
    {
        return false;
    }

    uint8_t phycfgr = 0;

    return PhyDiagnostics::readPhyConfig(
        PIN_W5500_CS,
        phycfgr
    );
}

bool Esp8266W5500Backend::isLinkUp()
{
    PhyStatus phy;

    if (!readPhyStatus(phy))
    {
        return false;
    }

    return phy.linkUp;
}

bool Esp8266W5500Backend::readPhyStatus(PhyStatus& phy)
{
    uint8_t phycfgr = 0;

    if (!PhyDiagnostics::readPhyConfig(
        PIN_W5500_CS,
        phycfgr
    ))
    {
        return false;
    }

    PhyDiagnostics::decodePhyConfig(
        phycfgr,
        phy
    );

    return true;
}

bool Esp8266W5500Backend::beginDhcp(uint32_t timeoutMs)
{
    dhcpTimeoutMs_ = timeoutMs;
    dhcpStartedAtMs_ = millis();

    dhcpActive_ = true;
    dhcpComplete_ = false;
    dhcpSuccess_ = false;

    eth.setDefault();

    return true;
}

bool Esp8266W5500Backend::isDhcpComplete()
{
    return dhcpComplete_;
}

bool Esp8266W5500Backend::isDhcpSuccess()
{
    return dhcpSuccess_;
}

bool Esp8266W5500Backend::configureStatic(
    IPAddress ip,
    IPAddress gateway,
    IPAddress subnet,
    IPAddress dns
)
{
    staticIp_ = ip;
    staticGateway_ = gateway;
    staticMask_ = subnet;
    staticDns_ = dns;

    IPAddress dns2(0, 0, 0, 0);

    eth.config(
        staticIp_,
        staticGateway_,
        staticMask_,
        staticDns_,
        dns2
    );

    return hasNonZeroIp(localIP());
}

IPAddress Esp8266W5500Backend::localIP()
{
    return eth.localIP();
}

IPAddress Esp8266W5500Backend::subnetMask()
{
    return eth.subnetMask();
}

IPAddress Esp8266W5500Backend::gatewayIP()
{
    return eth.gatewayIP();
}

IPAddress Esp8266W5500Backend::dnsIP()
{
    return IPAddress(ip4_addr_get_u32(ip_2_ip4(dns_getserver(0))));
}

bool Esp8266W5500Backend::resolveDns(
    const char* hostname,
    IPAddress& result,
    uint32_t timeoutMs
)
{
    result = IPAddress();

    WiFiClient client;

    const uint32_t started = millis();

    const bool connected = client.connect(hostname, 80);

    const uint32_t elapsedMs = millis() - started;

    if (connected)
    {
        result = client.remoteIP();
        client.stop();
        return true;
    }

    if (elapsedMs > timeoutMs)
    {
        return false;
    }

    return false;
}

bool Esp8266W5500Backend::tcpConnect(
    const char* host,
    uint16_t port,
    uint32_t timeoutMs,
    uint32_t& elapsedMs
)
{
    WiFiClient client;

    client.setTimeout(
        static_cast<uint16_t>(
            (timeoutMs / 1000UL) + 1UL
        )
    );

    const uint32_t started = millis();

    const bool connected = client.connect(
        host,
        port
    );

    elapsedMs = millis() - started;

    if (client.connected())
    {
        client.stop();
    }

    if (elapsedMs > timeoutMs)
    {
        return false;
    }

    return connected;
}

bool Esp8266W5500Backend::tcpConnect(
    IPAddress ip,
    uint16_t port,
    uint32_t timeoutMs,
    uint32_t& elapsedMs
)
{
    WiFiClient client;

    client.setTimeout(
        static_cast<uint16_t>(
            (timeoutMs / 1000UL) + 1UL
        )
    );

    const uint32_t started = millis();

    const bool connected = client.connect(
        ip,
        port
    );

    elapsedMs = millis() - started;

    if (client.connected())
    {
        client.stop();
    }

    if (elapsedMs > timeoutMs)
    {
        return false;
    }

    return connected;
}

BackendCapabilities Esp8266W5500Backend::capabilities() const
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

const char* Esp8266W5500Backend::backendName() const
{
    return "ESP8266 W5500lwIP";
}

bool Esp8266W5500Backend::hasNonZeroIp(
    const IPAddress& ip
) const
{
    return
        ip[0] != 0 ||
        ip[1] != 0 ||
        ip[2] != 0 ||
        ip[3] != 0;
}

#endif