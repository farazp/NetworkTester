#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "TestResult.h"

enum class DisplayPage : uint8_t
{
    Idle,
    Phy,
    Dhcp,
    Gateway,
    Dns,
    Tcp,
    Scan,
    Final,
    Message
};

class DisplayManager
{
public:
    DisplayManager();

    bool begin();

    void showIdle();
    void showPhy(const TestResult& result);
    void showDhcp(const TestResult& result);
    void showGateway(const TestResult& result);
    void showDns(const TestResult& result);
    void showTcp(const TestResult& result);
    void showScan(const TestResult& result);
    void showFinal(const TestResult& result);

    void showMessage(
        const char* title,
        const char* line1,
        const char* line2 = nullptr,
        const char* line3 = nullptr
    );

    void showPage(DisplayPage page, const TestResult& result);
    void nextPage(const TestResult& result);

    DisplayPage currentPage() const;

private:
    Adafruit_SSD1306 display_;
    DisplayPage currentPage_ = DisplayPage::Idle;

    void clearAndHeader(const char* title);
    void printIpLine(const char* label, const IPAddress& ip, uint8_t y);
    void printStatusLine(const char* label, TestStatus status, uint8_t y);
    void fitPrint(uint8_t x, uint8_t y, const char* text);
};