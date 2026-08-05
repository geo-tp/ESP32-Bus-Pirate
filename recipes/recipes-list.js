window.BIT_PIRATE_RECIPES = [
  {
    "title": "Scan an unknown I2C device",
    "slug": "scan-unknown-i2c-device",
    "description": "Use ESP32 Bit Pirate to scan an I2C bus and find connected devices.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔎",
    "mark": "I²C",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "Unknown module",
      "Sensor"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "beginner",
      "i2c",
      "debugging",
      "sensor",
      "education"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "5 min",
    "hardware": "Unknown I2C module, 4 jumper wires",
    "takeaway": "A quick address scan before you lose an hour chasing the wrong example code."
  },
  {
    "title": "Identify an unknown I2C device",
    "slug": "identify-unknown-i2c-device",
    "description": "Use scan, discovery, identify and ping to narrow down an unknown I2C address before safe register reads.",
    "date": "2026-06-29",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔎",
    "mark": "I²C",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "Unknown module",
      "Sensor",
      "EEPROM"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "beginner",
      "i2c",
      "identify",
      "discovery",
      "scan",
      "sensor",
      "debugging"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "6 min",
    "hardware": "Unknown I2C module, 4 jumper wires",
    "takeaway": "Turn an ACK address into a safer identification workflow before reading registers."
  },
  {
    "title": "Dump a SPI flash chip from your browser",
    "slug": "dump-spi-flash-browser",
    "description": "Read and save a SPI NOR flash dump with the browser SPI flash programmer workflow.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "💾",
    "mark": "SPI",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "Flash chip"
    ],
    "workflows": [
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "Web SPI Flash Programmer",
      "Web Tools"
    ],
    "tags": [
      "beginner",
      "spi",
      "flash",
      "firmware",
      "repair"
    ],
    "cover": "assets/visual-spi-flash.svg",
    "time": "10 min",
    "hardware": "SOIC clip or wired SPI flash",
    "takeaway": "Make a clean backup before a repair attempt, with the browser as the workbench."
  },
  {
    "title": "Sniff UART between two boards",
    "slug": "sniff-uart-between-two-boards",
    "description": "Tap a UART link with ESP32 Bit Pirate and inspect the serial conversation.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📡",
    "mark": "RX",
    "protocols": [
      "UART"
    ],
    "targets": [
      "UART link",
      "Two boards"
    ],
    "workflows": [
      "Sniff / capture",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "beginner",
      "uart",
      "debugging",
      "education"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "5 min",
    "hardware": "Two boards with a UART link",
    "takeaway": "Tap RX and ground first, then let the boot logs tell you what is really happening."
  },
  {
    "title": "Read an iButton ID",
    "slug": "read-ibutton-id",
    "description": "Use the 1-Wire mode to read a Dallas iButton or compatible device ID.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔑",
    "mark": "1W",
    "protocols": [
      "1-Wire"
    ],
    "targets": [
      "iButton",
      "1-Wire device"
    ],
    "workflows": [
      "Read ID",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "beginner",
      "1-wire",
      "ibutton",
      "debugging"
    ],
    "cover": "assets/visual-ibutton.svg",
    "time": "5 min",
    "hardware": "Dallas iButton, reader contacts, pull-up resistor",
    "takeaway": "Read the ROM code from a tag without writing a throwaway sketch."
  },
  {
    "title": "Use the logic analyzer with PulseView",
    "slug": "use-logic-analyzer-pulseview",
    "description": "Capture digital signals with the ESP32 Bit Pirate SUMP adapter and inspect them in PulseView.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📈",
    "mark": "LA",
    "protocols": [
      "Logic Analyzer"
    ],
    "targets": [
      "Digital signal"
    ],
    "workflows": [
      "Sniff / capture",
      "Measure"
    ],
    "tools": [
      "PulseView",
      "SUMP",
      "Web Logic Analyzer"
    ],
    "tags": [
      "beginner",
      "logic analyzer",
      "pulseview",
      "debugging",
      "education"
    ],
    "cover": "assets/visual-logic-analyzer.svg",
    "time": "10 min",
    "hardware": "One or two digital signals to capture",
    "takeaway": "Turn a suspicious pin into a waveform you can measure and share."
  },
  {
    "title": "Dump an I2C EEPROM safely",
    "slug": "dump-i2c-eeprom",
    "description": "Use the I2C EEPROM shell to probe, read and dump common 24XX / AT24C EEPROM chips.",
    "difficulty": "Beginner",
    "mark": "EE",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "I2C EEPROM",
      "24XX EEPROM"
    ],
    "workflows": [
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "beginner",
      "i2c",
      "eeprom",
      "backup",
      "repair"
    ],
    "cover": "assets/visual-i2c-eeprom.svg",
    "time": "10 min",
    "hardware": "24XX EEPROM, jumper wires, known-safe supply",
    "takeaway": "Probe the chip first, then dump it through the dedicated EEPROM shell instead of guessing register reads.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Recover a stuck I2C bus",
    "slug": "recover-stuck-i2c-bus",
    "description": "Use the I2C recover command after a target holds SDA low or the bus stops responding.",
    "difficulty": "Beginner",
    "mark": "I²C",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "I2C bus",
      "Stuck target"
    ],
    "workflows": [
      "Troubleshooting",
      "Recover"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2c",
      "troubleshooting",
      "recover",
      "debugging"
    ],
    "cover": "assets/visual-i2c-recover.svg",
    "time": "5 min",
    "hardware": "I2C target, jumper wires",
    "takeaway": "Before rewiring everything, clock the bus and send a stop condition to release a stuck target.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Read a DS18B20 temperature sensor",
    "slug": "read-ds18b20-temperature",
    "description": "Use 1-Wire mode to scan the bus and decode temperature from a DS18B20 sensor.",
    "difficulty": "Beginner",
    "mark": "1W",
    "protocols": [
      "1-Wire"
    ],
    "targets": [
      "DS18B20",
      "Temperature sensor"
    ],
    "workflows": [
      "Read sensor",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "1-wire",
      "ds18b20",
      "temperature",
      "sensor",
      "beginner"
    ],
    "cover": "assets/visual-ds18b20.svg",
    "time": "5 min",
    "hardware": "DS18B20, pull-up resistor, 3 wires",
    "takeaway": "Confirm the sensor ROM first, then let the firmware decode the temperature value.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Dump a 1-Wire EEPROM",
    "slug": "dump-one-wire-eeprom",
    "description": "Use the 1-Wire EEPROM shell to probe, read, analyze and dump DS2431 / DS2433 style devices.",
    "difficulty": "Intermediate",
    "mark": "1W",
    "protocols": [
      "1-Wire"
    ],
    "targets": [
      "1-Wire EEPROM",
      "DS2431"
    ],
    "workflows": [
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "1-wire",
      "eeprom",
      "ds2431",
      "backup",
      "repair"
    ],
    "cover": "assets/visual-1wire-eeprom.svg",
    "time": "5 min",
    "hardware": "DS2431/DS2433-style EEPROM, pull-up resistor",
    "takeaway": "Use the EEPROM shell instead of raw 1-Wire instructions when you need a reliable backup.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Measure a GPIO signal frequency",
    "slug": "measure-gpio-frequency",
    "description": "Use DIO mode to measure edges and estimate the frequency of a digital signal on a GPIO pin.",
    "difficulty": "Beginner",
    "mark": "DIO",
    "protocols": [
      "DIO"
    ],
    "targets": [
      "GPIO signal",
      "Digital line"
    ],
    "workflows": [
      "Measure",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "dio",
      "gpio",
      "frequency",
      "measure",
      "debugging"
    ],
    "cover": "assets/visual-gpio-measure.svg",
    "time": "5 min",
    "hardware": "Digital signal, jumper wire, shared ground",
    "takeaway": "Check if a pin is alive and roughly how fast it toggles before opening a full logic capture.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Generate PWM or move a servo",
    "slug": "generate-pwm-servo-signal",
    "description": "Use DIO mode to output PWM, pulse a pin, or send a simple RC servo angle command.",
    "difficulty": "Beginner",
    "mark": "PWM",
    "protocols": [
      "DIO"
    ],
    "targets": [
      "PWM output",
      "Servo"
    ],
    "workflows": [
      "Generate signal",
      "Test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "dio",
      "pwm",
      "servo",
      "gpio",
      "test"
    ],
    "cover": "assets/visual-pwm-servo.svg",
    "time": "5 min",
    "hardware": "LED, servo signal input, or test pin",
    "takeaway": "Generate a quick test signal from the bench without writing a sketch.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Capture an infrared remote signal",
    "slug": "capture-infrared-remote",
    "description": "Use Infrared mode to configure pins, receive remote control frames, and record signals for later reuse.",
    "difficulty": "Beginner",
    "mark": "IR",
    "protocols": [
      "Infrared"
    ],
    "targets": [
      "IR remote",
      "Receiver module"
    ],
    "workflows": [
      "Sniff / capture",
      "Record"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "infrared",
      "ir",
      "remote",
      "capture",
      "record"
    ],
    "cover": "assets/visual-infrared.svg",
    "time": "3 min",
    "hardware": "IR receiver module and remote control",
    "takeaway": "Point the remote, capture the protocol/data, then decide if you need to record or replay it.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Load and send a Flipper IR file",
    "slug": "load-flipper-ir-file",
    "description": "Upload .ir files to LittleFS, load them in Infrared mode, and send saved remote frames.",
    "difficulty": "Intermediate",
    "mark": "IR",
    "protocols": [
      "Infrared"
    ],
    "targets": [
      "IR file",
      "Remote control"
    ],
    "workflows": [
      "Replay",
      "File workflow"
    ],
    "tools": [
      "Serial CLI",
      "Web UI LittleFS"
    ],
    "tags": [
      "infrared",
      "flipper",
      "ir-file",
      "littlefs",
      "remote"
    ],
    "cover": "assets/visual-ir-file.svg",
    "time": "3 min",
    "hardware": "IR transmitter LED, .ir file, LittleFS upload path",
    "takeaway": "Use saved .ir files as reusable remote profiles instead of manually entering every command.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Use USB HID keyboard and mouse mode",
    "slug": "use-usb-hid-keyboard-mouse",
    "description": "Use USB mode to send simple keyboard text, mouse movement and mouse clicks over native USB HID.",
    "difficulty": "Beginner",
    "mark": "USB",
    "protocols": [
      "USB"
    ],
    "targets": [
      "USB host",
      "HID device"
    ],
    "workflows": [
      "Emulate device",
      "Test"
    ],
    "tools": [
      "Serial CLI",
      "USB HID"
    ],
    "tags": [
      "usb",
      "hid",
      "keyboard",
      "mouse",
      "test"
    ],
    "cover": "assets/visual-usb-hid.svg",
    "time": "5 min",
    "hardware": "ESP32-S3 native USB connection",
    "takeaway": "Test HID behavior directly from the firmware without writing a TinyUSB example.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Expose an SD card as USB storage",
    "slug": "usb-mass-storage-sd-card",
    "description": "Use USB storage mode to expose the SD card over USB mass storage from ESP32 Bit Pirate.",
    "difficulty": "Intermediate",
    "mark": "MSC",
    "protocols": [
      "USB"
    ],
    "targets": [
      "SD card",
      "USB host"
    ],
    "workflows": [
      "File workflow",
      "Inspect"
    ],
    "tools": [
      "Serial CLI",
      "USB Mass Storage"
    ],
    "tags": [
      "usb",
      "msc",
      "sd-card",
      "storage",
      "files"
    ],
    "cover": "assets/visual-usb-storage.svg",
    "time": "3 min",
    "hardware": "SD card wired to SPI and native USB board",
    "takeaway": "Move files through the board without removing the SD card when the hardware supports it.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Open the SPI SD card shell",
    "slug": "open-spi-sd-card-shell",
    "description": "Use SPI mode to open the SD card shell and run simple file commands like ls, cat, touch and mkdir.",
    "difficulty": "Beginner",
    "mark": "SD",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "SD card"
    ],
    "workflows": [
      "File workflow",
      "Inspect"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "spi",
      "sd-card",
      "filesystem",
      "files",
      "debugging"
    ],
    "cover": "assets/visual-spi-sdcard.svg",
    "time": "5 min",
    "hardware": "FAT/FAT32 SD card wired to SPI",
    "takeaway": "Check files from the bench without moving the card or adding a temporary sketch.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Analyze a SPI flash from the CLI",
    "slug": "analyze-spi-flash-cli",
    "description": "Use the SPI flash shell to probe a NOR flash chip, analyze contents, search strings and read bytes.",
    "difficulty": "Intermediate",
    "mark": "SPI",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "Flash chip",
      "Firmware image"
    ],
    "workflows": [
      "Analyze",
      "Reverse engineering"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "spi",
      "flash",
      "firmware",
      "strings",
      "analysis"
    ],
    "cover": "assets/visual-flash-analyze.svg",
    "time": "5 min",
    "hardware": "SPI NOR flash chip, SOIC clip or wires",
    "takeaway": "Probe first, then use analyze/search/strings before deciding what to dump or modify.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan Wi-Fi and find local devices",
    "slug": "scan-wifi-find-local-devices",
    "description": "Use Wi-Fi mode to scan networks, connect to your own network, show status and discover local devices.",
    "difficulty": "Beginner",
    "mark": "WiFi",
    "protocols": [
      "Wi-Fi"
    ],
    "targets": [
      "Local network",
      "IoT device"
    ],
    "workflows": [
      "Network discovery",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web CLI"
    ],
    "tags": [
      "wifi",
      "network",
      "discovery",
      "iot",
      "debugging"
    ],
    "cover": "assets/visual-wifi.svg",
    "time": "3 min",
    "hardware": "ESP32 Bit Pirate with Wi-Fi access",
    "takeaway": "Use the board as a small network probe after it connects to your own Wi-Fi.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Sniff CAN frames with MCP2515",
    "slug": "sniff-can-frames-mcp2515",
    "description": "Use CAN mode with an MCP2515 module to configure bitrate, check status and capture CAN frames.",
    "difficulty": "Intermediate",
    "mark": "CAN",
    "protocols": [
      "CAN"
    ],
    "targets": [
      "CAN bus",
      "MCP2515"
    ],
    "workflows": [
      "Sniff / capture",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "can",
      "mcp2515",
      "sniff",
      "frames",
      "debugging"
    ],
    "cover": "assets/visual-can.svg",
    "time": "5 min",
    "hardware": "MCP2515 CAN module, transceiver, CAN target",
    "takeaway": "Configure bitrate and verify controller status before trusting captured CAN frames.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Auto-detect an unknown UART baud rate",
    "slug": "detect-uart-baudrate-autobaud",
    "description": "Use ESP32 Bit Pirate UART autobaud to estimate the baud rate of an unknown serial line before opening a bridge or sending data.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "BAUD",
    "protocols": [
      "UART"
    ],
    "targets": [
      "Unknown UART",
      "Boot log"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "autobaud",
      "debugging",
      "serial",
      "beginner"
    ],
    "cover": "assets/visual-uart-autobaud.svg",
    "time": "5 min",
    "hardware": "Target UART TX line, shared ground",
    "takeaway": "Find the baud rate before you waste time guessing terminal settings.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Bridge a UART console",
    "slug": "bridge-uart-console",
    "description": "Use ESP32 Bit Pirate UART bridge mode to interact with a board console once RX, TX, voltage and baud rate are known.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "TTY",
    "protocols": [
      "UART"
    ],
    "targets": [
      "Console",
      "Boot log"
    ],
    "workflows": [
      "Bridge",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "bridge",
      "console",
      "serial",
      "debugging"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "3 min",
    "hardware": "Target UART RX/TX header, shared ground",
    "takeaway": "Turn the board into a quick serial console adapter after the passive checks are done.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Transfer files over UART with XMODEM",
    "slug": "transfer-uart-xmodem-sd",
    "description": "Use ESP32 Bit Pirate UART XMODEM send or receive with SD storage for bootloaders and serial file-transfer workflows.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "XMD",
    "protocols": [
      "UART"
    ],
    "targets": [
      "Bootloader",
      "SD card"
    ],
    "workflows": [
      "File transfer",
      "Repair"
    ],
    "tools": [
      "Serial CLI",
      "SD card"
    ],
    "tags": [
      "uart",
      "xmodem",
      "sdcard",
      "file transfer",
      "bootloader"
    ],
    "cover": "assets/visual-file-workflow.svg",
    "time": "10 min",
    "hardware": "Target UART bootloader, SD card, shared ground",
    "takeaway": "Stage the file on SD, then use XMODEM only when the target expects it.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan nearby Bluetooth devices",
    "slug": "scan-bluetooth-devices",
    "description": "Use ESP32 Bit Pirate Bluetooth scan mode to list nearby BLE devices before pairing, sniffing or HID testing.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "BLE",
    "protocols": [
      "Bluetooth"
    ],
    "targets": [
      "BLE device",
      "Advertisement"
    ],
    "workflows": [
      "Scan",
      "Identify"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "bluetooth",
      "ble",
      "scan",
      "advertising",
      "debugging"
    ],
    "cover": "assets/visual-ble.svg",
    "time": "5 min",
    "hardware": "ESP32-S3 board with Bluetooth enabled",
    "takeaway": "List nearby devices and addresses before attempting any connection.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Test BLE HID keyboard and mouse mode",
    "slug": "use-ble-hid-server",
    "description": "Start the ESP32 Bit Pirate BLE HID server and send simple keyboard or mouse actions to your own paired host.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "HID",
    "protocols": [
      "Bluetooth"
    ],
    "targets": [
      "Own computer",
      "BLE HID"
    ],
    "workflows": [
      "HID test",
      "Input test"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "bluetooth",
      "ble",
      "hid",
      "keyboard",
      "mouse"
    ],
    "cover": "assets/visual-ble.svg",
    "time": "3 min",
    "hardware": "Own BLE host computer or phone",
    "takeaway": "Test HID behavior without writing a BLEHIDDevice sketch.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Find active Sub-GHz frequencies",
    "slug": "scan-subghz-frequency-cc1101",
    "description": "Use ESP32 Bit Pirate SubGHz scan and sweep workflows with a CC1101 module to find RSSI peaks in common sub-GHz bands.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "SG",
    "protocols": [
      "SubGHz"
    ],
    "targets": [
      "CC1101",
      "RF signal"
    ],
    "workflows": [
      "Scan",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Device screen"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "scan",
      "sweep",
      "rf",
      "receive"
    ],
    "cover": "assets/visual-subghz.svg",
    "time": "10 min",
    "hardware": "CC1101 module, antenna, SPI wiring",
    "takeaway": "Discover the active frequency before trying to decode a frame.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Receive and analyze a Sub-GHz frame",
    "slug": "receive-subghz-frame",
    "description": "Tune a CC1101 receiver and use ESP32 Bit Pirate SubGHz receive mode to capture raw pulses and inspect likely encoding.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "RX",
    "protocols": [
      "SubGHz"
    ],
    "targets": [
      "CC1101",
      "Remote signal"
    ],
    "workflows": [
      "Receive",
      "Decode"
    ],
    "tools": [
      "Serial CLI",
      "Device screen"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "receive",
      "decode",
      "rf"
    ],
    "cover": "assets/visual-waveform.svg",
    "time": "5 min",
    "hardware": "CC1101 module, antenna, known own remote/sensor",
    "takeaway": "Capture the frame shape and encoding hints before deciding whether any replay workflow is appropriate.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan 2.4 GHz RF24 channel activity",
    "slug": "scan-rf24-channel-activity",
    "description": "Use ESP32 Bit Pirate RF24 scan, sweep and waterfall workflows with an nRF24L01 module to find active 2.4 GHz channels.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "RF24",
    "protocols": [
      "RF24"
    ],
    "targets": [
      "nRF24L01",
      "2.4 GHz"
    ],
    "workflows": [
      "Scan",
      "Waterfall"
    ],
    "tools": [
      "Serial CLI",
      "Device screen"
    ],
    "tags": [
      "rf24",
      "nrf24l01",
      "scan",
      "2.4ghz",
      "waterfall"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "5 min",
    "hardware": "nRF24L01 module, SPI wiring, 3.3V supply",
    "takeaway": "Use channel activity as a clue before trying any payload receive workflow.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Read a PN532 RFID tag UID",
    "slug": "read-rfid-tag-pn532",
    "description": "Use ESP32 Bit Pirate RFID mode with a PN532 reader to read UID, ATQA, SAK and tag type from a nearby 13.56 MHz tag.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "NFC",
    "protocols": [
      "RFID"
    ],
    "targets": [
      "PN532",
      "NFC tag"
    ],
    "workflows": [
      "Read",
      "Identify"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "rfid",
      "nfc",
      "pn532",
      "uid",
      "mifare"
    ],
    "cover": "assets/visual-rfid.svg",
    "time": "5 min",
    "hardware": "PN532 in I2C mode, 13.56 MHz tag",
    "takeaway": "Identify a tag family and UID before attempting any write or clone workflow.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan the FM band with Si4713",
    "slug": "scan-fm-band-si4713",
    "description": "Use ESP32 Bit Pirate FM sweep, trace and waterfall workflows with a Si4713 module to inspect FM band activity.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "FM",
    "protocols": [
      "FM"
    ],
    "targets": [
      "Si4713",
      "FM band"
    ],
    "workflows": [
      "Scan",
      "Waterfall"
    ],
    "tools": [
      "Serial CLI",
      "Device screen"
    ],
    "tags": [
      "fm",
      "si4713",
      "sweep",
      "waterfall",
      "radio"
    ],
    "cover": "assets/visual-fm.svg",
    "time": "5 min",
    "hardware": "Si4713 module, I2C wiring, reset pin",
    "takeaway": "Use sweep and trace to inspect the band before considering any broadcast experiment.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Check cellular modem status",
    "slug": "check-cellular-modem-status",
    "description": "Use ESP32 Bit Pirate CELL mode to query modem, SIM, network and operator status over a UART-connected cellular module.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "LTE",
    "protocols": [
      "CELL"
    ],
    "targets": [
      "Cellular modem",
      "SIM card"
    ],
    "workflows": [
      "Status",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "cellular",
      "modem",
      "simcom",
      "lte",
      "gsm",
      "uart"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "3 min",
    "hardware": "SIMCom-compatible modem, antenna, SIM card, UART wiring",
    "takeaway": "Check modem/SIM/network state before trying SMS, calls or USSD.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan for SWD pins",
    "slug": "scan-swd-pinout",
    "description": "Use ESP32 Bit Pirate JTAG mode to scan a group of GPIOs and look for SWDIO/SWCLK on your own target board.",
    "difficulty": "Advanced",
    "type": "Recipe",
    "mark": "SWD",
    "protocols": [
      "JTAG/SWD"
    ],
    "targets": [
      "Debug header",
      "Unknown pinout"
    ],
    "workflows": [
      "Pinout scan",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "swd",
      "jtag",
      "pinout",
      "debug",
      "reverse engineering"
    ],
    "cover": "assets/visual-swd.svg",
    "time": "5 min",
    "hardware": "Own target board with suspected SWD pins",
    "takeaway": "Reduce blind probing by scanning a controlled set of candidate pins.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Scan for a JTAG pinout",
    "slug": "scan-jtag-pinout",
    "description": "Use ESP32 Bit Pirate JTAG mode to scan candidate GPIOs for TDI, TDO, TCK and TMS on your own target board.",
    "difficulty": "Advanced",
    "type": "Recipe",
    "mark": "JTAG",
    "protocols": [
      "JTAG/SWD"
    ],
    "targets": [
      "Debug header",
      "Unknown pinout"
    ],
    "workflows": [
      "Pinout scan",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "jtag",
      "pinout",
      "jtagulator",
      "debug",
      "hardware"
    ],
    "cover": "assets/visual-jtag.svg",
    "time": "5 min",
    "hardware": "Own target board with candidate JTAG pads",
    "takeaway": "Let the firmware test permutations instead of manually trying every JTAG wire order.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Use OpenOCD adapter mode",
    "slug": "use-openocd-jtag-adapter",
    "description": "Use ESP32 Bit Pirate JTAG mode openocd command to switch the device into an OpenOCD-compatible adapter workflow after pinout is known.",
    "difficulty": "Advanced",
    "type": "Recipe",
    "mark": "OCD",
    "protocols": [
      "JTAG/SWD"
    ],
    "targets": [
      "OpenOCD",
      "Debug adapter"
    ],
    "workflows": [
      "Adapter",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "OpenOCD"
    ],
    "tags": [
      "openocd",
      "jtag",
      "swd",
      "adapter",
      "debug"
    ],
    "cover": "assets/visual-openocd.svg",
    "time": "5 min",
    "hardware": "Known JTAG/SWD target pinout",
    "takeaway": "Switch to adapter mode only after wiring and target voltage are confirmed.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Bridge a half-duplex UART console",
    "slug": "bridge-half-duplex-uart",
    "description": "Use HDUART mode to bridge a single-wire half-duplex UART console.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "HD",
    "protocols": [
      "HDUART"
    ],
    "targets": [
      "Single-wire UART",
      "Debug console"
    ],
    "workflows": [
      "Bridge",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "hduart",
      "uart",
      "half-duplex",
      "serial",
      "debugging"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "3 min",
    "hardware": "Single-wire UART target, shared ground",
    "takeaway": "Use one shared TX/RX GPIO when the target console is half-duplex instead of normal UART.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Read an SLE4442 smartcard",
    "slug": "read-sle4442-smartcard",
    "description": "Use 2WIRE mode to probe, check security bytes and dump an SLE4442 style smartcard.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "2W",
    "protocols": [
      "2WIRE"
    ],
    "targets": [
      "SLE4442",
      "Smartcard"
    ],
    "workflows": [
      "Read",
      "Dump / backup"
    ],
    "tools": [
      "Serial CLI",
      "Smartcard shell"
    ],
    "tags": [
      "2wire",
      "smartcard",
      "sle4442",
      "memory card",
      "dump"
    ],
    "cover": "assets/visual-smartcard.svg",
    "time": "5 min",
    "hardware": "SLE44XX card, smartcard socket, CLK/IO/RST wiring",
    "takeaway": "Probe ATR and security memory before any write or PSC operation.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Dump a 93Cxx Microwire EEPROM",
    "slug": "dump-microwire-93c-eeprom",
    "description": "Use 3WIRE mode to select a 93Cxx EEPROM model, probe it and dump memory.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "3W",
    "protocols": [
      "3WIRE"
    ],
    "targets": [
      "93Cxx EEPROM",
      "Microwire EEPROM"
    ],
    "workflows": [
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "Serial CLI",
      "EEPROM shell"
    ],
    "tags": [
      "3wire",
      "microwire",
      "93c46",
      "eeprom",
      "backup"
    ],
    "cover": "assets/visual-threewire.svg",
    "time": "5 min",
    "hardware": "93Cxx EEPROM, SOIC clip or jumper wires",
    "takeaway": "Select the right chip model and ORG setting before trusting a 3WIRE EEPROM dump.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Test an I2S microphone or speaker",
    "slug": "test-i2s-microphone-speaker",
    "description": "Use I2S mode to test microphone input, speaker output and tone playback.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "I2S",
    "protocols": [
      "I2S"
    ],
    "targets": [
      "I2S microphone",
      "I2S amplifier"
    ],
    "workflows": [
      "Test",
      "Measure"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "i2s",
      "audio",
      "microphone",
      "speaker",
      "test"
    ],
    "cover": "assets/visual-i2s.svg",
    "time": "5 min",
    "hardware": "I2S microphone or amplifier, BCLK/LRCK/DATA wiring",
    "takeaway": "Verify audio wiring with built-in tests before writing a custom sketch.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Connect a W5500 Ethernet module",
    "slug": "connect-w5500-ethernet",
    "description": "Use Ethernet mode with a W5500 module to get DHCP, check status, ping and discover local devices.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "ETH",
    "protocols": [
      "Ethernet"
    ],
    "targets": [
      "W5500",
      "Local network"
    ],
    "workflows": [
      "Network discovery",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "ethernet",
      "w5500",
      "dhcp",
      "network",
      "ping"
    ],
    "cover": "assets/visual-ethernet.svg",
    "time": "10 min",
    "hardware": "W5500 Ethernet module, SPI wiring, Ethernet cable",
    "takeaway": "Use a wired network path to separate Wi-Fi problems from LAN or application problems.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Test an addressable LED strip",
    "slug": "test-addressable-led-strip",
    "description": "Use LED mode to test fill, set, chase and reset on an addressable LED strip.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "LED",
    "protocols": [
      "LED"
    ],
    "targets": [
      "Addressable LED",
      "LED strip"
    ],
    "workflows": [
      "Test",
      "Generate signal"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "led",
      "neopixel",
      "ws2812",
      "strip",
      "test"
    ],
    "cover": "assets/visual-led.svg",
    "time": "5 min",
    "hardware": "Addressable LED strip or ring, external supply for larger strips",
    "takeaway": "Verify LED power, data direction, protocol and color order before writing animation code.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Bridge the ESP32-C5 Expander",
    "slug": "bridge-esp32-c5-expander",
    "description": "Use Expander mode to configure RX/TX, handshake with an ESP32-C5 module and bridge the terminal.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "EXP",
    "protocols": [
      "Expander"
    ],
    "targets": [
      "ESP32-C5",
      "Expansion module"
    ],
    "workflows": [
      "Bridge",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "expander",
      "esp32-c5",
      "uart",
      "bridge",
      "handshake"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "5 min",
    "hardware": "ESP32-C5 expander module, UART wiring, shared ground",
    "takeaway": "Confirm the expansion module handshake before using the bridged shell.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Send a CAN frame with MCP2515",
    "slug": "send-can-frame-mcp2515",
    "description": "Use CAN mode with an MCP2515 module to send a standard 11-bit CAN frame on a controlled bus.",
    "difficulty": "Advanced",
    "type": "Recipe",
    "mark": "CAN",
    "protocols": [
      "CAN"
    ],
    "targets": [
      "CAN bus",
      "MCP2515"
    ],
    "workflows": [
      "Transmit",
      "Test"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "can",
      "mcp2515",
      "send",
      "frame",
      "11-bit"
    ],
    "cover": "assets/visual-can.svg",
    "time": "10 min",
    "hardware": "MCP2515 CAN module, terminated test bus, known bitrate",
    "takeaway": "Send one known standard CAN frame after status confirms the adapter is healthy.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Read I2C registers",
    "slug": "read-i2c-registers",
    "description": "Use I2C read and regs commands to inspect known device registers after a scan.",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "REG",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "I2C sensor",
      "Register map"
    ],
    "workflows": [
      "Read",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2c",
      "register",
      "sensor",
      "datasheet",
      "debugging"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "5 min",
    "hardware": "Known I2C device with datasheet",
    "takeaway": "Read a documented ID or status register to confirm the device behaves like the expected part.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Check an HTTP endpoint",
    "slug": "check-http-endpoint",
    "description": "Use Wi-Fi or Ethernet mode and http get to verify a local IoT endpoint.",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "HTTP",
    "protocols": [
      "Wi-Fi",
      "Ethernet"
    ],
    "targets": [
      "HTTP endpoint",
      "IoT device"
    ],
    "workflows": [
      "Network discovery",
      "Debug"
    ],
    "tools": [
      "Serial CLI"
    ],
    "tags": [
      "wifi",
      "ethernet",
      "http",
      "iot",
      "network"
    ],
    "cover": "assets/visual-http-api.svg",
    "time": "5 min",
    "hardware": "Wi-Fi or W5500 Ethernet connection, local HTTP endpoint",
    "takeaway": "Test the actual web endpoint, not just ping reachability.",
    "date": "2026-06-23",
    "author": "Geo"
  },
  {
    "title": "Open a Web Serial terminal",
    "slug": "open-web-serial-terminal",
    "description": "Use the browser Web Serial terminal or a normal serial terminal to reach the ESP32 Bit Pirate CLI at 115200 baud.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": ">_",
    "protocols": [
      "Terminal",
      "Web Serial"
    ],
    "targets": [
      "ESP32 Bit Pirate",
      "CLI"
    ],
    "workflows": [
      "Setup",
      "Debug"
    ],
    "tools": [
      "Web Serial Terminal",
      "Serial CLI"
    ],
    "tags": [
      "beginner",
      "web serial",
      "terminal",
      "setup",
      "cli"
    ],
    "cover": "assets/visual-webserial.svg",
    "time": "1 min",
    "hardware": "ESP32 Bit Pirate board and USB cable",
    "takeaway": "Open the CLI from a browser before installing a desktop terminal."
  },
  {
    "title": "Use Bus Pirate-style bytecode instructions",
    "slug": "use-bytecode-instructions",
    "description": "Use ESP32 Bit Pirate bracket instruction syntax to write bytes, read bytes and add microsecond or millisecond delays.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "[]",
    "protocols": [
      "Instructions",
      "SPI",
      "UART"
    ],
    "targets": [
      "Unknown protocol",
      "Serial device",
      "SPI device"
    ],
    "workflows": [
      "Protocol experiment",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "bytecode",
      "instructions",
      "bus pirate syntax",
      "spi",
      "uart",
      "i2c"
    ],
    "cover": "assets/visual-protocol-lab.svg",
    "time": "5 min",
    "hardware": "Any target supported by the active mode",
    "takeaway": "Use compact bracket sequences when a full driver would be overkill."
  },
  {
    "title": "Read a SPI flash JEDEC ID",
    "slug": "read-spi-flash-jedec-id",
    "description": "Use ESP32 Bit Pirate SPI mode and the [0x9F r:3] instruction to read the JEDEC ID from a SPI NOR flash chip.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "9F",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "SPI flash",
      "Unknown flash chip"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "spi",
      "flash",
      "jedec",
      "w25q",
      "firmware",
      "debugging"
    ],
    "cover": "assets/visual-scan.svg",
    "time": "5 min",
    "hardware": "SPI NOR flash chip, jumper wires or clip",
    "takeaway": "Read manufacturer/device bytes before a full flash dump or write."
  },
  {
    "title": "Dump a SPI EEPROM 25X chip",
    "slug": "dump-spi-eeprom-25x",
    "description": "Use the ESP32 Bit Pirate SPI EEPROM shell to probe, read, dump, write or erase 25X-series SPI EEPROM chips.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "25X",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "SPI EEPROM",
      "25X EEPROM"
    ],
    "workflows": [
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "spi",
      "eeprom",
      "25x",
      "backup",
      "repair"
    ],
    "cover": "assets/visual-spi-eeprom.svg",
    "time": "10 min",
    "hardware": "25X-series SPI EEPROM, jumper wires or clip",
    "takeaway": "Use the dedicated SPI EEPROM shell instead of guessing raw reads."
  },
  {
    "title": "Program an AVR with AVRDUDE",
    "slug": "program-avr-with-avrdude",
    "description": "Use the ESP32 Bit Pirate AVRDUDE Bus Pirate SPI adapter to probe, read and write ATmega or ATtiny chips over ISP.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "AVR",
    "protocols": [
      "SPI",
      "AVR ISP"
    ],
    "targets": [
      "ATmega",
      "ATtiny",
      "Arduino-compatible board"
    ],
    "workflows": [
      "Program",
      "Dump / backup",
      "Repair"
    ],
    "tools": [
      "AVRDUDE",
      "USB Adapter"
    ],
    "tags": [
      "avr",
      "avrdude",
      "isp",
      "spi",
      "firmware",
      "fuses"
    ],
    "cover": "assets/visual-dump.svg",
    "time": "10 min",
    "hardware": "AVR target with ISP pins exposed",
    "takeaway": "Use ESP32 Bit Pirate as the Bus Pirate-style adapter AVRDUDE already understands."
  },
  {
    "title": "Write a SPI flash image with Flashrom",
    "slug": "write-spi-flash-with-flashrom",
    "description": "Use the ESP32 Bit Pirate Flashrom serprog adapter to write and verify a SPI NOR flash image with flashrom.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "WR",
    "protocols": [
      "SPI"
    ],
    "targets": [
      "SPI flash",
      "Firmware image"
    ],
    "workflows": [
      "Write",
      "Repair",
      "Verify"
    ],
    "tools": [
      "Flashrom",
      "USB Adapter",
      "Web SPI Flash Programmer"
    ],
    "tags": [
      "spi",
      "flashrom",
      "flash",
      "firmware",
      "write",
      "repair"
    ],
    "cover": "assets/visual-repair.svg",
    "time": "10 min",
    "hardware": "SPI NOR flash chip, known-good firmware image",
    "takeaway": "Write only after probe and backup; let Flashrom verify the image."
  },
  {
    "title": "Read USB host descriptors",
    "slug": "read-usb-host-descriptors",
    "description": "Use ESP32 Bit Pirate USB host mode to connect a USB device and dump its descriptors from the firmware.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "USB",
    "protocols": [
      "USB"
    ],
    "targets": [
      "USB device",
      "Descriptor dump"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "usb",
      "host",
      "descriptors",
      "debugging",
      "enumeration"
    ],
    "cover": "assets/visual-usb-host.svg",
    "time": "5 min",
    "hardware": "USB device and compatible ESP32-S3 host wiring",
    "takeaway": "Dump descriptors before writing USB host code."
  },
  {
    "title": "Use the USB IR Toy LIRC adapter",
    "slug": "use-usb-ir-toy-lirc-adapter",
    "description": "Use the ESP32 Bit Pirate USB IR Toy adapter with LIRC tools such as mode2, xmode2 and irrecord.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "IR",
    "protocols": [
      "Infrared",
      "USB Adapter"
    ],
    "targets": [
      "IR remote",
      "IR receiver",
      "IR LED"
    ],
    "workflows": [
      "Sniff / capture",
      "Replay",
      "Debug"
    ],
    "tools": [
      "LIRC",
      "USB IR Toy",
      "Serial CLI"
    ],
    "tags": [
      "infrared",
      "lirc",
      "irtoy",
      "remote",
      "capture"
    ],
    "cover": "assets/visual-ir-file.svg",
    "time": "10 min",
    "hardware": "IR receiver module and IR LED driver",
    "takeaway": "Use mature LIRC tools when you want raw IR timing capture from the desktop."
  },
  {
    "title": "Pulse a GPIO in microseconds",
    "slug": "pulse-gpio-microseconds",
    "description": "Use ESP32 Bit Pirate DIO mode to set pin levels, pulse a GPIO for a microsecond duration and reset it afterward.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "DIO",
    "protocols": [
      "DIO",
      "GPIO"
    ],
    "targets": [
      "GPIO pin",
      "Digital input"
    ],
    "workflows": [
      "Stimulate",
      "Debug",
      "Measure"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "dio",
      "pulse",
      "microseconds",
      "debugging"
    ],
    "cover": "assets/visual-dio.svg",
    "time": "5 min",
    "hardware": "One target input pin and jumper wire",
    "takeaway": "Generate a quick trigger pulse without writing a sketch."
  },
  {
    "title": "Sniff GPIO edges from the CLI",
    "slug": "sniff-gpio-edges-cli",
    "description": "Use ESP32 Bit Pirate DIO sniff and scan commands to monitor GPIO transitions before opening a full logic analyzer capture.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "EDGE",
    "protocols": [
      "DIO",
      "GPIO"
    ],
    "targets": [
      "Digital signal",
      "Unknown pin"
    ],
    "workflows": [
      "Sniff / capture",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "dio",
      "sniff",
      "edges",
      "logic",
      "debugging"
    ],
    "cover": "assets/visual-sniffer.svg",
    "time": "5 min",
    "hardware": "One digital signal and jumper wire",
    "takeaway": "Find out if a pin is alive before committing to a full capture."
  },
  {
    "title": "Open a Modbus TCP session",
    "slug": "open-modbus-tcp-session",
    "description": "Use ESP32 Bit Pirate Wi-Fi or Ethernet mode to open the interactive Modbus session against an authorized host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "MB",
    "protocols": [
      "Modbus",
      "Wi-Fi",
      "Ethernet"
    ],
    "targets": [
      "PLC",
      "Industrial device",
      "Local host"
    ],
    "workflows": [
      "Debug",
      "Network test"
    ],
    "tools": [
      "Serial CLI",
      "Ethernet",
      "Wi-Fi"
    ],
    "tags": [
      "modbus",
      "tcp",
      "ethernet",
      "wifi",
      "plc",
      "debugging"
    ],
    "cover": "assets/visual-modbus.svg",
    "time": "5 min",
    "hardware": "Authorized Modbus TCP endpoint on your network",
    "takeaway": "Use Modbus mode only on systems you own or are explicitly allowed to test."
  },
  {
    "title": "Test a raw TCP service with nc or telnet",
    "slug": "test-raw-tcp-nc-telnet",
    "description": "Use ESP32 Bit Pirate Wi-Fi or Ethernet mode with nc and telnet commands to debug authorized raw TCP services.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "TCP",
    "protocols": [
      "Wi-Fi",
      "Ethernet",
      "TCP"
    ],
    "targets": [
      "TCP service",
      "IoT module",
      "Serial-over-IP gateway"
    ],
    "workflows": [
      "Debug",
      "Network test"
    ],
    "tools": [
      "Serial CLI",
      "Wi-Fi",
      "Ethernet"
    ],
    "tags": [
      "tcp",
      "netcat",
      "telnet",
      "wifi",
      "ethernet",
      "debugging"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "5 min",
    "hardware": "Authorized TCP service on local network",
    "takeaway": "Check the service itself, not just network reachability."
  },
  {
    "title": "Check open ports with nmap mode",
    "slug": "check-open-ports-nmap",
    "description": "Use ESP32 Bit Pirate Wi-Fi or Ethernet nmap command to check selected TCP or UDP ports on an authorized host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "SCAN",
    "protocols": [
      "Wi-Fi",
      "Ethernet",
      "TCP",
      "UDP"
    ],
    "targets": [
      "Local host",
      "IoT device",
      "Lab server"
    ],
    "workflows": [
      "Network test",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Wi-Fi",
      "Ethernet"
    ],
    "tags": [
      "nmap",
      "ports",
      "wifi",
      "ethernet",
      "network",
      "debugging"
    ],
    "cover": "assets/visual-scan.svg",
    "time": "5 min",
    "hardware": "Authorized host on your own network",
    "takeaway": "Check only the ports you need, on systems you are allowed to test."
  },
  {
    "title": "Choose the right GPIO pinout",
    "slug": "choose-gpio-pinout-before-wiring",
    "description": "Use the ESP32 Bit Pirate pinout reference and mode config commands before wiring a target device.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "PIN",
    "protocols": [
      "Pinout",
      "GPIO"
    ],
    "targets": [
      "ESP32 board",
      "Peripheral"
    ],
    "workflows": [
      "Setup",
      "Troubleshooting"
    ],
    "tools": [
      "Wiki",
      "Serial CLI"
    ],
    "tags": [
      "pinout",
      "gpio",
      "wiring",
      "setup",
      "troubleshooting"
    ],
    "cover": "assets/visual-pinout.svg",
    "time": "5 min",
    "hardware": "Any ESP32 Bit Pirate supported board",
    "takeaway": "Check exposed GPIOs before assigning protocol pins."
  },
  {
    "title": "Save pin profiles and aliases",
    "slug": "save-pin-profiles-and-aliases",
    "description": "Use ESP32 Bit Pirate general commands such as profile, alias and repeat to make repeated bench workflows faster.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "CLI",
    "protocols": [
      "General",
      "CLI"
    ],
    "targets": [
      "Repeated workflow",
      "Bench setup"
    ],
    "workflows": [
      "Setup",
      "Automation"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "profile",
      "alias",
      "repeat",
      "cli",
      "workflow",
      "setup"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "5 min",
    "hardware": "ESP32 Bit Pirate firmware",
    "takeaway": "Turn repeated bench steps into a shorter command flow."
  },
  {
    "title": "Use the SubGHz Raw CDC adapter",
    "slug": "use-subghz-raw-cdc-adapter",
    "description": "Use the ESP32 Bit Pirate SubGHz Raw CDC adapter to control a CC1101 module from a terminal or script over USB CDC.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "RF",
    "protocols": [
      "SubGHz",
      "USB Adapter"
    ],
    "targets": [
      "CC1101",
      "OOK signal",
      "Raw RF timings"
    ],
    "workflows": [
      "Sniff / capture",
      "Script",
      "Debug"
    ],
    "tools": [
      "USB CDC",
      "Python",
      "Serial terminal"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "raw cdc",
      "ook",
      "rf",
      "script"
    ],
    "cover": "assets/visual-subghz.svg",
    "time": "10 min",
    "hardware": "CC1101 module wired to ESP32 Bit Pirate",
    "takeaway": "Drive CC1101 from scripts when the normal interactive UI is not enough."
  },
  {
    "title": "Dump SIM card information from a cellular modem",
    "slug": "dump-sim-card-info-cellular-modem",
    "description": "Use CELL mode to query SIM card status and identifiers from a cellular modem with the sim command.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📱",
    "mark": "SIM",
    "protocols": [
      "CELL",
      "UART",
      "AT"
    ],
    "targets": [
      "SIM card",
      "Cellular modem",
      "SIMCom module"
    ],
    "workflows": [
      "Identify",
      "Debug",
      "Field test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cell",
      "uart",
      "at",
      "sim-card",
      "cellular-modem",
      "simcom-module",
      "identify",
      "debug",
      "field-test"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "5 min",
    "hardware": "SIMCom-style cellular modem with SIM card",
    "takeaway": "Check SIM status before chasing network or SMS problems."
  },
  {
    "title": "Check GSM/LTE network registration",
    "slug": "check-cellular-network-registration",
    "description": "Use CELL mode to check cellular registration, signal information and available operators from a GSM/LTE modem.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📶",
    "mark": "LTE",
    "protocols": [
      "CELL",
      "UART",
      "AT"
    ],
    "targets": [
      "Cellular modem",
      "LTE module",
      "GSM module"
    ],
    "workflows": [
      "Debug",
      "Identify",
      "Field test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cell",
      "uart",
      "at",
      "cellular-modem",
      "lte-module",
      "gsm-module",
      "debug",
      "identify",
      "field-test"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "3 min",
    "hardware": "Cellular modem, LTE module, GSM module",
    "takeaway": "Use CELL mode to check cellular registration, signal information and available operators from a GSM/LTE modem."
  },
  {
    "title": "Send a USSD request from a modem",
    "slug": "send-ussd-cellular-modem",
    "description": "Use CELL mode to send USSD codes from a cellular modem after SIM and network registration checks.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "#️⃣",
    "mark": "USSD",
    "protocols": [
      "CELL",
      "UART",
      "AT"
    ],
    "targets": [
      "Cellular modem",
      "SIM card",
      "Operator service"
    ],
    "workflows": [
      "Test",
      "Field test",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cell",
      "uart",
      "at",
      "cellular-modem",
      "sim-card",
      "operator-service",
      "test",
      "field-test",
      "debug"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "5 min",
    "hardware": "Cellular modem, SIM card, Operator service",
    "takeaway": "Use CELL mode to send USSD codes from a cellular modem after SIM and network registration checks."
  },
  {
    "title": "List SIM phonebook contacts",
    "slug": "list-sim-phonebook-contacts",
    "description": "Use CELL mode to display contacts stored on a SIM card through a cellular modem phonebook command.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "☎️",
    "mark": "SIM",
    "protocols": [
      "CELL",
      "UART",
      "AT"
    ],
    "targets": [
      "SIM card",
      "Cellular modem"
    ],
    "workflows": [
      "Inspect",
      "Debug",
      "Recover data"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cell",
      "uart",
      "at",
      "sim-card",
      "cellular-modem",
      "inspect",
      "debug",
      "recover-data"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "5 min",
    "hardware": "SIM card, Cellular modem",
    "takeaway": "Use CELL mode to display contacts stored on a SIM card through a cellular modem phonebook command."
  },
  {
    "title": "Send an SMS from a cellular modem",
    "slug": "send-sms-from-cellular-modem",
    "description": "Use CELL mode sms operations to send a message from a GSM/LTE modem after SIM and network checks.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "📨",
    "mark": "SMS",
    "protocols": [
      "CELL",
      "UART",
      "AT"
    ],
    "targets": [
      "Cellular modem",
      "SIM card"
    ],
    "workflows": [
      "Test",
      "Field test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cell",
      "uart",
      "at",
      "cellular-modem",
      "sim-card",
      "test",
      "field-test"
    ],
    "cover": "assets/visual-cell.svg",
    "time": "5 min",
    "hardware": "Cellular modem, SIM card",
    "takeaway": "Use CELL mode sms operations to send a message from a GSM/LTE modem after SIM and network checks."
  },
  {
    "title": "Use the UART AT command helper",
    "slug": "use-uart-at-shell-modem",
    "description": "Use UART mode at helper to send guided AT commands to modems and serial modules.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📟",
    "mark": "AT",
    "protocols": [
      "UART",
      "AT"
    ],
    "targets": [
      "AT modem",
      "GPS module",
      "Serial module"
    ],
    "workflows": [
      "Debug",
      "Probe"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "at",
      "at-modem",
      "gps-module",
      "serial-module",
      "debug",
      "probe"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "3 min",
    "hardware": "AT modem, GPS module, Serial module",
    "takeaway": "Use UART mode at helper to send guided AT commands to modems and serial modules."
  },
  {
    "title": "Emulate a UART peripheral for firmware tests",
    "slug": "emulate-uart-gps-device",
    "description": "Use UART emulator mode to test how another board behaves when a serial peripheral sends expected data.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧪",
    "mark": "EMU",
    "protocols": [
      "UART"
    ],
    "targets": [
      "Firmware under test",
      "Serial peripheral"
    ],
    "workflows": [
      "Emulate",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "firmware-under-test",
      "serial-peripheral",
      "emulate",
      "debug"
    ],
    "cover": "assets/visual-gps.svg",
    "time": "3 min",
    "hardware": "Firmware under test, Serial peripheral",
    "takeaway": "Use UART emulator mode to test how another board behaves when a serial peripheral sends expected data."
  },
  {
    "title": "Auto-reply when UART text appears",
    "slug": "auto-reply-uart-trigger-pattern",
    "description": "Use UART trigger to send a predefined response when a byte pattern is detected.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🎯",
    "mark": "TRIG",
    "protocols": [
      "UART"
    ],
    "targets": [
      "Bootloader",
      "Serial shell",
      "Test fixture"
    ],
    "workflows": [
      "Automate",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "bootloader",
      "serial-shell",
      "test-fixture",
      "automate",
      "debug"
    ],
    "cover": "assets/visual-trigger.svg",
    "time": "3 min",
    "hardware": "Bootloader, Serial shell, Test fixture",
    "takeaway": "Use UART trigger to send a predefined response when a byte pattern is detected."
  },
  {
    "title": "Monitor I2C register changes",
    "slug": "monitor-i2c-register-changes",
    "description": "Use I2C monitor or trace to watch changing registers on a target device over time.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "👀",
    "mark": "MON",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "Sensor",
      "I2C peripheral"
    ],
    "workflows": [
      "Monitor",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2c",
      "sensor",
      "i2c-peripheral",
      "monitor",
      "debug"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "3 min",
    "hardware": "Sensor, I2C peripheral",
    "takeaway": "Use I2C monitor or trace to watch changing registers on a target device over time."
  },
  {
    "title": "Run I2C health diagnostics",
    "slug": "run-i2c-health-diagnostics",
    "description": "Use the I2C health command to check timing and stability of a target device.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🩺",
    "mark": "HLTH",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "Sensor",
      "I2C bus"
    ],
    "workflows": [
      "Diagnostics",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2c",
      "sensor",
      "i2c-bus",
      "diagnostics",
      "debug"
    ],
    "cover": "assets/visual-troubleshooting.svg",
    "time": "3 min",
    "hardware": "Sensor, I2C bus",
    "takeaway": "Use the I2C health command to check timing and stability of a target device."
  },
  {
    "title": "Emulate an I2C slave device",
    "slug": "emulate-i2c-slave-device",
    "description": "Use I2C slave mode to listen as a target address and log master operations.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "🎭",
    "mark": "SLV",
    "protocols": [
      "I2C"
    ],
    "targets": [
      "I2C master",
      "Firmware under test"
    ],
    "workflows": [
      "Emulate",
      "Sniff / capture"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2c",
      "i2c-master",
      "firmware-under-test",
      "emulate",
      "sniff-/-capture"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "3 min",
    "hardware": "I2C master, Firmware under test",
    "takeaway": "Use I2C slave mode to listen as a target address and log master operations."
  },
  {
    "title": "Write a test page to an NFC tag",
    "slug": "write-rfid-ntag-page-pn532",
    "description": "Use RFID mode with a PN532 module to write lab data to an NTAG or Ultralight page.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "✍️",
    "mark": "NFC",
    "protocols": [
      "RFID",
      "NFC",
      "I2C"
    ],
    "targets": [
      "PN532",
      "NTAG",
      "Ultralight tag"
    ],
    "workflows": [
      "Write",
      "Test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rfid",
      "nfc",
      "i2c",
      "pn532",
      "ntag",
      "ultralight-tag",
      "write",
      "test"
    ],
    "cover": "assets/visual-rfid.svg",
    "time": "3 min",
    "hardware": "PN532, NTAG, Ultralight tag",
    "takeaway": "Use RFID mode with a PN532 module to write lab data to an NTAG or Ultralight page."
  },
  {
    "title": "Erase a lab NFC/RFID tag",
    "slug": "erase-rfid-test-tag-pn532",
    "description": "Use RFID erase flow with PN532 for lab tags where wiping user data is expected.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧹",
    "mark": "NFC",
    "protocols": [
      "RFID",
      "NFC",
      "I2C"
    ],
    "targets": [
      "PN532",
      "MIFARE Classic",
      "NTAG"
    ],
    "workflows": [
      "Erase",
      "Test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rfid",
      "nfc",
      "i2c",
      "pn532",
      "mifare-classic",
      "ntag",
      "erase",
      "test"
    ],
    "cover": "assets/visual-rfid.svg",
    "time": "3 min",
    "hardware": "PN532, MIFARE Classic, NTAG",
    "takeaway": "Use RFID erase flow with PN532 for lab tags where wiping user data is expected."
  },
  {
    "title": "Record an infrared remote to a .ir file",
    "slug": "record-infrared-remote-file",
    "description": "Use INFRARED record to capture remote frames and save them to LittleFS in Flipper-style .ir format.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔴",
    "mark": "IR",
    "protocols": [
      "Infrared",
      "LittleFS"
    ],
    "targets": [
      "Remote control",
      "IR receiver"
    ],
    "workflows": [
      "Record",
      "Capture"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "infrared",
      "littlefs",
      "remote-control",
      "ir-receiver",
      "record",
      "capture"
    ],
    "cover": "assets/visual-ir-file.svg",
    "time": "3 min",
    "hardware": "Remote control, IR receiver",
    "takeaway": "Use INFRARED record to capture remote frames and save them to LittleFS in Flipper-style ."
  },
  {
    "title": "Use the infrared universal remote shell",
    "slug": "use-infrared-universal-remote-shell",
    "description": "Launch the INFRARED remote shell to send common keys such as power, volume and mute across known protocols.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📺",
    "mark": "IR",
    "protocols": [
      "Infrared"
    ],
    "targets": [
      "TV",
      "Projector",
      "IR appliance"
    ],
    "workflows": [
      "Test",
      "Remote control"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "infrared",
      "tv",
      "projector",
      "ir-appliance",
      "test",
      "remote-control"
    ],
    "cover": "assets/visual-infrared.svg",
    "time": "3 min",
    "hardware": "TV, Projector, IR appliance",
    "takeaway": "Launch the INFRARED remote shell to send common keys such as power, volume and mute across known protocols."
  },
  {
    "title": "Sweep SubGHz frequency activity",
    "slug": "sweep-subghz-frequency-activity",
    "description": "Use CC1101 SubGHz sweep to find active frequency areas before receiving or decoding frames.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📡",
    "mark": "RF",
    "protocols": [
      "SubGHz",
      "CC1101"
    ],
    "targets": [
      "Remote control",
      "OOK signal",
      "RF sensor"
    ],
    "workflows": [
      "Scan",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "remote-control",
      "ook-signal",
      "rf-sensor",
      "scan",
      "identify"
    ],
    "cover": "assets/visual-subghz.svg",
    "time": "3 min",
    "hardware": "Remote control, OOK signal, RF sensor",
    "takeaway": "Use CC1101 SubGHz sweep to find active frequency areas before receiving or decoding frames."
  },
  {
    "title": "Record a SubGHz signal to LittleFS",
    "slug": "record-subghz-signal-littlefs",
    "description": "Use SubGHz record to capture symbols from a CC1101 receiver and save them to LittleFS.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "💾",
    "mark": "RF",
    "protocols": [
      "SubGHz",
      "CC1101",
      "LittleFS"
    ],
    "targets": [
      "OOK remote",
      "RF sensor"
    ],
    "workflows": [
      "Record",
      "Capture"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "littlefs",
      "ook-remote",
      "rf-sensor",
      "record",
      "capture"
    ],
    "cover": "assets/visual-subghz.svg",
    "time": "5 min",
    "hardware": "OOK remote, RF sensor",
    "takeaway": "Use SubGHz record to capture symbols from a CC1101 receiver and save them to LittleFS."
  },
  {
    "title": "Load a Flipper SubGHz .sub file",
    "slug": "load-subghz-flipper-sub-file",
    "description": "Upload a .sub file to LittleFS and load it from SubGHz mode for CC1101 playback tests.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "📂",
    "mark": "SUB",
    "protocols": [
      "SubGHz",
      "CC1101",
      "LittleFS"
    ],
    "targets": [
      "SubGHz file",
      "CC1101"
    ],
    "workflows": [
      "Replay",
      "File workflow"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "subghz",
      "cc1101",
      "littlefs",
      "subghz-file",
      "cc1101",
      "replay",
      "file-workflow"
    ],
    "cover": "assets/visual-file-workflow.svg",
    "time": "10 min",
    "hardware": "SubGHz file, CC1101",
    "takeaway": "Upload a .sub file to LittleFS and load it from SubGHz mode for CC1101 playback tests."
  },
  {
    "title": "Start the Web CLI through Wi-Fi hotspot",
    "slug": "start-web-cli-hotspot",
    "description": "Use Wi-Fi hotspot mode to access the browser CLI without an existing router or saved credentials.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🌐",
    "mark": "WEB",
    "protocols": [
      "Wi-Fi",
      "Web CLI"
    ],
    "targets": [
      "Cardputer",
      "M5Stick",
      "ESP32-S3 board"
    ],
    "workflows": [
      "Setup",
      "Field test"
    ],
    "tools": [
      "Web CLI",
      "Wi-Fi Hotspot"
    ],
    "tags": [
      "wi-fi",
      "web-cli",
      "cardputer",
      "m5stick",
      "esp32-s3-board",
      "setup",
      "field-test"
    ],
    "cover": "assets/visual-webserial.svg",
    "time": "5 min",
    "hardware": "Cardputer, M5Stick, ESP32-S3 board",
    "takeaway": "Use Wi-Fi hotspot mode to access the browser CLI without an existing router or saved credentials."
  },
  {
    "title": "Make the first serial connection",
    "slug": "first-serial-connection-firmware",
    "description": "Connect to the ESP32 Bit Pirate CLI from a browser or desktop serial terminal and verify the welcome prompt.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔌",
    "mark": "CLI",
    "protocols": [
      "Serial CLI"
    ],
    "targets": [
      "ESP32-S3 board",
      "USB CDC port"
    ],
    "workflows": [
      "Setup",
      "First run"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "serial-cli",
      "esp32-s3-board",
      "usb-cdc-port",
      "setup",
      "first-run",
      "first-serial-connection-firmware",
      "make-the-first-serial-connection"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "3 min",
    "hardware": "ESP32-S3 board, USB CDC port",
    "takeaway": "Get to a working prompt before debugging the target hardware."
  },
  {
    "title": "Select a protocol mode quickly",
    "slug": "select-protocol-mode-cli",
    "description": "Use m, mode, or direct mode names to move from the global prompt to UART, I2C, SPI, DIO and other tools.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧭",
    "mark": "MODE",
    "protocols": [
      "General commands"
    ],
    "targets": [
      "CLI prompt"
    ],
    "workflows": [
      "Setup",
      "Navigation"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "cli-prompt",
      "setup",
      "navigation",
      "select-protocol-mode-cli",
      "select-a-protocol-mode-quickly"
    ],
    "cover": "assets/visual-protocol-lab.svg",
    "time": "1 min",
    "hardware": "CLI prompt",
    "takeaway": "Use direct mode selection instead of scrolling through menus every time."
  },
  {
    "title": "Use help and man before a workflow",
    "slug": "use-help-and-man-firmware",
    "description": "Use global help and the firmware guide before running a mode-specific command or wiring an unknown target.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "❔",
    "mark": "HELP",
    "protocols": [
      "General commands"
    ],
    "targets": [
      "CLI prompt"
    ],
    "workflows": [
      "Documentation",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "cli-prompt",
      "documentation",
      "setup",
      "use-help-and-man-firmware",
      "use-help-and-man-before-a-workflow"
    ],
    "cover": "assets/visual-education.svg",
    "time": "1 min",
    "hardware": "CLI prompt",
    "takeaway": "Find the command vocabulary from the firmware itself."
  },
  {
    "title": "Open the system status shell",
    "slug": "open-system-status-shell",
    "description": "Use the system shell to check firmware, memory, flash partitions, LittleFS, NVS and network status before debugging.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧰",
    "mark": "SYS",
    "protocols": [
      "System shell"
    ],
    "targets": [
      "ESP32-S3 board"
    ],
    "workflows": [
      "Diagnostics",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "system-shell",
      "esp32-s3-board",
      "diagnostics",
      "setup",
      "open-system-status-shell",
      "open-the-system-status-shell"
    ],
    "cover": "assets/visual-troubleshooting.svg",
    "time": "5 min",
    "hardware": "ESP32-S3 board",
    "takeaway": "Check firmware and memory state before blaming the target device."
  },
  {
    "title": "Create command aliases for common tasks",
    "slug": "create-command-alias-shortcuts",
    "description": "Use the global alias utility to create shortcuts for commands or command chains you repeat often.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "⚡",
    "mark": "ALIAS",
    "protocols": [
      "General commands"
    ],
    "targets": [
      "CLI prompt"
    ],
    "workflows": [
      "Automation",
      "Productivity"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "cli-prompt",
      "automation",
      "productivity",
      "create-command-alias-shortcuts",
      "create-command-aliases-for-common-tasks"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "5 min",
    "hardware": "CLI prompt",
    "takeaway": "Turn long bench commands into short repeatable actions."
  },
  {
    "title": "Repeat commands for quick testing",
    "slug": "repeat-commands-for-testing",
    "description": "Use repeat to run a command multiple times, such as repeated scans or GPIO pulses during a quick bench test.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔁",
    "mark": "REP",
    "protocols": [
      "General commands"
    ],
    "targets": [
      "CLI prompt",
      "GPIO pin",
      "I2C bus"
    ],
    "workflows": [
      "Automation",
      "Test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "cli-prompt",
      "gpio-pin",
      "i2c-bus",
      "automation",
      "test",
      "repeat-commands-for-testing",
      "repeat-commands-for-quick-testing"
    ],
    "cover": "assets/visual-trigger.svg",
    "time": "3 min",
    "hardware": "CLI prompt, GPIO pin, I2C bus",
    "takeaway": "Repeat a simple check without writing a script."
  },
  {
    "title": "Chain commands with delays",
    "slug": "chain-commands-with-delays-cli",
    "description": "Use command chaining with || plus delayms or delayus to create small one-line scripts directly in the CLI.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "⛓️",
    "mark": "CLI",
    "protocols": [
      "General commands"
    ],
    "targets": [
      "CLI prompt",
      "GPIO pin"
    ],
    "workflows": [
      "Automation",
      "Timing"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "cli-prompt",
      "gpio-pin",
      "automation",
      "timing",
      "chain-commands-with-delays-cli",
      "chain-commands-with-delays"
    ],
    "cover": "assets/visual-protocol-lab.svg",
    "time": "5 min",
    "hardware": "CLI prompt, GPIO pin",
    "takeaway": "Build tiny scripts from the prompt without a host-side Python file."
  },
  {
    "title": "Detect an unknown GPIO signal with wizard",
    "slug": "detect-unknown-pin-with-wizard",
    "description": "Use the global wizard command to monitor a GPIO and detect whether the pin looks digital, analog, clock-like or active.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🧙",
    "mark": "WIZ",
    "protocols": [
      "GPIO",
      "General commands"
    ],
    "targets": [
      "Unknown pin",
      "Test pad"
    ],
    "workflows": [
      "Identify",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "general-commands",
      "unknown-pin",
      "test-pad",
      "identify",
      "debug",
      "detect-unknown-pin-with-wizard",
      "detect-an-unknown-gpio-signal-with-wizard"
    ],
    "cover": "assets/visual-gpio-measure.svg",
    "time": "5 min",
    "hardware": "Unknown test pad or GPIO signal",
    "takeaway": "Use wizard as the first safe question before selecting a protocol mode."
  },
  {
    "title": "Enable smart pull-ups per mode",
    "slug": "enable-smart-pullups-per-mode",
    "description": "Use P and p to manage context-sensitive pull-ups for UART RX, HDUART IO, 1-Wire DQ, I2C SDA/SCL or SPI MISO.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "⬆️",
    "mark": "P/p",
    "protocols": [
      "General commands",
      "GPIO"
    ],
    "targets": [
      "I2C bus",
      "1-Wire device",
      "UART RX"
    ],
    "workflows": [
      "Setup",
      "Troubleshooting"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "general-commands",
      "gpio",
      "i2c-bus",
      "1-wire-device",
      "uart-rx",
      "setup",
      "troubleshooting",
      "enable-smart-pullups-per-mode",
      "enable-smart-pull-ups-per-mode"
    ],
    "cover": "assets/visual-pinout.svg",
    "time": "3 min",
    "hardware": "I2C bus, 1-Wire device, UART RX",
    "takeaway": "Use mode-aware pull-ups instead of guessing which input needs help."
  },
  {
    "title": "View GPIO logic on the device screen",
    "slug": "view-logic-on-device-screen",
    "description": "Use the global logic command to display a small logic analyzer view directly on the ESP32 screen.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📈",
    "mark": "LOG",
    "protocols": [
      "GPIO",
      "Logic"
    ],
    "targets": [
      "GPIO signal",
      "Device screen"
    ],
    "workflows": [
      "Debug",
      "Visualize"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "logic",
      "gpio-signal",
      "device-screen",
      "debug",
      "visualize",
      "view-logic-on-device-screen",
      "view-gpio-logic-on-the-device-screen"
    ],
    "cover": "assets/visual-waveform.svg",
    "time": "5 min",
    "hardware": "Device with screen, known-safe GPIO signal",
    "takeaway": "Get a quick visual signal check without opening PulseView."
  },
  {
    "title": "View analog GPIO values on screen",
    "slug": "view-analog-gpio-on-screen",
    "description": "Use analogic to read and display analog values from a GPIO directly on the ESP32 screen.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📊",
    "mark": "ADC",
    "protocols": [
      "GPIO",
      "ADC"
    ],
    "targets": [
      "Analog signal",
      "Potentiometer",
      "Sensor output"
    ],
    "workflows": [
      "Measure",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "adc",
      "analog-signal",
      "potentiometer",
      "sensor-output",
      "measure",
      "debug",
      "view-analog-gpio-on-screen",
      "view-analog-gpio-values-on-screen"
    ],
    "cover": "assets/visual-gpio-measure.svg",
    "time": "5 min",
    "hardware": "Known-safe analog signal",
    "takeaway": "Check whether a sensor output changes before writing ADC code."
  },
  {
    "title": "Listen to GPIO activity as audio",
    "slug": "listen-gpio-activity-as-audio",
    "description": "Use listen to route GPIO activity to configured I2S audio output for a quick audible signal check.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔊",
    "mark": "AUD",
    "protocols": [
      "GPIO",
      "I2S"
    ],
    "targets": [
      "GPIO signal",
      "I2S audio output"
    ],
    "workflows": [
      "Debug",
      "Signal check"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "i2s",
      "gpio-signal",
      "i2s-audio-output",
      "debug",
      "signal-check",
      "listen-gpio-activity-as-audio",
      "listen-to-gpio-activity-as-audio"
    ],
    "cover": "assets/visual-i2s.svg",
    "time": "5 min",
    "hardware": "Configured I2S output and safe GPIO signal",
    "takeaway": "Use your ears as a quick activity detector for repetitive signals."
  },
  {
    "title": "Manage LittleFS files from the Web UI",
    "slug": "manage-littlefs-files-webui",
    "description": "Use the Web UI Files panel to list, upload, download and delete files stored in ESP32 Bit Pirate flash.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📁",
    "mark": "FS",
    "protocols": [
      "LittleFS",
      "Web UI"
    ],
    "targets": [
      "ESP32-S3 board",
      "Stored files"
    ],
    "workflows": [
      "File workflow",
      "Setup"
    ],
    "tools": [
      "Web UI",
      "Web Serial Terminal"
    ],
    "tags": [
      "littlefs",
      "web-ui",
      "esp32-s3-board",
      "stored-files",
      "file-workflow",
      "setup",
      "manage-littlefs-files-webui",
      "manage-littlefs-files-from-the-web-ui"
    ],
    "cover": "assets/visual-file-workflow.svg",
    "time": "3 min",
    "hardware": "ESP32 Bit Pirate Web UI session",
    "takeaway": "Move .ir, .sub and other small workflow files without a separate filesystem tool."
  },
  {
    "title": "Open Python Scripting Lab",
    "slug": "open-python-scripting-lab",
    "description": "Run browser Python with Pyodide and Web Serial to control ESP32 Bit Pirate without installing Python locally.",
    "date": "2026-06-26",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🐍",
    "mark": "LAB",
    "protocols": [
      "Python",
      "Web Serial",
      "Serial CLI"
    ],
    "targets": [
      "ESP32 Bit Pirate",
      "Browser"
    ],
    "workflows": [
      "Automation",
      "Scripting",
      "Browser tools"
    ],
    "tools": [
      "Python Scripting Lab",
      "Web Tools"
    ],
    "tags": [
      "python",
      "python-lab",
      "web-serial",
      "browser-tools",
      "automation",
      "scripting",
      "open-python-scripting-lab"
    ],
    "cover": "assets/visual-browser-tools.svg",
    "time": "3 min",
    "hardware": "ESP32 Bit Pirate connected over USB",
    "takeaway": "Run a small Python workflow in the browser before installing or maintaining a local Python setup."
  },
  {
    "title": "Run an I2C scan from Python Scripting Lab",
    "slug": "run-i2c-scan-python-lab",
    "description": "Use Python Scripting Lab to switch ESP32 Bit Pirate into I2C mode and run a repeatable browser-driven scan.",
    "date": "2026-06-26",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🐍",
    "mark": "PY I2C",
    "protocols": [
      "I2C",
      "Python",
      "Web Serial"
    ],
    "targets": [
      "I2C device",
      "Sensor",
      "ESP32 Bit Pirate"
    ],
    "workflows": [
      "Identify",
      "Automation",
      "Scripting"
    ],
    "tools": [
      "Python Scripting Lab",
      "Web Tools"
    ],
    "tags": [
      "python",
      "python-lab",
      "i2c",
      "scan",
      "web-serial",
      "automation",
      "run-i2c-scan-python-lab"
    ],
    "cover": "assets/visual-i2c.svg",
    "time": "5 min",
    "hardware": "I2C target, jumper wires, ESP32 Bit Pirate over USB",
    "takeaway": "Turn the familiar I2C scan into a browser Python workflow you can edit, save and repeat."
  },
  {
    "title": "Use Python automation over serial",
    "slug": "use-python-automation-over-serial",
    "description": "Use serial automation from a Python script when a CLI workflow becomes repetitive or needs to run from a computer.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🐍",
    "mark": "PY",
    "protocols": [
      "Python",
      "Serial CLI"
    ],
    "targets": [
      "Host computer",
      "ESP32 Bit Pirate"
    ],
    "workflows": [
      "Automation",
      "Scripting"
    ],
    "tools": [
      "Python",
      "Serial CLI"
    ],
    "tags": [
      "python",
      "serial-cli",
      "host-computer",
      "esp32-bit-pirate",
      "automation",
      "scripting",
      "use-python-automation-over-serial"
    ],
    "cover": "assets/visual-terminal-shell.svg",
    "time": "10 min",
    "hardware": "Computer with Python and ESP32 Bit Pirate serial port",
    "takeaway": "Move from manual commands to repeatable host-side tests."
  },
  {
    "title": "Configure an nRF24L01 module",
    "slug": "configure-nrf24-module",
    "description": "Configure SPI pins, CE/CSN and the RF24 channel before using nRF24L01 send, receive or sweep commands.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📡",
    "mark": "RF24",
    "protocols": [
      "RF24",
      "SPI"
    ],
    "targets": [
      "nRF24L01 module"
    ],
    "workflows": [
      "Setup",
      "Radio lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rf24",
      "spi",
      "nrf24l01-module",
      "setup",
      "radio-lab",
      "configure-nrf24-module",
      "configure-an-nrf24l01-module"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "10 min",
    "hardware": "nRF24L01 module on SPI",
    "takeaway": "Start RF24 work by saving a known-good pin and channel configuration."
  },
  {
    "title": "Send a small RF24 payload",
    "slug": "send-rf24-payload",
    "description": "Use RF24 mode to send a short payload through an nRF24L01 module after configuration.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "📤",
    "mark": "RF24",
    "protocols": [
      "RF24"
    ],
    "targets": [
      "nRF24L01 module",
      "Second receiver"
    ],
    "workflows": [
      "Transmit",
      "Radio lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rf24",
      "nrf24l01-module",
      "second-receiver",
      "transmit",
      "radio-lab",
      "send-rf24-payload",
      "send-a-small-rf24-payload"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "5 min",
    "hardware": "Two nRF24L01 modules",
    "takeaway": "Verify a 2.4 GHz lab link with a tiny controlled payload."
  },
  {
    "title": "Receive RF24 payloads",
    "slug": "receive-rf24-payloads",
    "description": "Listen for nRF24L01 payloads on a configured RF24 channel and use the output as a simple radio lab receiver.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "📥",
    "mark": "RF24",
    "protocols": [
      "RF24"
    ],
    "targets": [
      "nRF24L01 module",
      "RF24 transmitter"
    ],
    "workflows": [
      "Receive",
      "Radio lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rf24",
      "nrf24l01-module",
      "rf24-transmitter",
      "receive",
      "radio-lab",
      "receive-rf24-payloads"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "5 min",
    "hardware": "nRF24L01 module receiving from a known transmitter",
    "takeaway": "Turn the firmware into a quick RF24 payload receiver for lab tests."
  },
  {
    "title": "Sweep RF24 channels for activity",
    "slug": "sweep-rf24-channels-activity",
    "description": "Sweep nRF24L01 channels to estimate 2.4 GHz activity before choosing a test channel.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📶",
    "mark": "RF24",
    "protocols": [
      "RF24"
    ],
    "targets": [
      "nRF24L01 module",
      "2.4 GHz lab"
    ],
    "workflows": [
      "Scan",
      "Radio lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rf24",
      "nrf24l01-module",
      "2-4-ghz-lab",
      "scan",
      "radio-lab",
      "sweep-rf24-channels-activity",
      "sweep-rf24-channels-for-activity"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "5 min",
    "hardware": "nRF24L01 module",
    "takeaway": "Choose quieter RF24 channels before transmit/receive tests."
  },
  {
    "title": "Configure an addressable LED strip",
    "slug": "configure-addressable-led-strip",
    "description": "Configure LED pin, protocol, brightness and strip length before running color tests on WS2812, SK6812, APA102 or similar LEDs.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "💡",
    "mark": "LED",
    "protocols": [
      "LED",
      "FastLED"
    ],
    "targets": [
      "Addressable LED strip"
    ],
    "workflows": [
      "Setup",
      "Visual test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "led",
      "fastled",
      "addressable-led-strip",
      "setup",
      "visual-test",
      "configure-addressable-led-strip",
      "configure-an-addressable-led-strip"
    ],
    "cover": "assets/visual-led.svg",
    "time": "3 min",
    "hardware": "Addressable RGB LED strip",
    "takeaway": "Create a safe LED baseline before testing animations or individual pixels."
  },
  {
    "title": "Set one LED pixel by index",
    "slug": "set-one-led-pixel-index",
    "description": "Use the LED set command to change a single pixel by index with named, hex or RGB colors.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🎯",
    "mark": "LED",
    "protocols": [
      "LED"
    ],
    "targets": [
      "Addressable LED strip"
    ],
    "workflows": [
      "Visual test",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "led",
      "addressable-led-strip",
      "visual-test",
      "debug",
      "set-one-led-pixel-index",
      "set-one-led-pixel-by-index"
    ],
    "cover": "assets/visual-neopixel.svg",
    "time": "3 min",
    "hardware": "Addressable RGB LED strip",
    "takeaway": "Verify LED order and indexing without animating the whole strip."
  },
  {
    "title": "Play an I2S test tone",
    "slug": "play-i2s-test-tone",
    "description": "Configure I2S pins and play a tone to verify BCLK, LRCK and DATA output wiring.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🎵",
    "mark": "I2S",
    "protocols": [
      "I2S"
    ],
    "targets": [
      "I2S DAC",
      "I2S amplifier",
      "Speaker"
    ],
    "workflows": [
      "Audio test",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2s",
      "i2s-dac",
      "i2s-amplifier",
      "speaker",
      "audio-test",
      "setup",
      "play-i2s-test-tone",
      "play-an-i2s-test-tone"
    ],
    "cover": "assets/visual-i2s.svg",
    "time": "3 min",
    "hardware": "I2S audio output device",
    "takeaway": "Verify I2S output with a known tone before playing or debugging audio data."
  },
  {
    "title": "Record from an I2S microphone",
    "slug": "record-i2s-microphone-signal",
    "description": "Use I2S record and test mic to check microphone signal strength and visualize input activity.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🎙️",
    "mark": "I2S",
    "protocols": [
      "I2S"
    ],
    "targets": [
      "I2S microphone"
    ],
    "workflows": [
      "Audio input",
      "Signal check"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "i2s",
      "i2s-microphone",
      "audio-input",
      "signal-check",
      "record-i2s-microphone-signal",
      "record-from-an-i2s-microphone"
    ],
    "cover": "assets/visual-i2s.svg",
    "time": "5 min",
    "hardware": "I2S microphone module",
    "takeaway": "Confirm I2S microphone wiring before writing custom audio code."
  },
  {
    "title": "Bridge a half-duplex UART bus",
    "slug": "open-hduart-bridge-session",
    "description": "Use HDUART bridge mode when TX and RX share a single wire, such as simple single-wire debug buses.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔁",
    "mark": "HDUART",
    "protocols": [
      "HDUART",
      "UART"
    ],
    "targets": [
      "Single-wire UART device"
    ],
    "workflows": [
      "Bridge",
      "Console"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "hduart",
      "uart",
      "single-wire-uart-device",
      "bridge",
      "console",
      "open-hduart-bridge-session",
      "bridge-a-half-duplex-uart-bus"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "3 min",
    "hardware": "Single-wire half-duplex UART target",
    "takeaway": "Talk to simple single-wire UART devices from the CLI."
  },
  {
    "title": "Send a timed HDUART query",
    "slug": "send-hduart-bytecode-query",
    "description": "Use HDUART bytecode syntax to send bytes, wait a controlled delay and read the response length.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "⏱️",
    "mark": "HDUART",
    "protocols": [
      "HDUART",
      "Bytecode"
    ],
    "targets": [
      "Single-wire UART device"
    ],
    "workflows": [
      "Request response",
      "Protocol lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "hduart",
      "bytecode",
      "single-wire-uart-device",
      "request-response",
      "protocol-lab",
      "send-hduart-bytecode-query",
      "send-a-timed-hduart-query"
    ],
    "cover": "assets/visual-trigger.svg",
    "time": "3 min",
    "hardware": "Single-wire half-duplex UART target",
    "takeaway": "Build controlled request/response tests on a shared UART line."
  },
  {
    "title": "Probe an SLE4442 smartcard ATR",
    "slug": "probe-sle4442-smartcard-atr",
    "description": "Open the 2WIRE smartcard shell and probe an SLE4442 card to read its ATR before any memory operation.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "💳",
    "mark": "2WIRE",
    "protocols": [
      "2WIRE",
      "SLE4442"
    ],
    "targets": [
      "SLE4442 smartcard"
    ],
    "workflows": [
      "Smartcard",
      "Identify"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "2wire",
      "sle4442",
      "sle4442-smartcard",
      "smartcard",
      "identify",
      "probe-sle4442-smartcard-atr",
      "probe-an-sle4442-smartcard-atr"
    ],
    "cover": "assets/visual-smartcard.svg",
    "time": "5 min",
    "hardware": "SLE4442 or compatible 2-wire smartcard",
    "takeaway": "Identify the card and ATR before dump, write or security actions."
  },
  {
    "title": "Check SLE4442 security status",
    "slug": "check-sle4442-security-status",
    "description": "Use the 2WIRE smartcard shell security check before trying PSC unlock, writes or protected memory operations.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔐",
    "mark": "2WIRE",
    "protocols": [
      "2WIRE",
      "SLE4442"
    ],
    "targets": [
      "SLE4442 smartcard"
    ],
    "workflows": [
      "Smartcard",
      "Security check"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "2wire",
      "sle4442",
      "sle4442-smartcard",
      "smartcard",
      "security-check",
      "check-sle4442-security-status"
    ],
    "cover": "assets/visual-smartcard.svg",
    "time": "5 min",
    "hardware": "SLE4442 smartcard",
    "takeaway": "Check retry counter and protection status before risky smartcard actions."
  },
  {
    "title": "Write bytes to a 93Cxx EEPROM",
    "slug": "write-93cxx-eeprom-bytes",
    "description": "Use the 3WIRE EEPROM shell to write a small test byte sequence to a 93C46, 93C56, 93C66, 93C76 or 93C86 EEPROM.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "✏️",
    "mark": "3WIRE",
    "protocols": [
      "3WIRE",
      "Microwire EEPROM"
    ],
    "targets": [
      "93Cxx EEPROM"
    ],
    "workflows": [
      "Memory write",
      "EEPROM lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "3wire",
      "microwire-eeprom",
      "93cxx-eeprom",
      "memory-write",
      "eeprom-lab",
      "write-93cxx-eeprom-bytes",
      "write-bytes-to-a-93cxx-eeprom"
    ],
    "cover": "assets/visual-threewire.svg",
    "time": "3 min",
    "hardware": "93C46/56/66/76/86 EEPROM",
    "takeaway": "Practice controlled Microwire EEPROM writes after a backup."
  },
  {
    "title": "Erase a lab 93Cxx EEPROM",
    "slug": "erase-lab-93cxx-eeprom",
    "description": "Use the 3WIRE EEPROM shell erase action on a disposable lab EEPROM after dumping its contents.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "🧽",
    "mark": "3WIRE",
    "protocols": [
      "3WIRE",
      "Microwire EEPROM"
    ],
    "targets": [
      "93Cxx EEPROM"
    ],
    "workflows": [
      "Erase",
      "EEPROM lab"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "3wire",
      "microwire-eeprom",
      "93cxx-eeprom",
      "erase",
      "eeprom-lab",
      "erase-lab-93cxx-eeprom",
      "erase-a-lab-93cxx-eeprom"
    ],
    "cover": "assets/visual-threewire.svg",
    "time": "10 min",
    "hardware": "Disposable 93Cxx EEPROM",
    "takeaway": "Practice destructive Microwire erase only on replaceable chips."
  },
  {
    "title": "Receive a specific CAN frame ID",
    "slug": "receive-specific-can-frame-id",
    "description": "Configure MCP2515 CAN mode and wait for frames matching a selected CAN identifier.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🚗",
    "mark": "CAN",
    "protocols": [
      "CAN",
      "MCP2515"
    ],
    "targets": [
      "CAN bus",
      "MCP2515 module"
    ],
    "workflows": [
      "Receive",
      "Filter"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "can",
      "mcp2515",
      "can-bus",
      "mcp2515-module",
      "receive",
      "filter",
      "receive-specific-can-frame-id",
      "receive-a-specific-can-frame-id"
    ],
    "cover": "assets/visual-can.svg",
    "time": "5 min",
    "hardware": "MCP2515 CAN module connected to a safe CAN bus",
    "takeaway": "Focus CAN logging on one identifier instead of reading every frame."
  },
  {
    "title": "Check MCP2515 CAN status",
    "slug": "check-mcp2515-can-status",
    "description": "Use the CAN status command to inspect MCP2515 controller state and error flags during bus testing.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📋",
    "mark": "CAN",
    "protocols": [
      "CAN",
      "MCP2515"
    ],
    "targets": [
      "MCP2515 module"
    ],
    "workflows": [
      "Status",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "can",
      "mcp2515",
      "mcp2515-module",
      "status",
      "debug",
      "check-mcp2515-can-status"
    ],
    "cover": "assets/visual-can.svg",
    "time": "3 min",
    "hardware": "MCP2515 CAN module",
    "takeaway": "Check controller errors before blaming the target bus."
  },
  {
    "title": "Trace one FM frequency",
    "slug": "trace-one-fm-frequency",
    "description": "Use FM trace to observe signal strength at one frequency with a Si4713 module.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📻",
    "mark": "FM",
    "protocols": [
      "FM",
      "Si4713"
    ],
    "targets": [
      "Si4713 FM module"
    ],
    "workflows": [
      "RF scan",
      "Signal check"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "fm",
      "si4713",
      "si4713-fm-module",
      "rf-scan",
      "signal-check",
      "trace-one-fm-frequency"
    ],
    "cover": "assets/visual-fm.svg",
    "time": "5 min",
    "hardware": "Si4713 FM transmitter module",
    "takeaway": "Check one FM frequency before experimenting with broadcast operations."
  },
  {
    "title": "Start a BLE HID keyboard server",
    "slug": "start-ble-hid-keyboard-server",
    "description": "Start the Bluetooth BLE HID server and send a short keyboard text string to a paired host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "⌨️",
    "mark": "BLE",
    "protocols": [
      "Bluetooth",
      "BLE HID"
    ],
    "targets": [
      "Computer",
      "Phone",
      "BLE host"
    ],
    "workflows": [
      "HID",
      "Input test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "bluetooth",
      "ble-hid",
      "computer",
      "phone",
      "ble-host",
      "hid",
      "input-test",
      "start-ble-hid-keyboard-server",
      "start-a-ble-hid-keyboard-server"
    ],
    "cover": "assets/visual-ble.svg",
    "time": "5 min",
    "hardware": "BLE-capable host paired with ESP32 Bit Pirate",
    "takeaway": "Validate BLE HID keyboard mode with a short controlled text send."
  },
  {
    "title": "Use BLE mouse movement",
    "slug": "use-ble-mouse-movement",
    "description": "Use Bluetooth HID mouse commands to send a small relative movement or click to a paired BLE host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🖱️",
    "mark": "BLE",
    "protocols": [
      "Bluetooth",
      "BLE HID"
    ],
    "targets": [
      "Computer",
      "Phone",
      "BLE host"
    ],
    "workflows": [
      "HID",
      "Input test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "bluetooth",
      "ble-hid",
      "computer",
      "phone",
      "ble-host",
      "hid",
      "input-test",
      "use-ble-mouse-movement"
    ],
    "cover": "assets/visual-ble.svg",
    "time": "5 min",
    "hardware": "BLE-capable host paired with ESP32 Bit Pirate",
    "takeaway": "Test BLE HID mouse mode without writing a custom HID sketch."
  },
  {
    "title": "Sniff 1-Wire bus timing",
    "slug": "sniff-one-wire-bus-timing",
    "description": "Use 1-Wire sniff mode to observe reset, presence and bit timing on a Dallas-style single-wire bus.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "⏱️",
    "mark": "1W",
    "protocols": [
      "1-Wire",
      "Timing"
    ],
    "targets": [
      "DS18B20",
      "iButton",
      "1-Wire EEPROM"
    ],
    "workflows": [
      "Sniff",
      "Timing"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "1-wire",
      "timing",
      "ds18b20",
      "ibutton",
      "1-wire-eeprom",
      "sniff",
      "sniff-one-wire-bus-timing",
      "sniff-1-wire-bus-timing"
    ],
    "cover": "assets/visual-1wire.svg",
    "time": "5 min",
    "hardware": "1-Wire target bus, pull-up resistor, shared ground",
    "takeaway": "Use 1-Wire sniff mode to observe reset, presence and bit timing on a Dallas-style single-wire bus."
  },
  {
    "title": "Copy an RW1990 iButton in the shell",
    "slug": "copy-rw1990-ibutton-shell",
    "description": "Open the 1-Wire iButton shell to read a Dallas key ID and write it to a compatible RW1990 test key.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔑",
    "mark": "1W",
    "protocols": [
      "1-Wire",
      "iButton"
    ],
    "targets": [
      "RW1990",
      "Dallas iButton"
    ],
    "workflows": [
      "Copy",
      "Shell"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "1-wire",
      "ibutton",
      "rw1990",
      "dallas-ibutton",
      "copy",
      "shell",
      "copy-rw1990-ibutton-shell",
      "copy-an-rw1990-ibutton-in-the-shell"
    ],
    "cover": "assets/visual-ibutton.svg",
    "time": "5 min",
    "hardware": "RW1990-compatible iButton test key and reader pads",
    "takeaway": "Open the 1-Wire iButton shell to read a Dallas key ID and write it to a compatible RW1990 test key."
  },
  {
    "title": "Write a 1-Wire scratchpad test pattern",
    "slug": "write-one-wire-scratchpad-pattern",
    "description": "Use the documented 1-Wire write sp command to send an eight-byte scratchpad pattern to a supported lab device.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "✏️",
    "mark": "1W",
    "protocols": [
      "1-Wire"
    ],
    "targets": [
      "1-Wire lab device"
    ],
    "workflows": [
      "Write",
      "Scratchpad"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "1-wire",
      "1-wire-lab-device",
      "write",
      "scratchpad",
      "write-one-wire-scratchpad-pattern",
      "write-a-1-wire-scratchpad-test-pattern"
    ],
    "cover": "assets/visual-1wire-eeprom.svg",
    "time": "5 min",
    "hardware": "Supported 1-Wire lab device",
    "takeaway": "Use the documented 1-Wire write sp command to send an eight-byte scratchpad pattern to a supported lab device."
  },
  {
    "title": "Send repeated UART text with spam",
    "slug": "send-uart-spam-pattern",
    "description": "Use UART spam to send a repeated text or newline pattern at a fixed interval while testing serial devices.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔁",
    "mark": "UART",
    "protocols": [
      "UART",
      "Serial"
    ],
    "targets": [
      "Serial device",
      "MCU console"
    ],
    "workflows": [
      "Stimulus",
      "Automation"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "serial",
      "serial-device",
      "mcu-console",
      "stimulus",
      "automation",
      "send-uart-spam-pattern",
      "send-repeated-uart-text-with-spam"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "5 min",
    "hardware": "UART target with RX/TX/GND",
    "takeaway": "Use UART spam to send a repeated text or newline pattern at a fixed interval while testing serial devices."
  },
  {
    "title": "Read UART as raw hex bytes",
    "slug": "read-uart-raw-hex-stream",
    "description": "Use UART raw mode to view serial traffic as hexadecimal bytes instead of printable ASCII.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔢",
    "mark": "UART",
    "protocols": [
      "UART",
      "Serial"
    ],
    "targets": [
      "Binary UART device"
    ],
    "workflows": [
      "Inspect",
      "Raw bytes"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "serial",
      "binary-uart-device",
      "inspect",
      "raw-bytes",
      "read-uart-raw-hex-stream",
      "read-uart-as-raw-hex-bytes"
    ],
    "cover": "assets/visual-uart.svg",
    "time": "5 min",
    "hardware": "UART target with binary or mixed output",
    "takeaway": "Use UART raw mode to view serial traffic as hexadecimal bytes instead of printable ASCII."
  },
  {
    "title": "Probe a UART device with ping",
    "slug": "probe-uart-device-with-ping",
    "description": "Use UART ping to send predefined probes and detect whether an unknown serial target answers.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📡",
    "mark": "UART",
    "protocols": [
      "UART",
      "Serial"
    ],
    "targets": [
      "Unknown UART target"
    ],
    "workflows": [
      "Probe",
      "Discovery"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "uart",
      "serial",
      "unknown-uart-target",
      "probe",
      "discovery",
      "probe-uart-device-with-ping",
      "probe-a-uart-device-with-ping"
    ],
    "cover": "assets/visual-uart-autobaud.svg",
    "time": "5 min",
    "hardware": "Unknown UART target with shared ground",
    "takeaway": "Use UART ping to send predefined probes and detect whether an unknown serial target answers."
  },
  {
    "title": "Press a USB HID gamepad button",
    "slug": "press-usb-hid-gamepad-button",
    "description": "Use USB gamepad mode to send a single HID button press such as A, B or LEFT to a connected host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🎮",
    "mark": "USB",
    "protocols": [
      "USB",
      "HID"
    ],
    "targets": [
      "Computer",
      "USB host"
    ],
    "workflows": [
      "Gamepad",
      "Input test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "usb",
      "hid",
      "computer",
      "usb-host",
      "gamepad",
      "input-test",
      "press-usb-hid-gamepad-button",
      "press-a-usb-hid-gamepad-button"
    ],
    "cover": "assets/visual-usb-hid.svg",
    "time": "5 min",
    "hardware": "ESP32-S3 native USB connected to a host",
    "takeaway": "Use USB gamepad mode to send a single HID button press such as A, B or LEFT to a connected host."
  },
  {
    "title": "Send a USB system control action",
    "slug": "send-usb-system-control-action",
    "description": "Use the USB sysctrl command to emulate hardware control button actions on a connected host.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "⏻",
    "mark": "USB",
    "protocols": [
      "USB",
      "HID"
    ],
    "targets": [
      "Computer",
      "USB host"
    ],
    "workflows": [
      "System control",
      "Input test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "usb",
      "hid",
      "computer",
      "usb-host",
      "system-control",
      "input-test",
      "send-usb-system-control-action",
      "send-a-usb-system-control-action"
    ],
    "cover": "assets/visual-usb-hid.svg",
    "time": "5 min",
    "hardware": "ESP32-S3 native USB connected to a test host",
    "takeaway": "Use the USB sysctrl command to emulate hardware control button actions on a connected host."
  },
  {
    "title": "Choose a dedicated USB adapter mode",
    "slug": "choose-dedicated-usb-adapter-mode",
    "description": "Use the USB adapters menu to reboot ESP32 Bit Pirate into flashrom, AVRDUDE, SUMP, OpenOCD, IR Toy or SubGHz tool modes.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔌",
    "mark": "USB",
    "protocols": [
      "USB",
      "Adapters"
    ],
    "targets": [
      "Host tools",
      "Browser tools"
    ],
    "workflows": [
      "Adapter mode",
      "Tooling"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "usb",
      "adapters",
      "host-tools",
      "browser-tools",
      "adapter-mode",
      "tooling",
      "choose-dedicated-usb-adapter-mode",
      "choose-a-dedicated-usb-adapter-mode"
    ],
    "cover": "assets/visual-browser-tools.svg",
    "time": "5 min",
    "hardware": "ESP32 Bit Pirate firmware with native USB",
    "takeaway": "Use the USB adapters menu to reboot ESP32 Bit Pirate into flashrom, AVRDUDE, SUMP, OpenOCD, IR Toy or SubGHz tool modes."
  },
  {
    "title": "Use ESP32 as a USB-UART dongle",
    "slug": "use-usb-uart-adapter",
    "description": "Turn ESP32 Bit Pirate into a dedicated USB-UART adapter for boot logs, serial consoles, AT modules and pyserial tools.",
    "date": "2026-06-29",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🔌",
    "mark": "USB",
    "protocols": [
      "USB",
      "UART"
    ],
    "targets": [
      "UART console",
      "AT module",
      "Microcontroller"
    ],
    "workflows": [
      "Connect & control",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Host terminal",
      "pyserial"
    ],
    "tags": [
      "beginner",
      "usb",
      "uart",
      "serial",
      "adapter",
      "cdc",
      "pyserial"
    ],
    "cover": "assets/visual-usb-adapters.svg",
    "time": "5 min",
    "hardware": "ESP32 Bit Pirate firmware, target UART device, three jumper wires",
    "takeaway": "Use an ESP32 as a practical USB-to-serial adapter when desktop tools need a CDC serial port."
  },
  {
    "title": "Configure the JTAG/SWD scan pin group",
    "slug": "configure-jtag-swd-scan-pins",
    "description": "Use JTAG config to select which GPIOs are included in SWD or JTAG pinout brute-force scans.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🧭",
    "mark": "JTAG",
    "protocols": [
      "JTAG",
      "SWD"
    ],
    "targets": [
      "Debug header",
      "Unknown board"
    ],
    "workflows": [
      "Pin discovery",
      "Scan"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "jtag",
      "swd",
      "debug-header",
      "unknown-board",
      "pin-discovery",
      "scan",
      "configure-jtag-swd-scan-pins",
      "configure-the-jtag-swd-scan-pin-group"
    ],
    "cover": "assets/visual-jtag.svg",
    "time": "5 min",
    "hardware": "Unknown debug header wired to selected GPIOs",
    "takeaway": "Use JTAG config to select which GPIOs are included in SWD or JTAG pinout brute-force scans."
  },
  {
    "title": "Run a passive Wi-Fi sniffer session",
    "slug": "run-passive-wifi-sniffer-session",
    "description": "Use Wi-Fi sniff mode to cycle channels and display nearby 2.4 GHz packet activity from the serial CLI.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "📶",
    "mark": "WiFi",
    "protocols": [
      "Wi-Fi"
    ],
    "targets": [
      "2.4 GHz Wi-Fi environment"
    ],
    "workflows": [
      "Sniff",
      "Radio survey"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "wi-fi",
      "2-4-ghz-wi-fi-environment",
      "sniff",
      "radio-survey",
      "run-passive-wifi-sniffer-session",
      "run-a-passive-wi-fi-sniffer-session"
    ],
    "cover": "assets/visual-wifi.svg",
    "time": "5 min",
    "hardware": "ESP32 Bit Pirate firmware",
    "takeaway": "Use Wi-Fi sniff mode to cycle channels and display nearby 2."
  },
  {
    "title": "Look up a MAC address vendor",
    "slug": "lookup-mac-address-vendor",
    "description": "Use Wi-Fi lookup mac to query vendor details for a MAC address while documenting devices on a bench network.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🏷️",
    "mark": "WiFi",
    "protocols": [
      "Wi-Fi",
      "Lookup"
    ],
    "targets": [
      "MAC address",
      "Network device"
    ],
    "workflows": [
      "Lookup",
      "Inventory"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "wi-fi",
      "lookup",
      "mac-address",
      "network-device",
      "inventory",
      "lookup-mac-address-vendor",
      "look-up-a-mac-address-vendor"
    ],
    "cover": "assets/visual-wifi.svg",
    "time": "5 min",
    "hardware": "Wi-Fi connection with internet access",
    "takeaway": "Use Wi-Fi lookup mac to query vendor details for a MAC address while documenting devices on a bench network."
  },
  {
    "title": "Look up IP address details",
    "slug": "lookup-ip-address-details",
    "description": "Use Wi-Fi lookup ip to get basic public IP or hostname details while checking network targets from the CLI.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "🌐",
    "mark": "WiFi",
    "protocols": [
      "Wi-Fi",
      "Lookup"
    ],
    "targets": [
      "IP address",
      "Hostname"
    ],
    "workflows": [
      "Lookup",
      "Network check"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "wi-fi",
      "lookup",
      "ip-address",
      "hostname",
      "network-check",
      "lookup-ip-address-details",
      "look-up-ip-address-details"
    ],
    "cover": "assets/visual-wifi.svg",
    "time": "5 min",
    "hardware": "Wi-Fi connection with internet access",
    "takeaway": "Use Wi-Fi lookup ip to get basic public IP or hostname details while checking network targets from the CLI."
  },
  {
    "title": "Start a Wi-Fi access point",
    "slug": "start-wifi-access-point-mode",
    "description": "Use Wi-Fi AP mode to create an access point from the firmware and optionally keep station fallback when credentials exist.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "📡",
    "mark": "WiFi",
    "protocols": [
      "Wi-Fi",
      "AP"
    ],
    "targets": [
      "Phone",
      "Laptop",
      "Lab network"
    ],
    "workflows": [
      "Access point",
      "Web CLI"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "wi-fi",
      "ap",
      "phone",
      "laptop",
      "lab-network",
      "access-point",
      "web-cli",
      "start-wifi-access-point-mode",
      "start-a-wi-fi-access-point"
    ],
    "cover": "assets/visual-wifi.svg",
    "time": "5 min",
    "hardware": "ESP32 Bit Pirate firmware and Wi-Fi client device",
    "takeaway": "Use Wi-Fi AP mode to create an access point from the firmware and optionally keep station fallback when credentials exist."
  },
  {
    "title": "Analyze a URL from the firmware CLI",
    "slug": "analyze-url-with-http-apis",
    "description": "Use Wi-Fi http analyze to run a URL analysis workflow from the firmware CLI after connecting to Wi-Fi.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "icon": "🔎",
    "mark": "HTTP",
    "protocols": [
      "Wi-Fi",
      "HTTP"
    ],
    "targets": [
      "URL",
      "Web endpoint"
    ],
    "workflows": [
      "HTTP analyze",
      "Network test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "wi-fi",
      "http",
      "url",
      "web-endpoint",
      "http-analyze",
      "network-test",
      "analyze-url-with-http-apis",
      "analyze-a-url-from-the-firmware-cli"
    ],
    "cover": "assets/visual-http-api.svg",
    "time": "5 min",
    "hardware": "Wi-Fi connection with internet access",
    "takeaway": "Use Wi-Fi http analyze to run a URL analysis workflow from the firmware CLI after connecting to Wi-Fi."
  },
  {
    "title": "Clone a MIFARE UID to a magic card",
    "slug": "clone-mifare-uid-magic-card",
    "description": "Use the RFID clone flow with a PN532 to copy a source UID to a compatible magic MIFARE Classic lab card.",
    "date": "2026-06-23",
    "author": "Geo",
    "difficulty": "Advanced",
    "type": "Recipe",
    "icon": "🪪",
    "mark": "RFID",
    "protocols": [
      "RFID",
      "PN532"
    ],
    "targets": [
      "MIFARE Classic",
      "Magic card"
    ],
    "workflows": [
      "Clone",
      "UID test"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rfid",
      "pn532",
      "mifare-classic",
      "magic-card",
      "clone",
      "uid-test",
      "clone-mifare-uid-magic-card",
      "clone-a-mifare-uid-to-a-magic-card"
    ],
    "cover": "assets/visual-rfid.svg",
    "time": "5 min",
    "hardware": "PN532 in I2C mode and compatible magic MIFARE Classic card",
    "takeaway": "Use the RFID clone flow with a PN532 to copy a source UID to a compatible magic MIFARE Classic lab card."
  },
  {
    "title": "Check logic levels before wiring a target",
    "slug": "check-logic-levels-before-wiring",
    "description": "Confirm logic voltage, shared ground and safe GPIO direction before connecting an unknown digital target to ESP32 Bit Pirate.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "icon": "⚡",
    "mark": "DIO",
    "protocols": [
      "GPIO",
      "DIO"
    ],
    "targets": [
      "Unknown digital target",
      "GPIO signal"
    ],
    "workflows": [
      "Safe setup",
      "Debug"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "gpio",
      "dio",
      "logic-level",
      "voltage",
      "shared-ground",
      "safe-wiring",
      "beginner"
    ],
    "cover": "assets/visual-gpio-measure.svg",
    "time": "3 min",
    "hardware": "ESP32 Bit Pirate board, target device and a multimeter when the signal level is unknown",
    "takeaway": "Check the electrical assumptions first: a correct command cannot protect a GPIO from an unsafe target voltage."
  },
  {
    "title": "Wire a CC1101 Sub-GHz module",
    "slug": "wire-cc1101-subghz-module",
    "description": "Wire the documented CC1101 SPI and GDO0 roles, then use SubGHz config to select the board pins.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "SubGHz",
    "protocols": [
      "SubGHz",
      "CC1101",
      "SPI"
    ],
    "targets": [
      "CC1101 module"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "cc1101",
      "subghz",
      "spi",
      "gdo0",
      "wiring",
      "pinout"
    ],
    "cover": "assets/visual-subghz.svg",
    "time": "5 min",
    "hardware": "CC1101 module and antenna",
    "takeaway": "Connect SPI plus GDO0, power the module at 3.3 V, then configure the selected board pins from the SubGHz shell."
  },
  {
    "title": "Wire an nRF24L01 RF24 module",
    "slug": "wire-nrf24l01-rf24-module",
    "description": "Wire the nRF24L01 SPI bus plus CE and CSN, then use RF24 config to assign the board pins.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "RF24",
    "protocols": [
      "RF24",
      "SPI"
    ],
    "targets": [
      "nRF24L01 module"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "rf24",
      "nrf24l01",
      "spi",
      "ce",
      "csn",
      "wiring",
      "pinout"
    ],
    "cover": "assets/visual-rf24.svg",
    "time": "5 min",
    "hardware": "nRF24L01 module",
    "takeaway": "The RF24 firmware setup requires SCK, MISO, MOSI, CE and CSN; the module itself uses 3.3 V."
  },
  {
    "title": "Wire a Si4713 FM module",
    "slug": "wire-si4713-fm-module",
    "description": "Connect the documented Si4713 I2C and reset roles, then use FM config to select the board pins.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "FM",
    "protocols": [
      "FM",
      "I2C"
    ],
    "targets": [
      "Si4713 FM module"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "si4713",
      "fm",
      "i2c",
      "reset",
      "wiring",
      "pinout"
    ],
    "cover": "assets/visual-fm.svg",
    "time": "5 min",
    "hardware": "Si4713 FM transmitter module",
    "takeaway": "FM mode config initializes the module from its SDA, SCL and reset connections."
  },
  {
    "title": "Wire a PN532 RFID reader in I2C mode",
    "slug": "wire-pn532-rfid-reader-i2c",
    "description": "Set the PN532 to I2C mode, connect SDA and SCL, then initialize it from RFID config.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "RFID",
    "protocols": [
      "RFID",
      "PN532",
      "I2C"
    ],
    "targets": [
      "PN532 reader"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "pn532",
      "rfid",
      "nfc",
      "i2c",
      "sda",
      "scl",
      "wiring"
    ],
    "cover": "assets/visual-rfid.svg",
    "time": "5 min",
    "hardware": "PN532 module and NFC/RFID tag",
    "takeaway": "The RFID firmware expects a PN532 set to I2C mode, then prompts for SDA and SCL during configuration."
  },
  {
    "title": "Wire a W5500 Ethernet module",
    "slug": "wire-w5500-ethernet-module",
    "description": "Connect the documented W5500 SPI and IRQ roles before configuring Ethernet mode.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "Ethernet",
    "protocols": [
      "Ethernet",
      "SPI"
    ],
    "targets": [
      "W5500 Ethernet module"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "w5500",
      "ethernet",
      "spi",
      "irq",
      "wiring",
      "pinout"
    ],
    "cover": "assets/visual-ethernet.svg",
    "time": "5 min",
    "hardware": "W5500 Ethernet module",
    "takeaway": "W5500 uses the SPI bus plus CS and IRQ; use the Ethernet configuration flow to map those roles to your board."
  },
  {
    "title": "Wire an MCP2515 CAN module",
    "slug": "wire-mcp2515-can-module",
    "description": "Connect the MCP2515 SPI and CAN bus roles before configuring CAN mode.",
    "date": "2026-06-24",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "CAN",
    "protocols": [
      "CAN",
      "SPI"
    ],
    "targets": [
      "MCP2515 CAN module"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "mcp2515",
      "can",
      "spi",
      "canh",
      "canl",
      "wiring"
    ],
    "cover": "assets/visual-can.svg",
    "time": "5 min",
    "hardware": "MCP2515 CAN module and authorized CAN bus",
    "takeaway": "MCP2515 uses the configured SPI roles; connect CANH/CANL only to a compatible, authorized CAN bus."
  },
  {
    "title": "Wire an SX1262 LoRa module",
    "slug": "wire-sx1262-lora-module",
    "description": "Connect SPI, RESET, BUSY and DIO1, then confirm that the radio probe reaches the ready state.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "Core1262-HF"
    ],
    "workflows": [
      "Wiring",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "lora",
      "sx1262",
      "core1262",
      "spi",
      "busy",
      "dio1",
      "wiring"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "10 min",
    "hardware": "SX1262 or compatible Core1262-style LoRa module",
    "takeaway": "SX1262 needs SPI plus RESET, BUSY and DIO1; the ready state confirms that the radio probe completed."
  },
  {
    "title": "Configure an SX1262 LoRa profile",
    "slug": "configure-sx1262-lora-radio",
    "description": "Build a known modem profile, verify every setting and estimate packet airtime.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Beginner",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "Core1262-HF"
    ],
    "workflows": [
      "Configuration",
      "Setup"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "lora",
      "sx1262",
      "frequency",
      "bandwidth",
      "spreading factor",
      "coding rate",
      "tcxo",
      "airtime"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "10 min",
    "hardware": "Configured SX1262 LoRa module and band-matched antenna",
    "takeaway": "Both LoRa endpoints need compatible frequency, BW, SF, CR, sync, CRC and IQ settings."
  },
  {
    "title": "Receive a LoRa packet with SX1262",
    "slug": "receive-lora-packet-sx1262",
    "description": "Use RSSI and CAD first, then decode a packet and inspect its signal metrics and payload.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "Controlled LoRa transmitter"
    ],
    "workflows": [
      "Receive",
      "Capture & analyze"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "lora",
      "sx1262",
      "receive",
      "rssi",
      "snr",
      "cad",
      "packet",
      "payload"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "10 min",
    "hardware": "SX1262 receiver and owned or authorized LoRa transmitter",
    "takeaway": "RSSI shows energy, CAD detects a compatible LoRa preamble, and receive decodes a complete packet."
  },
  {
    "title": "Send a LoRa packet with SX1262",
    "slug": "send-lora-packet-sx1262",
    "description": "Estimate airtime, send one controlled payload and verify it on an authorized receiver.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "Authorized LoRa receiver"
    ],
    "workflows": [
      "Transmit",
      "Connect & control"
    ],
    "tools": [
      "Serial CLI",
      "Web Serial Terminal"
    ],
    "tags": [
      "lora",
      "sx1262",
      "send",
      "tx",
      "payload",
      "hex",
      "airtime",
      "spam"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "8 min",
    "hardware": "SX1262 transmitter and owned or authorized LoRa receiver",
    "takeaway": "Preview payload and airtime, send one packet, then use peer RSSI and SNR to prove the complete link."
  },
  {
    "title": "Scan LoRa frequency activity with SX1262",
    "slug": "scan-lora-frequency-sx1262",
    "description": "Find RSSI peaks in a narrow range and inspect them with the on-device waterfall.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "Controlled RF source"
    ],
    "workflows": [
      "Scan",
      "Capture & analyze"
    ],
    "tools": [
      "Serial CLI",
      "Device display"
    ],
    "tags": [
      "lora",
      "sx1262",
      "scan",
      "waterfall",
      "rssi",
      "frequency",
      "threshold",
      "spectrum"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "12 min",
    "hardware": "SX1262 and a supported display-equipped board for waterfall",
    "takeaway": "Scan and waterfall expose RSSI energy; use CAD and receive before calling a signal compatible LoRa."
  },
  {
    "title": "Record and load a LoRa packet",
    "slug": "record-load-lora-packet-sx1262",
    "description": "Save profile, signal metrics and payload to LittleFS, then transmit the validated file once.",
    "date": "2026-07-14",
    "author": "Geo",
    "difficulty": "Intermediate",
    "type": "Recipe",
    "mark": "LoRa",
    "protocols": [
      "LoRa",
      "SPI"
    ],
    "targets": [
      "SX1262 LoRa module",
      "LittleFS .lora file"
    ],
    "workflows": [
      "Record",
      "Capture & analyze",
      "File workflow"
    ],
    "tools": [
      "Serial CLI",
      "Web UI LittleFS"
    ],
    "tags": [
      "lora",
      "sx1262",
      "record",
      "load",
      "littlefs",
      ".lora",
      "capture",
      "profile",
      "payload"
    ],
    "cover": "assets/visual-lora.svg",
    "time": "12 min",
    "hardware": "SX1262, authorized LoRa peer and LittleFS storage",
    "takeaway": "A versioned .lora file preserves the complete profile and payload; load sends once and restores the prior profile."
  }
];
