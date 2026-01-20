#include "WifiTypeConfigurator.h"


std::string WifiTypeConfigurator::configure(TerminalTypeEnum& terminalType) {
    switch (terminalType)
    {
    case TerminalTypeEnum::WiFiClient:
        #if defined(DEVICE_CARDPUTER)
            // Use this standalone setup for now
            setupCardputerWifi(); // endless loop until a valid WiFi is selected and connected
        #elif defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)
            // Use this standalone setup for now
            setupStickWifi(); // check stored creds
        #elif defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)
            // Use this standalone setup for now
            setupTembedWifi(); // endless loop until a valid WiFi is selected and connected
        #else
            // Use this standalone setup for now
            setupS3Wifi(); // check stored creds
        #endif

        return std::string(WiFi.localIP().toString().c_str());

    case TerminalTypeEnum::WiFiAp: {
        return "";
    }
    default:
        break;
    }

    return "";
}
