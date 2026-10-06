#include "NetworkTesterEngine.h"

#include "Config.h"
#include "PlatformCompat.h"

NetworkTesterEngine::NetworkTesterEngine(
    INetworkBackend& backend,
    DisplayManager& display,
    ButtonManager& button
)
    : backend_(backend),
      display_(display),
      button_(button),
      scanner_(backend)
{
}

void NetworkTesterEngine::begin()
{
    button_.begin();

    displayReady_ = display_.begin();

    if (!displayReady_)
    {
#if ENABLE_SERIAL_DEBUG
        Serial.println(F("[FATAL] OLED initialization failed"));
#endif
    }

    resetResult();

    if (displayReady_)
    {
        display_.showIdle();
    }

#if ENABLE_SERIAL_DEBUG
    Serial.println();
    Serial.println(F("================================"));
    Serial.println(F("Portable Ethernet Network Tester"));
    Serial.print(F("Platform: "));
    Serial.println(platformName());
    Serial.print(F("Backend : "));
    Serial.println(backend_.backendName());
    Serial.println(F("================================"));
#endif
}

void NetworkTesterEngine::update()
{
    button_.update();
    backend_.update();

    handleButton();

    switch (state_)
    {
        case TestState::Idle:
            updateIdle();
            break;

        case TestState::Initializing:
            updateInitializing();
            break;

        case TestState::W5500Check:
            updateW5500Check();
            break;

        case TestState::LinkTest:
            updateLinkTest();
            break;

        case TestState::DhcpStart:
            updateDhcpStart();
            break;

        case TestState::DhcpWait:
            updateDhcpWait();
            break;

        case TestState::StaticFallback:
            updateStaticFallback();
            break;

        case TestState::GatewayTest:
            updateGatewayTest();
            break;

        case TestState::DnsTest:
            updateDnsTest();
            break;

        case TestState::TcpTest:
            updateTcpTest();
            break;

        case TestState::ScanPrepare:
            updateScanPrepare();
            break;

        case TestState::ScanRunning:
            updateScanRunning();
            break;

        case TestState::Completed:
        case TestState::Cancelled:
        case TestState::Error:
            break;
    }

    platformYield();
}

void NetworkTesterEngine::transitionTo(TestState nextState)
{
    state_ = nextState;
    stateStartedAtMs_ = millis();

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[STATE] "));
    Serial.println(static_cast<uint8_t>(state_));
#endif
}

void NetworkTesterEngine::resetResult()
{
    result_ = TestResult{};
    scanner_.reset();
}

void NetworkTesterEngine::handleButton()
{
    if (button_.wasLongPressed())
    {
        handleLongPress();
    }

    if (button_.wasShortPressed())
    {
        handleShortPress();
    }
}

void NetworkTesterEngine::handleShortPress()
{
    if (state_ == TestState::Idle ||
        state_ == TestState::Completed ||
        state_ == TestState::Cancelled ||
        state_ == TestState::Error)
    {
        resetResult();
        transitionTo(TestState::Initializing);
        return;
    }

    if (displayReady_)
    {
        display_.nextPage(result_);
    }
}

void NetworkTesterEngine::handleLongPress()
{
    if (state_ == TestState::Idle)
    {
        if (displayReady_)
        {
            display_.showMessage(
                "CONFIG",
                "NOT IMPLEMENTED",
                "EDIT Config.h"
            );
        }

#if ENABLE_SERIAL_DEBUG
        Serial.println(F("[CONFIG] Configuration UI not implemented."));
        Serial.println(F("[CONFIG] Edit Config.h and rebuild firmware."));
#endif

        return;
    }

    if (state_ != TestState::Completed &&
        state_ != TestState::Cancelled &&
        state_ != TestState::Error)
    {
        result_.error = ErrorCode::E09_CANCELLED;

        result_.w5500 = TestStatus::Cancelled;
        result_.link = TestStatus::Cancelled;
        result_.dhcp = TestStatus::Cancelled;
        result_.gateway = TestStatus::Cancelled;
        result_.dns = TestStatus::Cancelled;
        result_.tcp = TestStatus::Cancelled;
        result_.scan = TestStatus::Cancelled;

        transitionTo(TestState::Cancelled);

        if (displayReady_)
        {
            display_.showMessage(
                "TEST CANCELLED",
                "E09 CANCELLED",
                "PRESS TO RESTART"
            );
        }

#if ENABLE_SERIAL_DEBUG
        Serial.println(F("[TEST] Cancelled by long press."));
#endif
    }
}

void NetworkTesterEngine::updateIdle()
{
    if (displayReady_ && display_.currentPage() != DisplayPage::Idle)
    {
        display_.showIdle();
    }
}

void NetworkTesterEngine::updateInitializing()
{
    if (displayReady_)
    {
        display_.showMessage(
            "INITIALIZING",
            "SPI / W5500",
            "PLEASE WAIT"
        );
    }

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[INIT] Starting SPI and W5500."));
#endif

    if (!backend_.begin())
    {
        fail(
            ErrorCode::E01_W5500_INIT_FAILED,
            "W5500 SPI INIT FAILED"
        );
        return;
    }

    result_.spiOk = true;

    transitionTo(TestState::W5500Check);
}

void NetworkTesterEngine::updateW5500Check()
{
    if (displayReady_)
    {
        display_.showMessage(
            "W5500 CHECK",
            "CHECKING CHIP",
            "SPI COMMUNICATION"
        );
    }

    result_.w5500Detected = backend_.isChipDetected();

    if (!result_.w5500Detected)
    {
        result_.w5500 = TestStatus::Fail;

        fail(
            ErrorCode::E01_W5500_INIT_FAILED,
            "W5500 NOT DETECTED"
        );

        return;
    }

    result_.w5500 = TestStatus::Pass;

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[W5500] Chip detected."));
#endif

    transitionTo(TestState::LinkTest);
}

void NetworkTesterEngine::updateLinkTest()
{
    result_.phy = PhyStatus{};

    const bool phyRead = backend_.readPhyStatus(result_.phy);

    if (!phyRead)
    {
        result_.phy.valid = false;
        result_.phy.linkUp = backend_.isLinkUp();
        result_.phy.speedValid = false;
        result_.phy.duplexValid = false;
        result_.phy.modeValid = false;
    }

    result_.linkUp = result_.phy.linkUp;

    if (displayReady_)
    {
        display_.showPhy(result_);
    }

    if (!result_.linkUp)
    {
        result_.link = TestStatus::Fail;

        fail(
            ErrorCode::E02_LINK_DOWN,
            "NO PHYSICAL LINK"
        );

        return;
    }

    result_.link = TestStatus::Pass;

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[PHY] Link UP."));

    if (result_.phy.speedValid)
    {
        Serial.print(F("[PHY] Speed: "));
        Serial.print(result_.phy.speedMbps);
        Serial.println(F(" Mbps"));
    }

    if (result_.phy.duplexValid)
    {
        Serial.print(F("[PHY] Duplex: "));
        Serial.println(
            result_.phy.fullDuplex ? F("FULL") : F("HALF")
        );
    }

    Serial.print(F("[PHY] Mode: "));
    Serial.println(result_.phy.mode);
#endif

    transitionTo(TestState::DhcpStart);
}

void NetworkTesterEngine::updateDhcpStart()
{
    result_.dhcp = TestStatus::Running;

    if (displayReady_)
    {
        display_.showMessage(
            "DHCP TEST",
            "REQUESTING IPV4",
            "PLEASE WAIT"
        );
    }

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[DHCP] Starting DHCP request."));
#endif

    if (!backend_.beginDhcp(DHCP_TIMEOUT_MS))
    {
        result_.dhcp = TestStatus::Fail;
        transitionTo(TestState::StaticFallback);
        return;
    }

    transitionTo(TestState::DhcpWait);
}

void NetworkTesterEngine::updateDhcpWait()
{
    if (!backend_.isDhcpComplete())
    {
        if (displayReady_)
        {
            display_.showMessage(
                "DHCP TEST",
                "WAITING RESPONSE",
                "LONG: CANCEL"
            );
        }

        return;
    }

    result_.dhcpElapsedMs = millis() - stateStartedAtMs_;

    if (!backend_.isDhcpSuccess())
    {
        result_.dhcp = TestStatus::Warning;
        result_.error = ErrorCode::E03_DHCP_TIMEOUT;

#if ENABLE_SERIAL_DEBUG
        Serial.println(F("[DHCP] No DHCP response."));
#endif

        transitionTo(TestState::StaticFallback);
        return;
    }

    result_.dhcp = TestStatus::Pass;
    result_.usingStaticFallback = false;

    collectNetworkConfiguration();

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[DHCP] Lease acquired."));
#endif

    if (!validIpConfiguration())
    {
        fail(
            ErrorCode::E04_INVALID_NETWORK_CONFIG,
            "INVALID DHCP IPV4 CONFIG"
        );

        return;
    }

    transitionTo(TestState::GatewayTest);
}

void NetworkTesterEngine::updateStaticFallback()
{
#if ENABLE_STATIC_FALLBACK

    if (displayReady_)
    {
        display_.showMessage(
            "DHCP NO RESPONSE",
            "STATIC TEST MODE",
            "APPLYING FALLBACK"
        );
    }

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[STATIC] Applying fallback static configuration."));
#endif

    const bool configured = backend_.configureStatic(
        fallbackIP(),
        fallbackGateway(),
        fallbackMask(),
        fallbackDns()
    );

    result_.staticFallback =
        configured ? TestStatus::Pass : TestStatus::Fail;

    result_.usingStaticFallback = configured;

    collectNetworkConfiguration();

    if (!configured || !validIpConfiguration())
    {
        fail(
            ErrorCode::E04_INVALID_NETWORK_CONFIG,
            "STATIC CONFIG INVALID"
        );

        return;
    }

    transitionTo(TestState::GatewayTest);

#else

    result_.staticFallback = TestStatus::Skipped;

    complete();

#endif
}

void NetworkTesterEngine::updateGatewayTest()
{
    result_.gateway = TestStatus::Running;

    if (displayReady_)
    {
        display_.showMessage(
            "GATEWAY TEST",
            "TCP REACHABILITY",
            "NOT ICMP PING"
        );
    }

    if (!validIpConfiguration())
    {
        result_.gateway = TestStatus::Skipped;
        transitionTo(TestState::DnsTest);
        return;
    }

    uint32_t elapsedMs = 0;

    const bool reachable = backend_.tcpConnect(
        result_.gatewayIp,
        GATEWAY_TEST_TCP_PORT,
        GATEWAY_TCP_TIMEOUT_MS,
        elapsedMs
    );

    result_.gatewayElapsedMs = elapsedMs;

    if (reachable)
    {
        result_.gateway = TestStatus::Pass;
    }
    else
    {
        result_.gateway = TestStatus::Warning;

        if (result_.error == ErrorCode::None ||
            result_.error == ErrorCode::E03_DHCP_TIMEOUT)
        {
            result_.error = ErrorCode::E05_GATEWAY_UNREACHABLE;
        }
    }

    if (displayReady_)
    {
        display_.showGateway(result_);
    }

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[GW] TCP reachability: "));
    Serial.println(reachable ? F("PASS") : F("FAIL"));
#endif

    transitionTo(TestState::DnsTest);
}

void NetworkTesterEngine::updateDnsTest()
{
#if ENABLE_DNS_TEST

    result_.dns = TestStatus::Running;

    if (displayReady_)
    {
        display_.showMessage(
            "DNS TEST",
            "RESOLVING HOST",
            DNS_TEST_HOST
        );
    }

    const uint32_t started = millis();

    IPAddress resolved;

    const bool resolvedOk = backend_.resolveDns(
        DNS_TEST_HOST,
        resolved,
        TCP_CONNECT_TIMEOUT_MS
    );

    result_.dnsElapsedMs = millis() - started;

    if (resolvedOk)
    {
        result_.dns = TestStatus::Pass;
        result_.dnsResolvedIp = resolved;
    }
    else
    {
        result_.dns = TestStatus::Warning;

        if (result_.error == ErrorCode::None ||
            result_.error == ErrorCode::E03_DHCP_TIMEOUT ||
            result_.error == ErrorCode::E05_GATEWAY_UNREACHABLE)
        {
            result_.error = ErrorCode::E06_DNS_FAILED;
        }
    }

    if (displayReady_)
    {
        display_.showDns(result_);
    }

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[DNS] "));
    Serial.println(resolvedOk ? F("PASS") : F("FAIL"));
#endif

    transitionTo(TestState::TcpTest);

#else

    result_.dns = TestStatus::Skipped;
    transitionTo(TestState::TcpTest);

#endif
}

void NetworkTesterEngine::updateTcpTest()
{
#if ENABLE_TCP_TEST

    result_.tcp = TestStatus::Running;

    if (displayReady_)
    {
        display_.showMessage(
            "TCP TEST",
            "CONNECTING",
            TCP_TEST_HOST
        );
    }

    uint32_t elapsedMs = 0;

    const bool connected = backend_.tcpConnect(
        TCP_TEST_HOST,
        TCP_TEST_PORT,
        TCP_CONNECT_TIMEOUT_MS,
        elapsedMs
    );

    result_.tcpElapsedMs = elapsedMs;
    result_.tcp = connected ? TestStatus::Pass : TestStatus::Warning;

    if (!connected)
    {
        if (result_.error == ErrorCode::None ||
            result_.error == ErrorCode::E03_DHCP_TIMEOUT ||
            result_.error == ErrorCode::E05_GATEWAY_UNREACHABLE ||
            result_.error == ErrorCode::E06_DNS_FAILED)
        {
            result_.error = ErrorCode::E07_TCP_FAILED;
        }
    }

    if (displayReady_)
    {
        display_.showTcp(result_);
    }

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[TCP] "));
    Serial.println(connected ? F("PASS") : F("FAIL"));
#endif

    transitionTo(TestState::ScanPrepare);

#else

    result_.tcp = TestStatus::Skipped;
    transitionTo(TestState::ScanPrepare);

#endif
}

void NetworkTesterEngine::updateScanPrepare()
{
#if ENABLE_NETWORK_SCAN

    if (displayReady_)
    {
        display_.showMessage(
            "LAN SCAN",
            "PREPARING",
            "TCP PROBE ONLY"
        );
    }

    if (!validIpConfiguration())
    {
        result_.scan = TestStatus::Skipped;
        complete();
        return;
    }

    if (!scanner_.prepare(result_.localIp, result_.subnetMask))
    {
        result_.scan = TestStatus::Skipped;

#if ENABLE_SERIAL_DEBUG
        Serial.println(F("[SCAN] Skipped: network larger than /24 or invalid."));
#endif

        complete();
        return;
    }

    result_.scan = TestStatus::Running;

#if ENABLE_SERIAL_DEBUG
    Serial.println(F("[SCAN] Started bounded TCP discovery."));
#endif

    transitionTo(TestState::ScanRunning);

#else

    result_.scan = TestStatus::Skipped;
    complete();

#endif
}

void NetworkTesterEngine::updateScanRunning()
{
    if (displayReady_)
    {
        display_.showScan(result_);
    }

    scanner_.update(result_);

    if (!scanner_.isFinished())
    {
        return;
    }

    if (scanner_.wasSkipped())
    {
        result_.scan = TestStatus::Skipped;
    }
    else
    {
        result_.scan = TestStatus::Pass;
    }

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[SCAN] Completed. Attempts: "));
    Serial.print(result_.scanAttempted);
    Serial.print(F(", responses: "));
    Serial.println(result_.scanResponded);
#endif

    complete();
}

void NetworkTesterEngine::collectNetworkConfiguration()
{
    result_.localIp = backend_.localIP();
    result_.subnetMask = backend_.subnetMask();
    result_.gatewayIp = backend_.gatewayIP();
    result_.dnsIp = backend_.dnsIP();

    result_.hasValidIPv4Config = validIpConfiguration();

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[NET] IP: "));
    Serial.println(result_.localIp);

    Serial.print(F("[NET] MASK: "));
    Serial.println(result_.subnetMask);

    Serial.print(F("[NET] GW: "));
    Serial.println(result_.gatewayIp);

    Serial.print(F("[NET] DNS: "));
    Serial.println(result_.dnsIp);
#endif
}

void NetworkTesterEngine::fail(
    ErrorCode error,
    const char* detail
)
{
    result_.error = error;

    strncpy(
        result_.detail,
        detail,
        sizeof(result_.detail) - 1
    );

    result_.detail[sizeof(result_.detail) - 1] = '\0';

    transitionTo(TestState::Error);

    if (displayReady_)
    {
        display_.showMessage(
            "NETWORK ERROR",
            errorCodeToString(error),
            detail
        );
    }

#if ENABLE_SERIAL_DEBUG
    Serial.print(F("[ERROR] "));
    Serial.print(errorCodeToString(error));
    Serial.print(F(": "));
    Serial.println(detail);
#endif

    logResult();
}

void NetworkTesterEngine::complete()
{
    transitionTo(TestState::Completed);

    if (displayReady_)
    {
        display_.showFinal(result_);
    }

    logResult();
}

bool NetworkTesterEngine::validIpConfiguration() const
{
    const bool ipValid =
        result_.localIp[0] != 0 ||
        result_.localIp[1] != 0 ||
        result_.localIp[2] != 0 ||
        result_.localIp[3] != 0;

    const bool maskValid =
        result_.subnetMask[0] != 0 ||
        result_.subnetMask[1] != 0 ||
        result_.subnetMask[2] != 0 ||
        result_.subnetMask[3] != 0;

    return ipValid && maskValid;
}

ResultClassification NetworkTesterEngine::classification() const
{
    if (result_.error == ErrorCode::E09_CANCELLED)
    {
        return ResultClassification::Cancelled;
    }

    if (!result_.w5500Detected || !result_.linkUp)
    {
        return ResultClassification::Fail;
    }

    if (!result_.hasValidIPv4Config)
    {
        return ResultClassification::Warning;
    }

    if (result_.gateway != TestStatus::Pass)
    {
        return ResultClassification::Warning;
    }

    if (ENABLE_DNS_TEST && result_.dns != TestStatus::Pass)
    {
        return ResultClassification::Warning;
    }

    if (ENABLE_TCP_TEST && result_.tcp != TestStatus::Pass)
    {
        return ResultClassification::Warning;
    }

    return ResultClassification::Pass;
}

void NetworkTesterEngine::logResult() const
{
#if ENABLE_SERIAL_DEBUG

    Serial.println();
    Serial.println(F("========== TEST RESULT =========="));

    Serial.print(F("W5500 : "));
    Serial.println(testStatusText(result_.w5500));

    Serial.print(F("LINK  : "));
    Serial.println(testStatusText(result_.link));

    Serial.print(F("DHCP  : "));
    Serial.println(testStatusText(result_.dhcp));

    Serial.print(F("STATIC: "));
    Serial.println(testStatusText(result_.staticFallback));

    Serial.print(F("GW    : "));
    Serial.println(testStatusText(result_.gateway));

    Serial.print(F("DNS   : "));
    Serial.println(testStatusText(result_.dns));

    Serial.print(F("TCP   : "));
    Serial.println(testStatusText(result_.tcp));

    Serial.print(F("SCAN  : "));
    Serial.println(testStatusText(result_.scan));

    Serial.print(F("RESULT: "));
    Serial.println(classificationText(classification()));

    if (result_.error != ErrorCode::None)
    {
        Serial.print(F("ERROR : "));
        Serial.println(errorCodeToString(result_.error));
    }

    Serial.println(F("================================="));
    Serial.println();

#endif
}