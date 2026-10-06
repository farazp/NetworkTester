#pragma once

#include <Arduino.h>
#include <IPAddress.h>

// ============================================================
// Feature switches
// ============================================================

#define ENABLE_NETWORK_SCAN true
#define ENABLE_DNS_TEST true
#define ENABLE_TCP_TEST true
#define ENABLE_STATIC_FALLBACK true
#define ENABLE_MACRAW_DIAGNOSTICS false
#define ENABLE_LLDP_TEST false
#define ENABLE_SERIAL_DEBUG true

// ============================================================
// Timeouts and behavior
// ============================================================

#define DHCP_TIMEOUT_MS             10000UL
#define TCP_CONNECT_TIMEOUT_MS      3000UL
#define GATEWAY_TCP_TIMEOUT_MS      1200UL

#define SCAN_TIMEOUT_MS             250UL
#define SCAN_DELAY_MS               10UL
#define SCAN_MAX_HOSTS              24
#define MAX_DISCOVERED_HOSTS        16
#define SCAN_TCP_PORT               53

#define BUTTON_DEBOUNCE_MS          35UL
#define BUTTON_LONG_PRESS_MS        1200UL

#define W5500_SPI_CLOCK_HZ          8000000UL

// ============================================================
// OLED
// ============================================================

#define OLED_WIDTH                  128
#define OLED_HEIGHT                 64
#define OLED_I2C_ADDRESS            0x3C

// ============================================================
// Remote tests
// ============================================================

// This hostname is used for a hostname-based TCP test.
// Do not treat a plain TCP connection as HTTPS validation.
#define DNS_TEST_HOST               "1.1.1.1"

#define TCP_TEST_HOST               "8.8.8.8"
#define TCP_TEST_PORT               53

// Gateway test is TCP reachability only, not ICMP ping.
#define GATEWAY_TEST_TCP_PORT       53

// ============================================================
// Device MAC address
//
// Change this address per tester.
// 0x02 marks a locally administered unicast MAC.
// ============================================================

static const uint8_t DEVICE_MAC[6] =
{
    0x02, 0x54, 0x45, 0x53, 0x54, 0x01
};

// ============================================================
// Static fallback test mode
//
// This is NOT DHCP.
// It must be valid for the actual connected network.
// ============================================================

#define FALLBACK_IP_A               192
#define FALLBACK_IP_B               168
#define FALLBACK_IP_C               1
#define FALLBACK_IP_D               250

#define FALLBACK_MASK_A             255
#define FALLBACK_MASK_B             255
#define FALLBACK_MASK_C             255
#define FALLBACK_MASK_D             0

#define FALLBACK_GATEWAY_A          192
#define FALLBACK_GATEWAY_B          168
#define FALLBACK_GATEWAY_C          1
#define FALLBACK_GATEWAY_D          1

#define FALLBACK_DNS_A              1
#define FALLBACK_DNS_B              1
#define FALLBACK_DNS_C              1
#define FALLBACK_DNS_D              1

inline IPAddress fallbackIP()
{
    return IPAddress(
        FALLBACK_IP_A,
        FALLBACK_IP_B,
        FALLBACK_IP_C,
        FALLBACK_IP_D
    );
}

inline IPAddress fallbackMask()
{
    return IPAddress(
        FALLBACK_MASK_A,
        FALLBACK_MASK_B,
        FALLBACK_MASK_C,
        FALLBACK_MASK_D
    );
}

inline IPAddress fallbackGateway()
{
    return IPAddress(
        FALLBACK_GATEWAY_A,
        FALLBACK_GATEWAY_B,
        FALLBACK_GATEWAY_C,
        FALLBACK_GATEWAY_D
    );
}

inline IPAddress fallbackDns()
{
    return IPAddress(
        FALLBACK_DNS_A,
        FALLBACK_DNS_B,
        FALLBACK_DNS_C,
        FALLBACK_DNS_D
    );
}