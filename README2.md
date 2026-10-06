# NetworkTester

Portable Ethernet Network Tester for:

- ESP8266 NodeMCU / ESP-12E / ESP-12F
- ESP32-C3 SuperMini
- WIZnet W5500 Ethernet controller
- SSD1306 I2C OLED 128x64
- One push button

---

## Scope

This tester verifies Ethernet network functionality at PHY and IP layers.

It can test:

- W5500 SPI communication
- W5500 presence
- Ethernet PHY link
- Link speed: 10 Mbps / 100 Mbps
- Duplex: Half / Full
- PHY operation mode
- DHCP availability
- IPv4 configuration
- Static fallback test mode
- Gateway TCP reachability
- DNS-related TCP resolution behavior
- TCP connectivity
- Limited TCP-based LAN discovery

It does not certify cables.

---

## Cable-Test Limitations

W5500 cannot test the following without external measurement hardware:

- Individual pair map
- Pair open
- Pair short
- Split pair
- Miswire
- Cable length
- Distance to fault
- TDR
- Fluke-style cable certification
- PoE voltage detection
- PoE class detection
- Managed switch port identity
- Guaranteed host MAC/vendor/hostname discovery

For real copper diagnostics, add one of these in a future revision:

- Dedicated PHY with cable diagnostic support
- External TDR hardware
- Remote cable terminator
- External `CableTesterModule`
- Dedicated pair continuity module

Do not interpret Link UP as a certified cable result.

---

## Required Libraries

### Shared

Install with Arduino Library Manager:

- Adafruit GFX Library
- Adafruit SSD1306

### ESP8266

Install:

- ESP8266 by ESP8266 Community
- Recommended core version: 3.1.x or later

The project uses the ESP8266 core W5500 lwIP integration:

```cpp
#include <W5500lwIP.h>
```

Do not install and use a second W5500 Ethernet library simultaneously unless you intentionally modify the backend.

### ESP32-C3

Install:

- esp32 by Espressif Systems
- Arduino Ethernet library
- Adafruit GFX Library
- Adafruit SSD1306

The ESP32-C3 backend currently uses:

```cpp
#include <Ethernet.h>
```

The selected Arduino Ethernet version must support W5500.

---

## ESP8266 Wiring

| Device Signal | NodeMCU Pin | ESP8266 GPIO | Notes |
|---|---:|---:|---|
| W5500 SCK | D5 | GPIO14 | HSPI Clock |
| W5500 MISO | D6 | GPIO12 | HSPI MISO |
| W5500 MOSI | D7 | GPIO13 | HSPI MOSI |
| W5500 CS | D0 | GPIO16 | Chip select |
| W5500 RST | D1 | GPIO5 | Active-low reset |
| W5500 INT | D2 | GPIO4 | Optional |
| OLED SDA | D3 | GPIO0 | Boot-sensitive pin |
| OLED SCL | D4 | GPIO2 | Boot-sensitive pin |
| Button | RX | GPIO3 | Button to GND |
| 3.3V | 3V3 | - | Verify module requirements |
| GND | GND | - | Common ground |

### ESP8266 boot warnings

GPIO0 and GPIO2 are boot strap pins.

- GPIO0 must be HIGH for normal flash boot.
- GPIO2 must be HIGH for normal flash boot.
- GPIO15 must be LOW for normal boot.
- Do not use GPIO6 through GPIO11 because they are used by flash.
- Verify OLED pull-ups do not pull GPIO0/GPIO2 LOW during reset.
- GPIO3 is UART RX. Serial input is unavailable while this pin is used as button input.

---

## ESP32-C3 Wiring

| Device Signal | ESP32-C3 GPIO | Notes |
|---|---:|---|
| W5500 SCK | GPIO4 | SPI clock |
| W5500 MISO | GPIO5 | SPI input |
| W5500 MOSI | GPIO6 | SPI output |
| W5500 CS | GPIO7 | Chip select |
| W5500 RST | GPIO3 | Active-low reset |
| W5500 INT | GPIO10 | Optional |
| OLED SDA | GPIO8 | Verify board revision |
| OLED SCL | GPIO9 | Verify board revision |
| Button | GPIO2 | Button to GND |
| 3.3V | 3V3 | Verify regulator capacity |
| GND | GND | Common ground |

### ESP32-C3 warning

ESP32-C3 SuperMini board revisions differ.

Before wiring:

- Verify board silkscreen.
- Verify USB wiring on your board.
- Verify boot strapping pins.
- Verify GPIO8/GPIO9 are usable on your board revision.
- Do not assume every SuperMini clone exposes identical GPIO functions.

---

## Electrical Requirements

- ESP8266 and ESP32-C3 are 3.3V logic devices.
- W5500 SPI logic must be 3.3V compatible.
- Do not connect 5V MISO directly to ESP8266/ESP32-C3.
- Do not assume every W5500 module has a regulator.
- Do not assume every W5500 module includes level shifting.
- Use common GND between MCU, OLED, and W5500.
- Keep SPI wires short.
- Use a stable power supply.
- Change the configured MAC address for every tester.

---

## Build Steps

1. Create a folder named `NetworkTester`.
2. Put all `.ino`, `.h`, and `.cpp` files in that folder.
3. Install required board package and libraries.
4. Open `NetworkTester.ino`.
5. Open `Config.h`.
6. Change `DEVICE_MAC` to a unique address.
7. Adjust fallback static IP configuration if needed.
8. Select board:
   - NodeMCU 1.0 (ESP-12E Module) for ESP8266
   - ESP32C3 Dev Module or matching SuperMini target for ESP32-C3
9. Set Serial Monitor to 115200 baud.
10. Compile and upload.

---

## Button Operation

### Idle state

- Short press: start full network test
- Long press: show configuration placeholder

### During test

- Short press: switch OLED information page
- Long press: cancel test safely

No delay-based debounce is used.

---

## Test Sequence

1. Initialize OLED, button, SPI, and W5500.
2. Verify W5500 SPI communication.
3. Read W5500 PHY status.
4. Verify Ethernet link.
5. Attempt DHCP.
6. Collect IP, subnet, gateway, and DNS values.
7. Optionally configure static fallback.
8. Test gateway TCP reachability.
9. Test DNS/TCP resolution behavior.
10. Test configurable TCP endpoint.
11. Optionally scan local subnet with TCP probes.
12. Show final result.

---

## Gateway Test

The default gateway test is not ICMP ping.

It is shown as:

```text
METHOD: TCP REACH
```

The firmware attempts a TCP connection to:

```cpp
GATEWAY_TEST_TCP_PORT
```

Default port is 80.

A failure means the selected TCP port did not respond.

It does not conclusively prove that the gateway is unreachable:

- Gateway may block the selected port.
- Gateway may permit ICMP but reject TCP.
- Firewall may block access.
- Port 80 may not be open.

---

## DNS Test

The project separates:

1. DNS-related hostname connection behavior
2. TCP connectivity

On constrained Arduino networking stacks, a hostname TCP connection generally triggers DNS resolution through the underlying stack.

A successful TCP connection to a hostname confirms:

- A name was resolved sufficiently for the stack to connect.
- TCP connectivity to that host/port succeeded.

A failed TCP connection does not always distinguish between:

- DNS failure
- Routing failure
- Firewall block
- Remote server outage
- Closed TCP port

For strict standalone DNS UDP query testing, add a dedicated DNS client implementation for the selected stack.

---

## Scan Behavior

The LAN scanner:

- Runs only on `/24` or smaller networks.
- Skips `/16`, `/20`, and all subnets larger than `/24`.
- Uses a configurable TCP port probe.
- Does not allocate memory per target IP.
- Stores only a small fixed-size result list.
- Does not claim that no TCP response means no host exists.
- Does not display MAC addresses.
- Does not display vendor/OUI names.
- Does not display hostnames unless a real hostname-resolution protocol is added.

---

## MACRAW / ARP / LLDP

MACRAW is disabled by default:

```cpp
#define ENABLE_MACRAW_DIAGNOSTICS false
```

LLDP is disabled by default:

```cpp
#define ENABLE_LLDP_TEST false
```

Reasons:

- W5500 MACRAW uses Socket 0.
- Raw frame handling requires a custom Ethernet frame parser.
- High-level TCP/IP usage can conflict with raw socket handling.
- Standard Arduino Ethernet APIs do not expose a portable ARP cache lookup API.
- LLDP uses Ethernet type `0x88CC`.
- CDP is Cisco proprietary and is not LLDP.
- Switch identity cannot be claimed without actually receiving and parsing LLDP/CDP frames.

Without raw Ethernet support:

```text
RAW ETH: NOT AVAILABLE
LLDP: NOT AVAILABLE
```

---

## Error Codes

| Code | Meaning | Type |
|---|---|---|
| E01 | W5500 initialization failed | Fatal |
| E02 | Link is down | Fatal |
| E03 | DHCP timeout | Warning |
| E04 | Invalid IPv4 configuration | Warning |
| E05 | Gateway TCP reachability failed | Warning |
| E06 | DNS test failed | Warning |
| E07 | TCP test failed | Warning |
| E08 | Network scan failed | Warning |
| E09 | Test cancelled | Nonfatal |
| E10 | Memory limit | Warning/Fatal |
| E11 | Raw Ethernet unavailable | Informational |

---

## Final Result Rules

### FAIL

- W5500 not detected
- W5500 SPI initialization fails
- Ethernet link is down

### WARNING

- Link is active but DHCP fails
- Static fallback is active
- Gateway TCP probe fails
- DNS-related connection test fails
- TCP endpoint fails
- Scan skipped due to subnet larger than `/24`

### PASS

- W5500 communication works
- Link is UP
- DHCP succeeds or static configuration is valid
- Gateway TCP probe succeeds
- DNS-related hostname test succeeds
- TCP test succeeds

---

## Troubleshooting

### E01 W5500 INIT FAILED

Check:

- W5500 power supply
-