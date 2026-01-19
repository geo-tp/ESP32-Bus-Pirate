#ifndef UNIT_TEST

#include <Views/SerialTerminalView.h>
#include <Views/M5DeviceView.h>
#include <Views/WebTerminalView.h>
#include <Views/NoScreenDeviceView.h>
#include <Views/TembedDeviceView.h>
#include <Views/CardputerTerminalView.h>
#include <Views/CardputerDeviceView.h>
#include <Inputs/SerialTerminalInput.h>
#include <Inputs/CardputerInput.h>
#include <Inputs/StickInput.h>
#include <Inputs/StampS3Input.h>
#include <Inputs/TembedInput.h>
#include <Inputs/S3DevKitInput.h>
#include <Providers/DependencyProvider.h>
#include <Dispatchers/ActionDispatcher.h>
#include <Servers/HttpServer.h>
#include <Servers/WebSocketServer.h>
#include <Inputs/WebTerminalInput.h>
#include <Selectors/HorizontalSelector.h>
#include <Config/TerminalTypeConfigurator.h>
#include <Config/WifiTypeConfigurator.h>
#include <Config/UsbConfigurator.h>
#include <Enums/TerminalTypeEnum.h>
#include <States/GlobalState.h>

/*
This file initializes the device (M5Stick / Cardputer / StampS3 / T-Embed / S3 DevKit),
selects the terminal mode (Serial, Wi-Fi Client/AP, Standalone Cardputer),
and then launches the main loop through the ActionDispatcher.

- Terminal View: the interface where the user SEES and INTERACTS with the CLI.
    * SerialTerminalView    -> text terminal via USB serial (COM/tty).
    * WebTerminalView       -> text terminal in a browser (via WebSocket).
    * CardputerTerminalView -> Cardputer LCD acts as the terminal screen.

- Device View: the interface for device’s screen (if any).
    * M5DeviceView, TembedDeviceView, CardputerDeviceView, NoScreenDeviceView, etc.
    * Used for UI elements like mode, pinout mapping, or logic traces.

- Terminal Input: how the user TYPES commands into the system.
    * SerialTerminalInput  -> keyboard input over USB serial.
    * WebTerminalInput     -> keystrokes/events from a browser WebSocket.

- Device Input: physical buttons on the device.
    * StickInput, TembedInput, StampS3Input, S3DevKitInput -> button/encoders.

- ActionDispatcher: the central loop that reads user actions,
  dispatches them to controllers/services, and keeps the system running.

- DependencyProvider: constructs and wires together the correct Views,
  Inputs, Services, and Controllers based on the current configuration.
*/


void setup() {    
    #if defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)
        // Setup the Stick
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5.begin(cfg);
        M5DeviceView deviceView;
        deviceView.setRotation(3);
        StickInput deviceInput;
    #elif DEVICE_CARDPUTER
        // Setup the Cardputer
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5Cardputer.begin(cfg, true);
        M5DeviceView deviceView;
        deviceView.setRotation(1);
        CardputerInput deviceInput;
    #elif DEVICE_M5STAMPS3
        // Setup the StampS3/AtomS3
        #include <M5Unified.h>
        auto cfg = M5.config();
        M5.begin(cfg);
        NoScreenDeviceView deviceView;
        StampS3Input deviceInput;
    #elif defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)
        // Setup the T-embed
        TembedDeviceView deviceView;
        TembedInput deviceInput;
    #else
        // Fallback to S3 dev kit
        NoScreenDeviceView deviceView;
        S3DevKitInput deviceInput;
    #endif

    deviceView.logo();
    GlobalState& state = GlobalState::getInstance();
    LittleFsService littleFsService;

    // Select the terminal type
    HorizontalSelector selector(deviceView, deviceInput);
    TerminalTypeConfigurator configurator(selector);
    TerminalTypeEnum terminalType = configurator.configure();
    state.setTerminalMode(terminalType);
    std::string webIp = "0.0.0.0";

    // Configure Wi-Fi if needed
    if (terminalType == TerminalTypeEnum::WiFiClient) {
        WifiTypeConfigurator wifiTypeConfigurator(deviceView, deviceInput);
        webIp = wifiTypeConfigurator.configure(terminalType);
        
        if (webIp == "0.0.0.0") {
            terminalType = TerminalTypeEnum::Serial;
        } else {
            state.setTerminalIp(webIp);
        }
    }

    switch (terminalType) {
        case TerminalTypeEnum::Serial: {
            // Serial View/Input
            SerialTerminalView serialView;
            SerialTerminalInput serialInput;
            
            // Baudrate
            auto baud = std::to_string(state.getSerialTerminalBaudRate());
            serialView.setBaudrate(state.getSerialTerminalBaudRate());

            // Configure USB
            auto usb = UsbConfigurator::configure(serialView, serialInput, deviceInput);

            // Build the provider for serial type and run the dispatcher loop
            // too big to fit on the stack anymore, allocated on the heap
            DependencyProvider* provider = new DependencyProvider(serialView, deviceView, serialInput, deviceInput, 
                                                                  usb.usbService, usb.usbController, littleFsService);
            ActionDispatcher dispatcher(*provider);
            dispatcher.setup(terminalType, baud);
            dispatcher.run(); // Forever
            break;
        }
        case TerminalTypeEnum::WiFiAp: // Not Yet Implemented
        case TerminalTypeEnum::WiFiClient: {
            // Configure Server
            httpd_handle_t server = nullptr;
            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            if (httpd_start(&server, &config) != ESP_OK) {
                return;
            }

            JsonTransformer jsonTransformer;
            HttpServer httpServer(server, littleFsService, jsonTransformer);
            WebSocketServer wsServer(server);

            // Web View/Input
            WebTerminalView webView(wsServer);
            WebTerminalInput webInput(wsServer);
            deviceView.loading();
            delay(6000); // let the server begin
            
            // Setup routes for index and ws
            wsServer.setupRoutes();
            httpServer.setupRoutes();

            // Configure USB
            auto usb = UsbConfigurator::configure(webView, webInput, deviceInput);

            // Build the provider for webui type and run the dispatcher loop
            // too big to fit on the stack anymore, allocated on the heap
            DependencyProvider* provider = new DependencyProvider(webView, deviceView, webInput, deviceInput, 
                                                                  usb.usbService, usb.usbController, littleFsService);
            ActionDispatcher dispatcher(*provider);
            
            dispatcher.setup(terminalType, webIp);
            dispatcher.run(); // Forever
            break;
        }

        #ifdef DEVICE_CARDPUTER
        case TerminalTypeEnum::Standalone: 
            // Cardputer all in one
            CardputerTerminalView standaloneView; // cardputer screen as terminal
            CardputerInput standaloneInput; // cardputer keyboard for command input
            standaloneView.initialize();
            CardputerDeviceView deviceView; // used for logic analyzer only
            S3DevKitInput deviceInput; // the G0 button of the cardputer

            // Configure USB
            auto usb = UsbConfigurator::configure(standaloneView, standaloneInput, deviceInput);

            // Build the provider for cardputer standalone and run the dispatcher loop
            DependencyProvider* provider = new DependencyProvider(standaloneView, deviceView, standaloneInput, deviceInput, 
                                                                usb.usbService, usb.usbController, littleFsService);
            ActionDispatcher dispatcher(*provider);
            dispatcher.setup(terminalType, "standalone");
            dispatcher.run(); // Forever
            break;
        #endif
    }
}

void loop() {
    // Empty as all logic is handled in dispatcher
}

#endif