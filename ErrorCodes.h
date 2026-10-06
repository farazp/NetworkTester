#pragma once

#include <Arduino.h>

enum class ErrorCode : uint8_t
{
    None = 0,
    E01_W5500_INIT_FAILED,
    E02_LINK_DOWN,
    E03_DHCP_TIMEOUT,
    E04_INVALID_NETWORK_CONFIG,
    E05_GATEWAY_UNREACHABLE,
    E06_DNS_FAILED,
    E07_TCP_FAILED,
    E08_SCAN_FAILED,
    E09_CANCELLED,
    E10_MEMORY_LIMIT,
    E11_RAW_ETHERNET_UNAVAILABLE
};

inline const char* errorCodeToString(ErrorCode code)
{
    switch (code)
    {
        case ErrorCode::None:
            return "NONE";

        case ErrorCode::E01_W5500_INIT_FAILED:
            return "E01 W5500 INIT";

        case ErrorCode::E02_LINK_DOWN:
            return "E02 LINK DOWN";

        case ErrorCode::E03_DHCP_TIMEOUT:
            return "E03 DHCP TIMEOUT";

        case ErrorCode::E04_INVALID_NETWORK_CONFIG:
            return "E04 INVALID IPV4";

        case ErrorCode::E05_GATEWAY_UNREACHABLE:
            return "E05 GW UNREACH";

        case ErrorCode::E06_DNS_FAILED:
            return "E06 DNS FAILED";

        case ErrorCode::E07_TCP_FAILED:
            return "E07 TCP FAILED";

        case ErrorCode::E08_SCAN_FAILED:
            return "E08 SCAN FAILED";

        case ErrorCode::E09_CANCELLED:
            return "E09 CANCELLED";

        case ErrorCode::E10_MEMORY_LIMIT:
            return "E10 MEMORY LIMIT";

        case ErrorCode::E11_RAW_ETHERNET_UNAVAILABLE:
            return "E11 RAW ETH N/A";

        default:
            return "UNKNOWN";
    }
}

inline bool isFatalError(ErrorCode code)
{
    return code == ErrorCode::E01_W5500_INIT_FAILED ||
           code == ErrorCode::E02_LINK_DOWN;
}