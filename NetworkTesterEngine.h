#pragma once

#include <Arduino.h>

#include "ButtonManager.h"
#include "DisplayManager.h"
#include "INetworkBackend.h"
#include "NetworkScanner.h"
#include "TestResult.h"

enum class TestState : uint8_t
{
    Idle,
    Initializing,
    W5500Check,
    LinkTest,
    DhcpStart,
    DhcpWait,
    StaticFallback,
    GatewayTest,
    DnsTest,
    TcpTest,
    ScanPrepare,
    ScanRunning,
    Completed,
    Cancelled,
    Error
};

class NetworkTesterEngine
{
public:
    NetworkTesterEngine(
        INetworkBackend& backend,
        DisplayManager& display,
        ButtonManager& button
    );

    void begin();
    void update();

private:
    INetworkBackend& backend_;
    DisplayManager& display_;
    ButtonManager& button_;

    NetworkScanner scanner_;

    TestState state_ = TestState::Idle;
    TestResult result_;

    uint32_t stateStartedAtMs_ = 0;

    bool displayReady_ = false;

    void transitionTo(TestState nextState);
    void resetResult();

    void handleButton();
    void handleShortPress();
    void handleLongPress();

    void updateIdle();
    void updateInitializing();
    void updateW5500Check();
    void updateLinkTest();
    void updateDhcpStart();
    void updateDhcpWait();
    void updateStaticFallback();
    void updateGatewayTest();
    void updateDnsTest();
    void updateTcpTest();
    void updateScanPrepare();
    void updateScanRunning();

    void collectNetworkConfiguration();

    void fail(ErrorCode error, const char* detail);
    void complete();

    ResultClassification classification() const;
    bool validIpConfiguration() const;

    void logResult() const;
};