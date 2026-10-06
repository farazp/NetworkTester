#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "TestResult.h"

struct BackendCapabilities
{
    bool dhcpSupported = true;
    bool dnsSupported = true;
    bool tcpSupported = true;

    bool rawEthernetSupported = false;
    bool arpCacheReadable = false;
    bool icmpPingSupported = false;

    bool directPhyReadSupported = true;
};

class INetworkBackend
{
public:
    virtual ~INetworkBackend() = default;

    virtual bool begin() = 0;
    virtual void update() = 0;

    virtual bool isChipDetected() = 0;
    virtual bool isLinkUp() = 0;

    virtual bool readPhyStatus(PhyStatus& phy) = 0;

    virtual bool beginDhcp(uint32_t timeoutMs) = 0;
    virtual bool isDhcpComplete() = 0;
    virtual bool isDhcpSuccess() = 0;

    virtual bool configureStatic(
        IPAddress ip,
        IPAddress gateway,
        IPAddress subnet,
        IPAddress dns
    ) = 0;

    virtual IPAddress localIP() = 0;
    virtual IPAddress subnetMask() = 0;
    virtual IPAddress gatewayIP() = 0;
    virtual IPAddress dnsIP() = 0;

    virtual bool resolveDns(
        const char* hostname,
        IPAddress& result,
        uint32_t timeoutMs
    ) = 0;

    virtual bool tcpConnect(
        const char* host,
        uint16_t port,
        uint32_t timeoutMs,
        uint32_t& elapsedMs
    ) = 0;

    virtual bool tcpConnect(
        IPAddress ip,
        uint16_t port,
        uint32_t timeoutMs,
        uint32_t& elapsedMs
    ) = 0;

    virtual BackendCapabilities capabilities() const = 0;
    virtual const char* backendName() const = 0;
};