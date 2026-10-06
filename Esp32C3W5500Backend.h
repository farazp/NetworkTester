#pragma once

#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <IPAddress.h>

#include "INetworkBackend.h"

class Esp32C3W5500Backend : public INetworkBackend
{
public:
    Esp32C3W5500Backend();

    bool begin() override;
    void update() override;

    bool isChipDetected() override;
    bool isLinkUp() override;

    bool readPhyStatus(PhyStatus& phy) override;

    bool beginDhcp(uint32_t timeoutMs) override;
    bool isDhcpComplete() override;
    bool isDhcpSuccess() override;

    bool configureStatic(
        IPAddress ip,
        IPAddress gateway,
        IPAddress subnet,
        IPAddress dns
    ) override;

    IPAddress localIP() override;
    IPAddress subnetMask() override;
    IPAddress gatewayIP() override;
    IPAddress dnsIP() override;

    bool resolveDns(
        const char* hostname,
        IPAddress& result,
        uint32_t timeoutMs
    ) override;

    bool tcpConnect(
        const char* host,
        uint16_t port,
        uint32_t timeoutMs,
        uint32_t& elapsedMs
    ) override;

    bool tcpConnect(
        IPAddress ip,
        uint16_t port,
        uint32_t timeoutMs,
        uint32_t& elapsedMs
    ) override;

    BackendCapabilities capabilities() const override;
    const char* backendName() const override;

private:
    bool initialized_ = false;

    bool dhcpActive_ = false;
    bool dhcpComplete_ = false;
    bool dhcpSuccess_ = false;

    uint32_t dhcpStartedAtMs_ = 0;
    uint32_t dhcpTimeoutMs_ = 0;

    bool hasNonZeroIp(const IPAddress& ip) const;
};

#endif