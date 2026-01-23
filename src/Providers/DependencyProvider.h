#pragma once

/*
The DependencyProvider is responsible for creating, holding,
and injecting shared instances of core components
(such as services, controllers, etc) throughout the application.
*/

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IDeviceView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IUsbController.h"
#include "Interfaces/IUsbService.h"
#include "Services/SdService.h"
#include "Services/NvsService.h"
#include "Services/LedService.h"
#include "Services/UartService.h"
#include "Services/I2cService.h"
#include "Services/OneWireService.h"
#include "Services/InfraredService.h"
#include "Services/UsbS3Service.h"
#include "Services/HdUartService.h"
#include "Services/SpiService.h"
#include "Services/PinService.h"
#include "Services/BluetoothService.h"
#include "Services/WifiService.h"
#include "Services/WifiOpenScannerService.h"
#include "Services/I2sService.h"
#include "Services/SshService.h"
#include "Services/JtagService.h"
#include "Services/CanService.h"
#include "Services/SystemService.h"
#include "Services/ThreeWireService.h"
#include "Services/EthernetService.h"
#include "Services/HttpService.h"
#include "Services/TelnetService.h"
#include "Services/ModbusService.h"
#include "Services/SubGhzService.h"
#include "Services/RfidService.h"
#include "Services/Rf24Service.h"
#include "Services/LittleFsService.h"
#include "Controllers/UartController.h"
#include "Controllers/I2cController.h"
#include "Controllers/OneWireController.h"
#include "Controllers/UtilityController.h"
#include "Controllers/UsbS3Controller.h"
#include "Controllers/HdUartController.h"
#include "Controllers/JtagController.h"
#include "Controllers/SpiController.h"
#include "Controllers/TwoWireController.h"
#include "Controllers/ThreeWireController.h"
#include "Controllers/InfraredController.h"
#include "Controllers/DioController.h"
#include "Controllers/LedController.h"
#include "Controllers/BluetoothController.h"
#include "Controllers/WifiController.h"
#include "Controllers/I2sController.h"
#include "Controllers/CanController.h"
#include "Controllers/SubGhzController.h"
#include "Controllers/EthernetController.h"
#include "Controllers/RfidController.h"
#include "Controllers/Rf24Controller.h"
#include "Transformers/TerminalCommandTransformer.h"
#include "Transformers/InstructionTransformer.h"
#include "Transformers/ArgTransformer.h"
#include "Transformers/JsonTransformer.h"
#include "Transformers/WebRequestTransformer.h"
#include "Transformers/SubGhzTransformer.h"
#include "Managers/CommandHistoryManager.h"
#include "Managers/BinaryAnalyzeManager.h"
#include "Managers/UserInputManager.h"
#include "Managers/SubGhzAnalyzeManager.h"
#include "Shells/SdCardShell.h"
#include "Shells/UniversalRemoteShell.h"
#include "Shells/I2cEepromShell.h"
#include "Shells/SpiFlashShell.h"
#include "Shells/SpiEepromShell.h"
#include "Shells/SmartCardShell.h"
#include "Shells/ThreeWireEepromShell.h"
#include "Shells/IbuttonShell.h"
#include "Shells/UartAtShell.h"
#include "Shells/SysInfoShell.h"
#include "Shells/ModbusShell.h"
#include "Shells/OneWireEepromShell.h"
#include "Shells/GuideShell.h"
#include "Config/TerminalTypeConfigurator.h"

class DependencyProvider
{
public:
    DependencyProvider(ITerminalView &terminalView, IDeviceView &deviceView,
                       IInput &terminalInput, IInput &deviceInput,
                       IUsbService &usbService,
                       IUsbController &usbController,
                       LittleFsService &littleFsService);

    // Core Components
    ITerminalView &getTerminalView();
    void setTerminalView(ITerminalView &view);
    IDeviceView &getDeviceView();
    IInput &getTerminalInput();
    IInput &getDeviceInput();

    // Services
    SdService &getSdService();
    NvsService &getNvsService();
    LedService &getLedService();
    UartService &getUartService();
    I2cService &getI2cService();
    OneWireService &getOneWireService();
    TwoWireService &getTwoWireService();
    ThreeWireService& getThreeWireService();
    InfraredService &getInfraredService();
    IUsbService &getUsbService();
    SpiService &getSpiService();
    HdUartService &getHdUartService();
    PinService &getPinService();
    BluetoothService &getBluetoothService();
    WifiService &getWifiService();
    WifiOpenScannerService &getWifiScannerService();
    I2sService &getI2sService();
    SshService &getSshService();
    NetcatService &getNetcatService();
    NmapService &getNmapService();
    ICMPService &getICMPService();
    JtagService &getJtagService();
    CanService &getCanService();
    SystemService &getSystemService();
    EthernetService &getEthernetService();
    HttpService &getHttpService();
    TelnetService &getTelnetService();
    ModbusService &getModbusService();
    SubGhzService &getSubGhzService();
    RfidService &getRfidService();
    Rf24Service &getRf24Service();
    LittleFsService &getLittleFsService();

    // Controllers
    UartController &getUartController();
    I2cController &getI2cController();
    UtilityController &getUtilityController();
    OneWireController &getOneWireController();
    InfraredController &getInfraredController();
    IUsbController &getUsbController();
    HdUartController &getHdUartController();
    SpiController &getSpiController();
    JtagController &getJtagController();
    TwoWireController &getTwoWireController();
    ThreeWireController &getThreeWireController();
    DioController &getDioController();
    LedController &getLedController();
    BluetoothController &getBluetoothController();
    WifiController &getWifiController();
    I2sController &getI2sController();
    CanController &getCanController();
    EthernetController &getEthernetController();
    SubGhzController &getSubGhzController();
    RfidController &getRfidController();
    Rf24Controller &getRf24Controller();

    // Transformers
    TerminalCommandTransformer &getCommandTransformer();
    InstructionTransformer &getInstructionTransformer();
    ArgTransformer &getArgTransformer();
    WebRequestTransformer &getWebRequestTransformer();
    JsonTransformer &getJsonTransformer();
    InfraredRemoteTransformer &getInfraredTransformer();
    SubGhzTransformer &getSubGhzTransformer();

    // Managers
    CommandHistoryManager &getCommandHistoryManager();
    UserInputManager &getUserInputManager();
    BinaryAnalyzeManager &getBinaryAnalyzeManager();
    SubGhzAnalyzeManager &getSubGhzAnalyzeManager();

    // Shells
    SdCardShell &getSdCardShell();
    UniversalRemoteShell &getUniversalRemoteShell();
    I2cEepromShell &getI2cEepromShell();
    SpiFlashShell &getSpiFlashShell();
    SpiEepromShell &getSpiEepromShell();
    SmartCardShell &getSmartCardShell();
    ThreeWireEepromShell &getThreeWireEepromShell();
    IbuttonShell &getIbuttonShell();
    UartAtShell &getUartAtShell();
    SysInfoShell &getSysInfoShell();
    ModbusShell &getModbusShell();
    OneWireEepromShell &getOneWireEepromShell();    
    GuideShell &getGuideShell();

    // Selectors
    HorizontalSelector &getHorizontalSelector();

    // Config
    TerminalTypeConfigurator &getTerminalTypeConfigurator();

    // Disable
    void disableAllProtocols();

private:
    // Core Components
    ITerminalView &terminalView;
    IDeviceView &deviceView;
    IInput &terminalInput;
    IInput &deviceInput;
    IUsbService &usbService;
    IUsbController &usbController;
    LittleFsService &littleFsService;

    // Services
    SdService sdService;
    NvsService nvsService;
    LedService ledService;
    UartService uartService;
    I2cService i2cService;
    OneWireService oneWireService;
    TwoWireService twoWireService;
    ThreeWireService threeWireService;
    InfraredService infraredService;
    HdUartService hdUartService;
    SpiService spiService;
    PinService pinService;
    WifiService wifiService;
    WifiOpenScannerService wifiScannerService;
    BluetoothService bluetoothService;
    I2sService i2sService;
    SshService sshService;
    NetcatService netcatService;
    NmapService nmapService;
    ICMPService icmpService;
    JtagService jtagService;
    CanService canService;
    SystemService systemService;
    EthernetService ethernetService;
    HttpService httpService;
    TelnetService telnetService;
    ModbusService modbusService;
    SubGhzService subGhzService;
    RfidService rfidService;
    Rf24Service rf24Service;

    // Controllers
    UartController uartController;
    I2cController i2cController;
    OneWireController oneWireController;
    UtilityController utilityController;
    InfraredController infraredController;
    HdUartController hdUartController;
    SpiController spiController;
    JtagController jtagController;
    TwoWireController twoWireController;
    ThreeWireController threeWireController;
    DioController dioController;
    LedController ledController;
    WifiController wifiController;
    BluetoothController bluetoothController;
    I2sController i2sController;
    CanController canController;
    EthernetController ethernetController;
    SubGhzController subGhzController;
    RfidController rfidController;
    Rf24Controller rf24Controller;

    // Transformers
    TerminalCommandTransformer commandTransformer;
    InstructionTransformer instructionTransformer;
    ArgTransformer argTransformer;
    WebRequestTransformer webRequestTransformer;
    JsonTransformer jsonTransformer;
    InfraredRemoteTransformer infraredTransformer;
    SubGhzTransformer subGhzTransformer;

    // Managers
    CommandHistoryManager commandHistoryManager;
    UserInputManager userInputManager;
    BinaryAnalyzeManager binaryAnalyzeManager;
    SubGhzAnalyzeManager subGhzAnalyzeManager;

    // Shells
    SdCardShell sdCardShell;
    UniversalRemoteShell universalRemoteShell;
    I2cEepromShell i2cEepromShell;
    SpiFlashShell spiFlashShell;
    SpiEepromShell spiEepromShell;
    SmartCardShell smartCardShell;
    ThreeWireEepromShell threeWireEepromShell;
    IbuttonShell ibuttonShell;
    UartAtShell uartAtShell;
    SysInfoShell sysInfoShell;
    ModbusShell modbusShell;
    OneWireEepromShell oneWireEepromShell;
    GuideShell guideShell;

    // Selectors
    HorizontalSelector horizontalSelector;

    // Config
    TerminalTypeConfigurator terminalTypeConfigurator;
};