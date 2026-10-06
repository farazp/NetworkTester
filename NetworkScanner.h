#pragma once

#include <Arduino.h>
#include <IPAddress.h>

#include "INetworkBackend.h"
#include "TestResult.h"

class NetworkScanner
{
public:
    explicit NetworkScanner(INetworkBackend& backend);

    void reset();

    bool prepare(
        const IPAddress& localIp,
        const IPAddress& subnetMask
    );

    bool update(TestResult& result);

    bool isRunning() const;
    bool isFinished() const;
    bool wasSkipped() const;

private:
    INetworkBackend& backend_;

    bool prepared_ = false;
    bool running_ = false;
    bool finished_ = false;
    bool skipped_ = false;

    IPAddress localIp_;
    IPAddress mask_;
    IPAddress network_;

    uint32_t networkValue_ = 0;
    uint32_t currentHostValue_ = 0;
    uint32_t lastHostValue_ = 0;

    uint16_t prefixLength_ = 0;
    uint16_t attempted_ = 0;

    uint32_t lastProbeMs_ = 0;

    uint8_t prefixLengthFromMask(const IPAddress& mask) const;
    uint32_t ipToUInt32(const IPAddress& ip) const;
    IPAddress uint32ToIp(uint32_t value) const;

    void recordHost(
        TestResult& result,
        const IPAddress& ip,
        uint16_t port,
        uint16_t elapsedMs
    );
};