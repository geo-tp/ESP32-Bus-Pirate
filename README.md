# ESP32 Bus Pirate

![ESP32 Bus Pirate Firmware](images/logo_protocols_banner_small.png)


**ESP32 Bus Pirate** is an open-source firmware that turns your device into a multi-protocol hacker's tool, inspired by the [legendary Bus Pirate](https://buspirate.com/).

It supports sniffing, sending, scripting, and interacting with various digital protocols (I2C, UART, 1-Wire, SPI, etc.) via a serial terminal or web-based CLI. It can also communicates with radio protocols like Bluetooth, Wi-Fi, Sub-GHz, RFID.

Use the [ESP32 Bus Pirate Web Flasher](https://geo-tp.github.io/ESP32-Bus-Pirate/webflasher/) to install the firmware in one click. See the [Wiki](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki) for step-by-step guides on every mode and command. See [ESP32 Bus Pirate Scripts](https://github.com/geo-tp/ESP32-Bus-Pirate-Scripts) for a collection of scripts.

![Demo showing the different mode of the ESP32 Bus Pirate](images/demo12.gif)

## Features

- Interactive command-line interface (CLI) via **USB Serial or WiFi Web**.
- **Modes for:**
   - [HiZ](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/01-HiZ) (default)
   - [I2C](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/05-I2C) (scan, glitch, slave mode, dump, eeprom)
   - [SPI](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/06-SPI) (eeprom, flash, sdcard, slave mode)
   - [UART](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/03-UART) / [Half-Duplex UART](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/04-HDUART) (bridge, read, write)
   - [1WIRE](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/02-1WIRE) (ibutton, eeprom)
   - [2WIRE](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/07-2WIRE) (sniff, smartcard) / [3WIRE](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/08-3WIRE) (eeprom)
   - [DIO](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/09-DIO) (Digital I/O, read, pullup, set, pwm)
   - [Infrared](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/11-INFRARED) (device-b-gone, universal remote)
   - [USB](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/12-USB) (HID, mouse, keyboard, gamepad, storage)
   - [Bluetooth](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/13-BLUETOOTH) (BLE HID, scan, spoofing, sniffing)
   - [Wi-Fi](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/14-WIFI) / [Ethernet](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/18-ETHERNET) (sniff, deauth, nmap, netcat)
   - [JTAG](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/15-JTAG) (scan pinout, SWD)
   - [LED](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/10-LED) (animations, set LEDs)
   - [I2S](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/16-I2S) (test speakers, mic, play sound)
   - [CAN](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/17-CAN) (sniff, send and receive frames)
   - [SUBGHZ](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/19-SUBGHZ) (sniff, scan, replay)
   - [RFID](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/20-RFID) (read, write, clone)
   - [RF24](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/21-RF24) (scan, sniff)


- **Protocol sniffers** for I2C, Wi-Fi, Bluetooth, 1Wire, 2wire, CAN, SubGhz.
- Baudrate **auto-detection**, AT commands and various tools for UART.
- Registers manipulation, **eeprom dump tools**, identify devices for I2C.
- Read all sort of **EEPROM, Flash** and various others tools for SPI.
- Scripting using **Bus Pirate-style bytecode** instructions or **Python**.
- Device-B-Gone command with more than **80 supported INFRARED protocols**.
- Direct I/O management, **PWM, servo**, pulse.
- Analyze radio signals and frequencies **on every bands**.
- Near than **50 addressable LEDs protocols** supported.
- **Ethernet and WiFi** are supported to access networks.
- Import and export data with the **LittleFS** over HTTP.
- Web interface with **live terminal**, or a classic **serial CLI**.

## Supported Devices


| Device               |                                     | Description                       |
|-----------------------|------------------------------------------|---------------------------------------------------|
| **ESP32 S3 Dev Kit**  | ![ESP32 S3 Dev Kit](/images/s3-devkit_s.jpg)     | More than 20 available GPIO, 1 button |
| **M5 Cardputer**      | ![Cardputer](/images/cardputer_s.png)            | 2 GPIO (Grove), screen, keyboard, mic, speaker, IR TX, SD card                |
| **M5 Cardputer ADV**  | ![Cardputer ADV](/images/cardputer-adv_s.jpg)    | 12 GPIO (Grove, Header), screen, keyboard, mic, speaker, IR TX, SD card, IMU                |
| **M5 Stick C Plus 2** | ![M5 Stick C Plus 2](/images/m5stick_s.jpg)      | 5 GPIO (Grove, Header), screen, mic, buzzer, IR TX, 3 buttons                     |
| **M5 StampS3**        | ![M5 StampS3](/images/stamps3_s.jpg)             | 9 GPIO (exposed pins), 1 button                       |
| **M5 AtomS3 Lite**    | ![Atom S3 Lite](/images/atom_s.jpg)            | 8 GPIO (Grove, Header), 1 buttton                     |
| **LILYGO T-Embed**    | ![LILYGO T-Embed](/images/tembed_s.jpg)          | 9 GPIO (Grove, Header), screen, encoder, speaker, mic, SD card                                           |
| **LILYGO T-Embed CC1101** | ![LILYGO T-Embed CC1101](/images/tembedcc1101_s.jpg) | 4 GPIO (2x Qwiic), screen, encoder, speaker, mic, SD Card, CC1101, PN532, IR TX, IR RX                                   |
| **Seeed Studio Xiao S3** | ![Xiao ESP32-S3](/images/xiaos3_s.jpg)        | 9 GPIO (exposed pins), 1 button        

- **Other ESP32-S3-based Boards**

  - All boards based on the **ESP32-S3 can be supported**, provided they have **8 MB of flash.**

  - You can [**flash the DevKit firmware**](https://geo-tp.github.io/ESP32-Bus-Pirate/webflasher/) **onto any ESP32-S3 board.**

  - Keep in mind that the **default pin mapping in the firmware may not match** your specific board.

## Getting Started

![ESP32 Bus Pirate web flasher](images/flasher.jpg)

1. 🔧 Flash the firmware  
   - Use the [ESP32 Bus Pirate Web Flasher](https://geo-tp.github.io/ESP32-Bus-Pirate/webflasher/) to burn the firmware directly from a web browser.
   - You can also burn it on [M5Burner](https://docs.m5stack.com/en/download), in the M5stick, AtomS3, M5StampS3 or Cardputer category.

2. 🔌 Connect via Serial or Web
   - Serial: any terminal app (see [Connect via Serial](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Serial))
   - Web: configure Wi-Fi and access the CLI via browser (see [Wi-Fi Connection](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/00-Terminal))

3. 🧪 Use commands like:
   ```bash
   mode
   help
   scan
   sniff
   ...
    ```
   See detailed explanations about [Terminal Commands](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki) and how each one works.

## Wiki

![ESP32 Bus Pirate Wiki](images/bus_pirate_wiki.png)

📚 Visit the **[Wiki](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki)** for detailed documentation on every mode and command.

Includes:
- [Terminal mode](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/00-Terminal) - About serial and web terminal.
- [Mode overviews](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki) - Browse supported modes.
- [Instruction syntax](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Instructions) - Master the instructions.
- [Serial setup](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Serial) - Serial access via USB.
- [Python scripting examples](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Python) - Automate tasks using Python.


The wiki is the best place to learn how everything works.

## Scripting

![ESP32 Bus Pirate Scripts](images/bus_pirate_scripts.png)

You can automate interactions with the ESP32 Bus Pirate using **Python scripts over serial.**

**Examples and ready-to-use scripts** are available in the repository: [ESP32 Bus Pirate Scripts](https://github.com/geo-tp/ESP32-Bus-Pirate-Scripts).

**Including:** Logging data in a file, eeprom and flash dump, interracting with GPIOs, LED animation...
   
## ESP32 Bus Pirate on M5 Devices
![ESP32 Bus Pirate running on M5 Stack devices](images/m5buspirate_s.jpg)

## ESP32 Bus Pirate on T-Embed
![ESP32 Bus Pirate running on M5 Stack devices](images/tembedbuspirate_s.jpg)

## Web & Serial Interfaces

The ESP32 Bus Pirate firmware provides two command-line interface (CLI) modes:

| Interface         | Advantages                                                                 | Ideal for...                          |
|------------------|-----------------------------------------------------------------------------|----------------------------------------|
| **Web Interface** | - Accessible from any browser<br>- Works over Wi-Fi<br>- No cables needed | Quick tests, demos, headless setups   |
| **Serial Interface** | - Faster performance<br>- Instant responsiveness<br>- Handles large data smoothly | Intensive sessions, frequent interactions |


Both interfaces share the same command structure and can be used interchangeably ([more details](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/00-Terminal)).

![An iPhone screenshot showing Bus Pirate web cli](images/presentation_mobile.png)

## Using the ESP32 Bus Pirate to speak UART over WiFi
![Using the ESP32 Bus pirate with UART](images/demo2.gif)

## Contribute
See [How To Contribute](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Contribute) section, which outlines a **simple way to add a new command** to any mode.

## Visuals Assets

#### ![ESP32 Bus Pirate Firmware](images/logo_square_small.png)

[Images, logo, presentations, illustrations](https://github.com/geo-tp/ESP32-Bus-Pirate/wiki/99-Visual-Assets)

These visuals can be **freely used in blog posts, documentation, videos, or articles** to help explain and promote the firmware.


## Warning
> ⚠️ **Voltage Warning**: Devices should only operate at **3.3V** or **5V**.  
> - Do **not** connect peripherals using other voltage levels — doing so may **damage your ESP32**.

> ⚠️ **Usage Warning**: This firmware is provided for **educational, diagnostic, and interoperability testing purposes only**.
> - Do not use it to interfere with, probe, or manipulate devices without proper authorization.
> - Avoid any unauthorized RF transmissions (e.g., sub-GHz) that could violate local regulations or disrupt networks and communications.
> - The authors are not responsible for any misuse of this software or hardware, including legal consequences resulting from unauthorized access or signal emission.
> - Always stay within the bounds of your country’s laws and responsible disclosure policies.


