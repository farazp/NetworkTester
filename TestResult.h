#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include "ErrorCodes.h"

enum class TestStatus : uint8_t
{
    NotRun,
    Running,
    Pass,
    Fail,
    Warning,
    Skipped,
    Cancelled,
    NotAvailable
};

enum class ResultClassification : uint8_t
{
    Unknown,
    Pass,
    Warning,
    Fail,
    Cancelled
};

struct PhyStatus
{
    bool valid = false;
    bool linkUp = false;

    bool speedValid = false;
    uint16_t speedMbps = 0;

    bool duplexValid = false;
    bool fullDuplex = false;

    bool modeValid = false;
    char mode[16] = "N/A";
};

struct ScanHost
{
    IPAddress ip;
    uint16_t port = 0;
    uint16_t elapsedMs = 0;
};

struct TestResult
{
    TestStatus w5500 = TestStatus::NotRun;
    TestStatus link = TestStatus::NotRun;
    TestStatus dhcp = TestStatus::NotRun;
    TestStatus staticFallback = TestStatus::NotRun;
    TestStatus gateway = TestStatus::NotRun;
    TestStatus dns = TestStatus::NotRun;
    TestStatus tcp = TestStatus::NotRun;
    TestStatus scan = TestStatus::NotRun;

    ErrorCode error = ErrorCode::None;

    bool spiOk = false;
    bool w5500Detected = false;
    bool linkUp = false;
    bool usingStaticFallback = false;
    bool hasValidIPv4Config = false;

    uint32_t dhcpElapsedMs = 0;
    uint32_t gatewayElapsedMs = 0;
    uint32_t dnsElapsedMs = 0;
    uint32_t tcpElapsedMs = 0;

    IPAddress localIp;
    IPAddress subnetMask;
    IPAddress gatewayIp;
    IPAddress dnsIp;
    IPAddress dnsResolvedIp;

    PhyStatus phy;

    uint16_t scanAttempted = 0;
    uint16_t scanResponded = 0;
    uint8_t discoveredCount = 0;
    ScanHost discovered[50];

    char detail[48] = "";
};

inline const char* testStatusText(TestStatus status)
{
    switch (status)
    {
        case TestStatus::NotRun:       return "N/R";
        case TestStatus::Running:      return "RUN";
        case TestStatus::Pass:         return "OK";
        case TestStatus::Fail:         return "FAIL";
        case TestStatus::Warning:      return "WARN";
        case TestStatus::Skipped:      return "SKIP";
        case TestStatus::Cancelled:    return "CANCEL";
        case TestStatus::NotAvailable: return "N/A";
        default:                       return "?";
    }
}

inline const char* classificationText(ResultClassification value)
{
    switch (value)
    {
        case ResultClassification::Pass:
            return "PASS";

        case ResultClassification::Warning:
            return "WARNING";

        case ResultClassification::Fail:
            return "FAIL";

        case ResultClassification::Cancelled:
            return "CANCELLED";

        default:
            return "UNKNOWN";
    }
}