#include "NetworkScanner.h"

#include "Config.h"
#include "PlatformCompat.h"

NetworkScanner::NetworkScanner(INetworkBackend& backend)
    : backend_(backend)
{
}

void NetworkScanner::reset()
{
    prepared_ = false;
    running_ = false;
    finished_ = false;
    skipped_ = false;

    networkValue_ = 0;
    currentHostValue_ = 0;
    lastHostValue_ = 0;

    prefixLength_ = 0;
    attempted_ = 0;
    lastProbeMs_ = 0;
}

uint32_t NetworkScanner::ipToUInt32(const IPAddress& ip) const
{
    return
        (static_cast<uint32_t>(ip[0]) << 24) |
        (static_cast<uint32_t>(ip[1]) << 16) |
        (static_cast<uint32_t>(ip[2]) << 8) |
        static_cast<uint32_t>(ip[3]);
}

IPAddress NetworkScanner::uint32ToIp(uint32_t value) const
{
    return IPAddress(
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    );
}

uint8_t NetworkScanner::prefixLengthFromMask(
    const IPAddress& mask
) const
{
    uint32_t value = ipToUInt32(mask);
    uint8_t count = 0;

    while (value & 0x80000000UL)
    {
        count++;
        value <<= 1;
    }

    return count;
}

bool NetworkScanner::prepare(
    const IPAddress& localIp,
    const IPAddress& subnetMask
)
{
    reset();

    localIp_ = localIp;
    mask_ = subnetMask;

    prefixLength_ = prefixLengthFromMask(mask_);

    if (prefixLength_ < 24)
    {
        skipped_ = true;
        finished_ = true;
        return false;
    }

    const uint32_t ipValue = ipToUInt32(localIp_);
    const uint32_t maskValue = ipToUInt32(mask_);

    networkValue_ = ipValue & maskValue;
    network_ = uint32ToIp(networkValue_);

    const uint32_t hostMask = ~maskValue;
    const uint32_t hostCount = hostMask + 1UL;

    if (hostCount < 4 || hostCount > SCAN_MAX_HOSTS + 2UL)
    {
        skipped_ = true;
        finished_ = true;
        return false;
    }

    currentHostValue_ = networkValue_ + 1UL;
    lastHostValue_ = networkValue_ + hostCount - 2UL;

    prepared_ = true;
    running_ = true;
    finished_ = false;
    lastProbeMs_ = 0;

    return true;
}

void NetworkScanner::recordHost(
    TestResult& result,
    const IPAddress& ip,
    uint16_t port,
    uint16_t elapsedMs
)
{
    result.scanResponded++;

    if (result.discoveredCount >= MAX_DISCOVERED_HOSTS)
    {
        return;
    }

    ScanHost& host = result.discovered[result.discoveredCount];

    host.ip = ip;
    host.port = port;
    host.elapsedMs = elapsedMs;

    result.discoveredCount++;
}

bool NetworkScanner::update(TestResult& result)
{
    if (!running_ || finished_)
    {
        return false;
    }

    const uint32_t now = millis();

    if ((now - lastProbeMs_) < SCAN_DELAY_MS)
    {
        return true;
    }

    if (currentHostValue_ > lastHostValue_ ||
        attempted_ >= SCAN_MAX_HOSTS)
    {
        running_ = false;
        finished_ = true;
        return false;
    }

    const IPAddress target = uint32ToIp(currentHostValue_);

    currentHostValue_++;
    attempted_++;

    result.scanAttempted = attempted_;

    if (target == localIp_)
    {
        return true;
    }

    uint32_t elapsedMs = 0;

    const bool connected = backend_.tcpConnect(
        target,
        SCAN_TCP_PORT,
        SCAN_TIMEOUT_MS,
        elapsedMs
    );

    lastProbeMs_ = millis();

    if (connected)
    {
        recordHost(
            result,
            target,
            SCAN_TCP_PORT,
            static_cast<uint16_t>(
                elapsedMs > 65535UL ? 65535UL : elapsedMs
            )
        );
    }

    platformYield();

    return true;
}

bool NetworkScanner::isRunning() const
{
    return running_;
}

bool NetworkScanner::isFinished() const
{
    return finished_;
}

bool NetworkScanner::wasSkipped() const
{
    return skipped_;
}