#include "UartEmulationShell.h"
#include <Arduino.h>
#include <cstdio>
#include <cctype>

UartEmulationShell::UartEmulationShell(
    ITerminalView& view,
    IInput& input,
    UartService& uartService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager
)
: terminalView(view),
  terminalInput(input),
  uartService(uartService),
  argTransformer(argTransformer),
  userInputManager(userInputManager) {}


void UartEmulationShell::run() {
    terminalView.println("\n=== UART Emulator ===");
    terminalView.println("Emulate various UART devices to test things.");
    terminalView.println("Using current configuration from settings.\n");

    while (true) {

        int choice = userInputManager.readValidatedChoiceIndex(
            "Select device to emulate",
            kDevices,
            kDeviceCount,
            kDeviceCount - 1 // Exit by default
        );

        if (choice == 0) {
            emulateGps();
        }
        else if (choice == 1) {
            emulateBoot();
        }
        else if (choice == 2) {
            emulateShell(false);
        }
        else if (choice == 3) {
            emulateShell(true);
        }
        else if (choice == 4) {
            emulateAt();
        }
        else {
            terminalView.println("Exiting UART Emulator...\n");
            break; // Exit
        }
    }
}

void UartEmulationShell::startEmulation() {
    uartService.clearUartBuffer();
    uartService.flush();
    terminalView.println("\nUART Emulator: Running... Press [ENTER] to stop.\n");
}

void UartEmulationShell::emulateGps() {
    startEmulation();

    terminalView.println("GPS emulator started... Sending NMEA sentences\n");

    // Start somewhere 
    double lat = 45.376992; //  N 
    double lon = 5.647038;  // E

    uint32_t lastMs = 0;
    uint32_t tick = 0;

    // Base time/date
    int hh = 3, mm = 16, ss = 38;
    const char* dateDDMMYY = "030226";
    const char* zdaDD = "03";
    const char* zdaMM = "02";
    const char* zdaYYYY = "2026";

    while (true) {
        if (shouldStopByEnter()) break;

        uint32_t now = millis();
        if (now - lastMs < 1000) {
            delay(5);
            continue;
        }
        lastMs = now;

        // ---- time ----
        ss++;
        if (ss >= 60) { ss = 0; mm++; }
        if (mm >= 60) { mm = 0; hh++; }
        if (hh >= 24) { hh = 0; }

        char timeField[16];
        snprintf(timeField, sizeof(timeField), "%02d%02d%02d.000", hh, mm, ss);

        // ---- position offset ----
        lat += 0.00001;  // ~1.1m
        lon += 0.00002;

        // Convert deg -> ddmm.mmmmm (LAT) / dddmm.mmmmm (LON)
        auto toLatField = [](double d, char* out, size_t n, char* hemi) {
            double a = (d < 0) ? -d : d;
            int deg = (int)a;
            double min = (a - deg) * 60.0;
            snprintf(out, n, "%02d%08.5f", deg, min);
            *hemi = (d < 0) ? 'S' : 'N';
        };
        auto toLonField = [](double d, char* out, size_t n, char* hemi) {
            double a = (d < 0) ? -d : d;
            int deg = (int)a;
            double min = (a - deg) * 60.0;
            snprintf(out, n, "%03d%08.5f", deg, min);
            *hemi = (d < 0) ? 'W' : 'E';
        };

        char latField[20], lonField[20];
        char latH, lonH;
        toLatField(lat, latField, sizeof(latField), &latH);
        toLonField(lon, lonField, sizeof(lonField), &lonH);

        // speed
        float speedKn = 6.0f + (float)((tick % 20) - 10) * 0.12f;    // 4.8 .. 7.2
        if (speedKn < 0.0f) speedKn = 0.0f;
        float courseT = 275.0f + (float)((tick % 30) - 15) * 0.55f;  // ~266 .. 283 etc
        while (courseT < 0) courseT += 360.0f;
        while (courseT >= 360.0f) courseT -= 360.0f;
        float speedKmh = speedKn * 1.852f;

        // ---- payloads  ----

        // BDGSV
        const char* bdgsv_short = "BDGSV,1,1,02,09,20,045,17,22,79,067,26";

        // RMC
        char rmc[220];
        snprintf(rmc, sizeof(rmc),
                 "GNRMC,%s,A,%s,%c,%s,%c,%.2f,%.2f,%s,,,A",
                 timeField, latField, latH, lonField, lonH, speedKn, courseT, dateDDMMYY);

        // VTG
        char vtg[180];
        snprintf(vtg, sizeof(vtg),
                 "GNVTG,%.2f,T,,M,%.2f,N,%.2f,K,A",
                 courseT, speedKn, speedKmh);

        // ZDA
        char zda[120];
        snprintf(zda, sizeof(zda),
                 "GNZDA,%s,%s,%s,%s,00,00",
                 timeField, zdaDD, zdaMM, zdaYYYY);

        // TXT
        const char* txt = "GPTXT,01,01,01,ANTENNA OPEN";

        // GGA
        char gga[220];
        snprintf(gga, sizeof(gga),
                 "GNGGA,%s,%s,%c,%s,%c,1,05,5.4,0.0,M,0.0,M,,",
                 timeField, latField, latH, lonField, lonH);

        // GLL : lat, N, lon, E, time, A, A
        char gll[200];
        snprintf(gll, sizeof(gll),
                 "GNGLL,%s,%c,%s,%c,%s,A,A",
                 latField, latH, lonField, lonH, timeField);

        // GSA
        const char* gpgsa = "GPGSA,A,3,07,09,30,,,,,,,,,,7.5,5.4,5.2";
        const char* bdgsa = "BDGSA,A,3,09,22,,,,,,,,,,,7.5,5.4,5.2";

        // GSV
        const char* gpgsv1 = "GPGSV,3,1,09,05,48,306,12,06,05,193,,07,57,059,18,09,19,087,16";
        const char* gpgsv2 = "GPGSV,3,2,09,11,29,228,19,13,30,275,,14,20,154,,21,77,269,21";
        const char* gpgsv3 = "GPGSV,3,3,09,30,82,148,27";

        // ---- Emission ----
        uartService.println(nmeaWrap(bdgsv_short));
        uartService.println(nmeaWrap(rmc));
        uartService.println(nmeaWrap(vtg));
        uartService.println(nmeaWrap(zda));
        uartService.println(nmeaWrap(txt));

        uartService.println(nmeaWrap(gga));
        uartService.println(nmeaWrap(gll));
        uartService.println(nmeaWrap(gpgsa));
        uartService.println(nmeaWrap(bdgsa));
        uartService.println(nmeaWrap(gpgsv1));
        uartService.println(nmeaWrap(gpgsv2));
        uartService.println(nmeaWrap(gpgsv3));

        uartService.println(nmeaWrap(bdgsv_short));

        tick++;
    }

    terminalView.println("GPS emulation stopped.\n");
}

void UartEmulationShell::emulateBoot() {
    startEmulation();

    terminalView.println("Boot started... Sending uboot basic sequence.\n");
    
    uartService.println("");
    const char* bootLines[] = {
        "U-Boot 2020.10 (Feb 03 2026 - 12:34:56 +0000)",
        "",
        "CPU:   ARMv7 Processor [410fc075] revision 5 (ARMv7)",
        "Model: Generic Embedded Board",
        "DRAM:  512 MiB",
        "MMC:   mmc@1: 0, mmc@2: 1",
        "Loading Environment from MMC... OK",
        "In:    serial",
        "Out:   serial",
        "Err:   serial",
        "Net:   eth0",
        "Hit ESC key to stop autoboot:  3",
        "Hit ESC key to stop autoboot:  2",
        "Hit ESC key to stop autoboot:  1",
        "",
        "Starting kernel ...",
        "",
        "[    0.000000] Linux version 5.10.0",
        "[    0.120000] init: mounting rootfs...",
        "[    0.240000] init: starting services...",
        "[    0.300000] system ready",
        ""
    };

    uartService.println("Boot started");

    bool escPressed = false;

    // BOOT SEQUENCE
    for (size_t i = 0; i < sizeof(bootLines)/sizeof(bootLines[0]); ++i) {

        // check stop terminal side
        if (shouldStopByEnter()) {
            terminalView.println("Boot sequence stopped.\n");
            return;
        }
        
        // check ESC uart side
        int c0 = uartService.read();
        if (c0 == 27) { // ESC
            uartService.println("Boot sequence stopped.\n ");
            escPressed = true;
            break;
        }

        uartService.println(bootLines[i]);

        // delay while checking for ENTER or ESC
        uint32_t waitMs = (i >= 11 && i <= 14) ? 1000 : 100;
        uint32_t t0 = millis();

        while ((millis() - t0) < waitMs) {

            if (shouldStopByEnter()) {
                terminalView.println("Boot sequence stopped.\n");
                return;
            }

            int c = terminalInput.readChar();
            if (c == 27) { // ESC
                uartService.println("Boot sequence stopped.\n ");
                escPressed = true;
                // exit both loops
                i = sizeof(bootLines)/sizeof(bootLines[0]);
                break;
            }

            delay(5);
        }

        if (escPressed) break;
    }

    // LOGIN PROMPT
    if (escPressed) {

        uartService.print("login: ");

        std::string line;

        while (true) {

            if (shouldStopByEnter()) {
                terminalView.println("Boot sequence stopped.\n");
                return;
            }

            if (!uartService.available()) {
                delay(5);
                continue;
            }

            line = uartService.readLine();
            if (line == "root") break;
            
            uartService.println("Login incorrect");
            uartService.print("login: ");
        }

        uartService.print("Password: ");

        while (true) {

            if (shouldStopByEnter()) {
                terminalView.println("Boot sequence stopped.\n");
                return;
            }

            if (!uartService.available()) {
                delay(5);
                continue;
            }

            line = uartService.readLine();
            if (line == "esp32buspirate") break;

            uartService.println("Login incorrect, quitting boot...");
            return;
        }

        uartService.println("Login success. Welcome root.");
        uartService.print("uboot> ");
    }

    terminalView.println("Boot sequence finished.\n");
}

void UartEmulationShell::emulateShell(bool protectedShell) {
    startEmulation();

    terminalView.println("Shell emulator started... Waiting for commands.\n");

    uartService.println("Welcome to the UART Shell.");

    // Auth if protected (login + password)
    if (protectedShell) {
        std::string line;

        // LOGIN
        uartService.print("login: ");
        while (true) {
            if (shouldStopByEnter()) {
                terminalView.println("Shell emulation stopped.\n");
                return;
            }
            if (!uartService.available()) {
                delay(5);
                continue;
            }

            line = uartService.readLine();

            // trim
            auto isSpace = [](unsigned char c){ return std::isspace(c); };
            while (!line.empty() && isSpace((unsigned char)line.front())) line.erase(line.begin());
            while (!line.empty() && isSpace((unsigned char)line.back()))  line.pop_back();

            if (line == "root") break;

            uartService.println("Login incorrect");
            uartService.print("login: ");
        }

        // PASSWORD
        uartService.print("Password: ");
        while (true) {
            if (shouldStopByEnter()) {
                terminalView.println("Shell emulation stopped.\n");
                return;
            }
            if (!uartService.available()) {
                delay(5);
                continue;
            }

            line = uartService.readLine();

            // trim
            auto isSpace = [](unsigned char c){ return std::isspace(c); };
            while (!line.empty() && isSpace((unsigned char)line.front())) line.erase(line.begin());
            while (!line.empty() && isSpace((unsigned char)line.back()))  line.pop_back();

            if (line == "esp32buspirate") break;

            uartService.println("Login incorrect");
            uartService.print("login: ");
            while (true) {
                if (shouldStopByEnter()) {
                    terminalView.println("Shell emulation stopped.\n");
                    return;
                }
                if (!uartService.available()) {
                    delay(5);
                    continue;
                }

                line = uartService.readLine();

                while (!line.empty() && isSpace((unsigned char)line.front())) line.erase(line.begin());
                while (!line.empty() && isSpace((unsigned char)line.back()))  line.pop_back();

                if (line == "root") break;
                uartService.print("login: ");
            }

            uartService.print("Password: ");
        }

        uartService.println("Login success.");
    }

    // SHELL LOOP
    uartService.print("> ");
    while (true) {
        if (shouldStopByEnter()) break;

        if (!uartService.available()) {
            delay(5);
            continue;
        }

        std::string line = uartService.readLine();

        if (line.empty()) {
            uartService.print("> ");
            continue;
        }

        auto isSpace = [](unsigned char c){ return std::isspace(c); };
        while (!line.empty() && isSpace((unsigned char)line.front())) line.erase(line.begin());
        while (!line.empty() && isSpace((unsigned char)line.back()))  line.pop_back();

        if (line == "help" || line == "?") {
            uartService.println("Commands: help, version, status, echo <txt>, exit");
        }
        else if (line == "version") {
            uartService.println("UartEmuShell v1");
        }
        else if (line == "status") {
            uartService.println("OK");
        }
        else if (line.rfind("echo ", 0) == 0) {
            uartService.println(line.substr(5));
        }
        else if (line == "exit") {
            uartService.println("bye");
            break;
        }
        else {
            uartService.println("unknown");
        }

        uartService.print("> ");
    }

    terminalView.println("Shell emulation stopped.\n");
}

void UartEmulationShell::emulateAt() {
    startEmulation();

    terminalView.println("AT modem emulator started... Waiting for commands.\n");

    uartService.println("Type: AT, ATI, AT+GMR, AT+RST, AT+CSQ, AT+CREG?, ATD..., ATH, AT+CMGF=1");
    uartService.print("OK\r\n");

    while (true) {
        if (shouldStopByEnter()) break;

        if (!uartService.available()) {
            delay(5);
            continue;
        }

        std::string line = uartService.readLine();

        // trim
        auto isSpace = [](unsigned char c){ return std::isspace(c); };
        while (!line.empty() && isSpace((unsigned char)line.front())) line.erase(line.begin());
        while (!line.empty() && isSpace((unsigned char)line.back()))  line.pop_back();

        if (line.empty()) continue;

        // Many modems accept lowercase/uppercase
        std::string u = line;
        std::transform(u.begin(), u.end(), u.begin(), [](unsigned char c){ return (char)std::toupper(c); });

        // basic commands
        if (u == "AT" || u == "ATZ") {
            uartService.println("OK");
        }
        else if (u == "ATI") {
            uartService.println("Manufacturer: UART-EMU");
            uartService.println("Model: AT Modem");
            uartService.println("Revision: 1.0");
            uartService.println("OK");
        }
        else if (u == "AT+GMR") {
            uartService.println("AT-EMU 1.0.0");
            uartService.println("OK");
        }
        else if (u == "AT+RST") {
            uartService.println("OK");
            uartService.println("REBOOTING...");
            delay(200);
            uartService.println("READY");
        }
        else if (u == "ATH") {
            uartService.println("OK");
        }
        else if (u.rfind("ATD", 0) == 0) {
            // Dial
            uartService.println("CONNECT 9600");
        }
        else if (u == "AT+CSQ") {
            // Signal quality
            uartService.println("+CSQ: 20,0");
            uartService.println("OK");
        }
        else if (u == "AT+CREG?") {
            // network registration
            uartService.println("+CREG: 0,1");
            uartService.println("OK");
        }
        else if (u.rfind("AT+CMGF=", 0) == 0) {
            // SMS text mode
            uartService.println("OK");
        }
        else if (u.rfind("AT+CMGS=", 0) == 0) {
            uartService.print("> ");
            while (true) {
                if (shouldStopByEnter()) {
                    terminalView.println("\nAT modem emulation stopped.\n");
                    return;
                }
                if (uartService.available()) {
                    (void)uartService.readLine();
                    break;
                }
                delay(5);
            }
            uartService.println("+CMGS: 1");
            uartService.println("OK");
        }
        else {
            uartService.println("ERROR");
        }
    }

    terminalView.println("AT modem emulation stopped.\n");
}

bool UartEmulationShell::shouldStopByEnter() {
    int c = terminalInput.readChar();
    if (c == KEY_NONE) return false;
    return (c == '\r' || c == '\n');
}

uint8_t UartEmulationShell::nmeaChecksum(const char* payload) {
    uint8_t cs = 0;
    for (const char* p = payload; *p; ++p) {
        cs ^= (uint8_t)(*p);
    }
    return cs;
}

std::string UartEmulationShell::nmeaWrap(const char* payload) {
    char buf[200];
    uint8_t cs = nmeaChecksum(payload);
    snprintf(buf, sizeof(buf), "$%s*%02X", payload, cs);
    return std::string(buf);
}