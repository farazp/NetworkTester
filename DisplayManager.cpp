#include "DisplayManager.h"

#include "Config.h"
#include "HardwareConfig.h"

DisplayManager::DisplayManager()
    : display_(OLED_WIDTH, OLED_HEIGHT, &Wire, -1)
{
}

bool DisplayManager::begin()
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (!display_.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
    {
        return false;
    }

    display_.clearDisplay();
    display_.setTextColor(SSD1306_WHITE);
    display_.setTextSize(1);
    display_.setCursor(0, 0);
    display_.println(F("NETWORK TESTER"));
    display_.println(F("DISPLAY READY"));
    display_.display();

    return true;
}

void DisplayManager::clearAndHeader(const char* title)
{
    display_.clearDisplay();
    display_.setTextColor(SSD1306_WHITE);
    display_.setTextSize(1);
    display_.setCursor(0, 0);
    display_.println(title);
    display_.drawLine(0, 10, OLED_WIDTH - 1, 10, SSD1306_WHITE);
}

void DisplayManager::fitPrint(uint8_t x, uint8_t y, const char* text)
{
    display_.setCursor(x, y);
    display_.print(text);
}

void DisplayManager::printIpLine(
    const char* label,
    const IPAddress& ip,
    uint8_t y
)
{
    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s%u.%u.%u.%u",
        label,
        ip[0],
        ip[1],
        ip[2],
        ip[3]
    );

    fitPrint(0, y, buffer);
}

void DisplayManager::printStatusLine(
    const char* label,
    TestStatus status,
    uint8_t y
)
{
    char buffer[26];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        label,
        testStatusText(status)
    );

    fitPrint(0, y, buffer);
}

void DisplayManager::showIdle()
{
    currentPage_ = DisplayPage::Idle;

    clearAndHeader("NETWORK TESTER");

    fitPrint(0, 18, "LINK: CHECKING");
    fitPrint(0, 34, "PRESS TO START");
    fitPrint(0, 50, "HOLD: CONFIG");

    display_.display();
}

void DisplayManager::showPhy(const TestResult& result)
{
    currentPage_ = DisplayPage::Phy;

    clearAndHeader("PHY STATUS");

    char buffer[28];

    snprintf(
        buffer,
        sizeof(buffer),
        "LINK  : %s",
        result.phy.linkUp ? "UP" : "DOWN"
    );
    fitPrint(0, 16, buffer);

    if (result.phy.speedValid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "SPEED : %uM",
            result.phy.speedMbps
        );
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "SPEED : N/A");
    }

    fitPrint(0, 28, buffer);

    if (result.phy.duplexValid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "DUPLEX: %s",
            result.phy.fullDuplex ? "FULL" : "HALF"
        );
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "DUPLEX: N/A");
    }

    fitPrint(0, 40, buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "MODE  : %s",
        result.phy.modeValid ? result.phy.mode : "N/A"
    );

    fitPrint(0, 52, buffer);

    display_.display();
}

void DisplayManager::showDhcp(const TestResult& result)
{
    currentPage_ = DisplayPage::Dhcp;

    clearAndHeader("DHCP TEST");

    printStatusLine("STATUS", result.dhcp, 16);

    if (result.usingStaticFallback)
    {
        fitPrint(0, 28, "STATIC TEST MODE");
    }
    else
    {
        printIpLine("IP: ", result.localIp, 28);
    }

    printIpLine("GW: ", result.gatewayIp, 40);

    char buffer[28];

    snprintf(
        buffer,
        sizeof(buffer),
        "TIME: %lums",
        static_cast<unsigned long>(result.dhcpElapsedMs)
    );

    fitPrint(0, 52, buffer);

    display_.display();
}

void DisplayManager::showGateway(const TestResult& result)
{
    currentPage_ = DisplayPage::Gateway;

    clearAndHeader("GATEWAY TEST");

    fitPrint(0, 16, "METHOD: TCP REACH");
    printStatusLine("STATUS", result.gateway, 28);
    printIpLine("GW: ", result.gatewayIp, 40);

    char buffer[28];

    snprintf(
        buffer,
        sizeof(buffer),
        "TIME: %lums",
        static_cast<unsigned long>(result.gatewayElapsedMs)
    );

    fitPrint(0, 52, buffer);

    display_.display();
}

void DisplayManager::showDns(const TestResult& result)
{
    currentPage_ = DisplayPage::Dns;

    clearAndHeader("DNS TEST");

    fitPrint(0, 16, "HOST: example.com");
    printStatusLine("STATUS", result.dns, 28);

    if (result.dns == TestStatus::Pass)
    {
        printIpLine("IP: ", result.dnsResolvedIp, 40);
    }
    else
    {
        fitPrint(0, 40, "IP: N/A");
    }

    display_.display();
}

void DisplayManager::showTcp(const TestResult& result)
{
    currentPage_ = DisplayPage::Tcp;

    clearAndHeader("TCP TEST");

    fitPrint(0, 16, "HOST: example.com");

    char portBuffer[25];
    snprintf(portBuffer, sizeof(portBuffer), "PORT: %u", TCP_TEST_PORT);
    fitPrint(0, 28, portBuffer);

    printStatusLine("STATUS", result.tcp, 40);

    char timeBuffer[28];
    snprintf(
        timeBuffer,
        sizeof(timeBuffer),
        "TIME: %lums",
        static_cast<unsigned long>(result.tcpElapsedMs)
    );

    fitPrint(0, 52, timeBuffer);

    display_.display();
}

void DisplayManager::showScan(const TestResult& result)
{
    currentPage_ = DisplayPage::Scan;

    clearAndHeader("LAN SCAN");

    printStatusLine("STATUS", result.scan, 16);

    char buffer[30];

    snprintf(
        buffer,
        sizeof(buffer),
        "TRY: %u  HIT: %u",
        result.scanAttempted,
        result.scanResponded
    );

    fitPrint(0, 28, buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "SAVED: %u/%u",
        result.discoveredCount,
        MAX_DISCOVERED_HOSTS
    );

    fitPrint(0, 40, buffer);

    if (result.scan == TestStatus::Skipped)
    {
        fitPrint(0, 52, "NETWORK TOO LARGE");
    }
    else
    {
        fitPrint(0, 52, "TCP PROBE ONLY");
    }

    display_.display();
}

void DisplayManager::showFinal(const TestResult& result)
{
    currentPage_ = DisplayPage::Final;

    clearAndHeader("NETWORK RESULT");

    printStatusLine("LINK", result.link, 16);
    printStatusLine("DHCP", result.dhcp, 26);
    printStatusLine("GW", result.gateway, 36);
    printStatusLine("DNS", result.dns, 46);

    char buffer[30];

    ResultClassification classification = ResultClassification::Unknown;

    if (result.error == ErrorCode::E09_CANCELLED)
    {
        classification = ResultClassification::Cancelled;
    }
    else if (!result.w5500Detected || !result.linkUp)
    {
        classification = ResultClassification::Fail;
    }
    else if (
        !result.hasValidIPv4Config ||
        result.gateway != TestStatus::Pass ||
        result.dns == TestStatus::Fail ||
        result.tcp == TestStatus::Fail
    )
    {
        classification = ResultClassification::Warning;
    }
    else
    {
        classification = ResultClassification::Pass;
    }

    snprintf(
        buffer,
        sizeof(buffer),
        "RESULT: %s",
        classificationText(classification)
    );

    fitPrint(0, 56, buffer);

    display_.display();
}

void DisplayManager::showMessage(
    const char* title,
    const char* line1,
    const char* line2,
    const char* line3
)
{
    currentPage_ = DisplayPage::Message;

    clearAndHeader(title);

    if (line1 != nullptr)
    {
        fitPrint(0, 18, line1);
    }

    if (line2 != nullptr)
    {
        fitPrint(0, 34, line2);
    }

    if (line3 != nullptr)
    {
        fitPrint(0, 50, line3);
    }

    display_.display();
}

void DisplayManager::showPage(
    DisplayPage page,
    const TestResult& result
)
{
    switch (page)
    {
        case DisplayPage::Idle:
            showIdle();
            break;

        case DisplayPage::Phy:
            showPhy(result);
            break;

        case DisplayPage::Dhcp:
            showDhcp(result);
            break;

        case DisplayPage::Gateway:
            showGateway(result);
            break;

        case DisplayPage::Dns:
            showDns(result);
            break;

        case DisplayPage::Tcp:
            showTcp(result);
            break;

        case DisplayPage::Scan:
            showScan(result);
            break;

        case DisplayPage::Final:
            showFinal(result);
            break;

        default:
            showFinal(result);
            break;
    }
}

void DisplayManager::nextPage(const TestResult& result)
{
    uint8_t value = static_cast<uint8_t>(currentPage_);

    value++;

    if (value > static_cast<uint8_t>(DisplayPage::Final))
    {
        value = static_cast<uint8_t>(DisplayPage::Phy);
    }

    showPage(static_cast<DisplayPage>(value), result);
}

DisplayPage DisplayManager::currentPage() const
{
    return currentPage_;
}