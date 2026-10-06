# Portable Ethernet Network Tester

A portable Ethernet network tester based on:

- ESP8266 NodeMCU / ESP-12E / ESP-12F
- Or ESP32-C3 SuperMini
- WIZnet W5500 Ethernet controller
- SSD1306 128x64 I2C OLED
- One push button

## Supported Tests

The tester performs:

1. W5500 SPI communication check
2. W5500 PHY link status
3. PHY speed: 10 Mbps / 100 Mbps
4. PHY duplex: half/full
5. PHY operation mode
6. DHCP acquisition
7. IPv4 configuration display
8. Static fallback test mode
9. Gateway TCP reachability probe
10. DNS resolution
11. TCP connection test
12. Optional bounded LAN TCP scan

## Not Supported

The W5500 alone cannot test:

- Pair map
- Pair opens
- Pair shorts
- Split pairs
- Cable miswire
- Cable length
- Distance to cable fault
- TDR
- Cable certification
- PoE voltage or class
- Guaranteed switch port identity
- Guaranteed hostname/vendor identity

Do not interpret this device as a cable certifier.

## ESP8266 Wiring

| Function | NodeMCU Pin | GPIO | W5500/OLED |
|---|---:|---:|---|
| W5500 SCK | D5 | GPIO14 | SCK |
| W5500 MISO | D6 | GPIO12 | MISO |
| W5500 MOSI | D7 | GPIO13 | MOSI |
| W5500 CS | D0 | GPIO16 | CS |
| W5500 Reset | D1 | GPIO5 | RST |
| W5500 INT | D2 | GPIO4 | INT optional |
| OLED SDA | D3 | GPIO0 | SDA |
| OLED SCL | D4 | GPIO2 | SCL |
| Button | RX | GPIO3 | Button to GND |
| Power | 3V3 | - | W5500/OLED VCC |
| Ground | GND | - | All GND |

### ESP8266 boot warning

GPIO0 and GPIO2 are boot strap pins.

- GPIO0 must remain high for normal boot.
- GPIO2 must remain high for normal boot.
- Verify your OLED does not pull them low at power-up.
- Do not use GPIO6 through GPIO11 because they are connected to flash.

## ESP32-C3 Wiring

| Function | ESP32-C3 GPIO | W5500/OLED |
|---|---:|---|
| W5500 SCK | GPIO4 | SCK |
| W5500 MISO | GPIO5 | MISO |
| W5500 MOSI | GPIO6 | MOSI |
| W5500 CS | GPIO7 | CS |
| W5500 Reset | GPIO3 | RST |
| W5500 INT | GPIO10 | INT optional |
| OLED SDA | GPIO8 | SDA |
| OLED SCL | GPIO9 | SCL |
| Button | GPIO2 | Button to GND |
| Power | 3V3 | W5500/OLED VCC |
| Ground | GND | All GND |

ESP32-C3 SuperMini pin assignments differ between board revisions.
Verify the exact board layout before wiring.

## Electrical Safety

- ESP8266 and ESP32-C3 GPIO are 3.3 V only.
- Verify whether your W5500 module includes a 3.3 V regulator.
- Do not assume a W5500 module is 5 V logic tolerant.
- Use a stable power supply.
- Keep SPI wires short.
- Connect all grounds together.
- Use a unique locally administered MAC address.

## Required Libraries

### ESP8266

- ESP8266 Arduino Core 3.x or later
- Adafruit GFX Library
- Adafruit SSD1306

The ESP8266 W5500 backend uses `W5500lwIP.h`.

### ESP32-C3

- ESP32 Arduino Core
- Arduino Ethernet library
- Adafruit GFX Library
- Adafruit SSD1306

## Build Instructions

1. Create a folder named `NetworkTester`.
2. Copy all source files into that folder.
3. Install the required board package and libraries.
4. Open `NetworkTester.ino`.
5. Edit `Config.h`.
6. Set a unique MAC address.
7. Select your board.
8. Compile and upload.
9. Open serial monitor at 115200 baud.

## Configuration

Edit `Config.h` to change:

- DHCP timeout
- DNS test host
- TCP test host and port
- Gateway TCP test port
- Static fallback address
- Scan enable/disable
- Scan host limit
- OLED I2C address
- MAC address

## Button Behavior

### Idle

- Short press: start full automatic test
- Long press: display configuration placeholder

### During a test

- Short press: switch OLED page
- Long press: cancel test safely

## Gateway Test

By default, the gateway test is a TCP reachability probe.

It is not ICMP ping.

A failed result means:

- The tested TCP port did not respond.
- The gateway may still be reachable through ICMP.
- The gateway may block the selected TCP port.

The display labels it as:

`METHOD: TCP REACH`

## LAN Scan

The scan is a limited TCP probe:

- Only scans networks /24 or smaller.
- Skips /16 and other large networks.
- Does not use raw ARP.
- Does not claim a host does not exist when TCP fails.
- Does not display MAC address, hostname, or vendor unless a real protocol implementation is added.

## MACRAW / ARP / LLDP

MACRAW is disabled by default.

Reasons:

- W5500 MACRAW uses Socket 0.
- Raw Ethernet frame processing requires custom parser code.
- ARP table export is not generally available in high-level libraries.
- LLDP requires receiving raw Ethernet frames with EtherType 0x88CC.
- CDP is Cisco proprietary and is not LLDP.
- Raw Ethernet logic can conflict with high-level TCP/IP networking.

## Result Rules

### FAIL

- W5500 is not detected
- SPI initialization fails
- Ethernet PHY link is down

### WARNING

- Link works but DHCP fails
- Static fallback is required
- Gateway TCP reachability fails
- DNS fails
- TCP connectivity fails
- Scan is skipped because network is too large

### PASS

- W5500 communication works
- Physical link is up
- DHCP or valid static configuration is active
- Gateway TCP reachability succeeds
- DNS succeeds
- TCP connectivity succeeds

## Troubleshooting

### E01 W5500 INIT

Check:

- CS pin
- SCK/MISO/MOSI wiring
- 3.3 V supply
- W5500 reset
- Common ground
- SPI clock speed

### E02 LINK DOWN

Check:

- Ethernet cable
- RJ45 connector
- Switch port
- Link LEDs
- W5500 module power

### E03 DHCP TIMEOUT

Possible causes:

- No DHCP server
- Switch port isolation
- VLAN mismatch
- MAC filtering
- 802.1X access control
- DHCP server unavailable

### E05 GW UNREACH

The gateway TCP port did not respond.
This does not prove the gateway is physically unreachable.

### E06 DNS FAILED

Possible causes:

- Invalid DNS server
- No route to DNS server
- DNS blocked
- Captive portal
- Isolated VLAN

### E07 TCP FAILED

Possible causes:

- Internet blocked
- Remote host unavailable
- Port blocked
- Firewall policy
- DNS resolved but route/connectivity failed