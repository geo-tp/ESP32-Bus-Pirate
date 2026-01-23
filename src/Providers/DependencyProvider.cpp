#include "DependencyProvider.h"

DependencyProvider::DependencyProvider(ITerminalView &terminalView, IDeviceView &deviceView,
                                       IInput &terminalInput, IInput &deviceInput,
                                       IUsbService &usbService, IUsbController &usbController,
                                       LittleFsService &littleFsService)
    : terminalView(terminalView),
      deviceView(deviceView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      usbService(usbService),
      usbController(usbController),
      littleFsService(littleFsService),

      // Services
      sdService(),
      nvsService(),
      ledService(),
      uartService(),
      i2cService(),
      oneWireService(),
      twoWireService(),
      threeWireService(),
      infraredService(),
      spiService(),
      pinService(),
      bluetoothService(),
      wifiService(),
      wifiScannerService(),
      i2sService(),
      sshService(),
      jtagService(),
      canService(),
      systemService(),
      ethernetService(),
      httpService(),
      telnetService(),
      modbusService(),
      subGhzService(),
      rfidService(),
      rf24Service(),

      // Transformers
      commandTransformer(),
      instructionTransformer(),
      argTransformer(),
      webRequestTransformer(),
      jsonTransformer(),
      infraredTransformer(),
      subGhzTransformer(),

      // Managers
      commandHistoryManager(),
      binaryAnalyzeManager(terminalView, terminalInput),
      userInputManager(terminalView, terminalInput, argTransformer),
      subGhzAnalyzeManager(),

      // Shells
      sdCardShell(sdService, terminalView, terminalInput, argTransformer, userInputManager),
      spiFlashShell(spiService, terminalView, terminalInput, argTransformer, userInputManager, binaryAnalyzeManager),
      spiEepromShell(spiService, terminalView, terminalInput, argTransformer, userInputManager, binaryAnalyzeManager),
      smartCardShell(twoWireService, terminalView, terminalInput, argTransformer, userInputManager),
      universalRemoteShell(terminalView, terminalInput, infraredService, argTransformer, userInputManager),
      ibuttonShell(terminalView, terminalInput, userInputManager, argTransformer, oneWireService),
      i2cEepromShell(terminalView, terminalInput, i2cService, argTransformer, userInputManager, binaryAnalyzeManager),
      uartAtShell(terminalView, terminalInput, userInputManager, argTransformer, uartService),
      threeWireEepromShell(terminalView, terminalInput, userInputManager, threeWireService, argTransformer),
      sysInfoShell(terminalView, terminalInput, userInputManager, argTransformer, systemService, wifiService),
      modbusShell(terminalView, terminalInput, argTransformer, userInputManager, modbusService),
      oneWireEepromShell(terminalView, terminalInput, oneWireService, argTransformer, userInputManager, binaryAnalyzeManager),
      guideShell(terminalView, terminalInput, userInputManager),

      // Selectors
      horizontalSelector(deviceView, deviceInput),

      // Configurators
      terminalTypeConfigurator(horizontalSelector),

      // Controllers
      uartController(terminalView, terminalInput, deviceInput, uartService, sdService, hdUartService, argTransformer, userInputManager, uartAtShell),
      i2cController(terminalView, terminalInput, i2cService, argTransformer, userInputManager, i2cEepromShell),
      oneWireController(terminalView, terminalInput, oneWireService, argTransformer, userInputManager, ibuttonShell, oneWireEepromShell),
      infraredController(terminalView, terminalInput, infraredService, littleFsService, argTransformer, infraredTransformer, userInputManager, universalRemoteShell),
      utilityController(terminalView, deviceView, terminalInput, pinService, userInputManager, argTransformer, sysInfoShell, guideShell),
      hdUartController(terminalView, terminalInput, deviceInput, hdUartService, uartService, argTransformer, userInputManager),
      spiController(terminalView, terminalInput, spiService, sdService, argTransformer, userInputManager, binaryAnalyzeManager, sdCardShell, spiFlashShell, spiEepromShell),
      jtagController(terminalView, terminalInput, jtagService, userInputManager),
      twoWireController(terminalView, terminalInput, userInputManager, twoWireService, smartCardShell),
      threeWireController(terminalView, terminalInput, userInputManager, threeWireService, argTransformer, threeWireEepromShell),
      dioController(terminalView, terminalInput, pinService, argTransformer),
      ledController(terminalView, terminalInput, ledService, argTransformer, userInputManager),
      bluetoothController(terminalView, terminalInput, deviceInput, bluetoothService, argTransformer, userInputManager),
      i2sController(terminalView, terminalInput, i2sService, argTransformer, userInputManager),
      wifiController(terminalView, terminalInput, deviceInput, wifiService, wifiScannerService, ethernetService, sshService, netcatService, nmapService, icmpService, nvsService, httpService, telnetService, argTransformer, jsonTransformer, userInputManager, modbusShell),
      canController(terminalView, terminalInput, userInputManager, canService, argTransformer),
      subGhzController(terminalView, terminalInput, deviceView, subGhzService, pinService, i2sService, littleFsService, argTransformer, subGhzTransformer, userInputManager, subGhzAnalyzeManager),
      rfidController(terminalView, terminalInput, rfidService, userInputManager, argTransformer),
      rf24Controller(terminalView, terminalInput, deviceView, rf24Service, pinService, argTransformer, userInputManager),
      ethernetController(terminalView, terminalInput, deviceInput, wifiService, wifiScannerService, ethernetService, sshService, netcatService, nmapService, icmpService, nvsService, httpService, telnetService, argTransformer, jsonTransformer, userInputManager, modbusShell)
{
}

// Accessors for core components
ITerminalView &DependencyProvider::getTerminalView() { return terminalView; }
void DependencyProvider::setTerminalView(ITerminalView &view) { terminalView = view; };
IDeviceView &DependencyProvider::getDeviceView() { return deviceView; }
IInput &DependencyProvider::getTerminalInput() { return terminalInput; }
IInput &DependencyProvider::getDeviceInput() { return deviceInput; }

// Services
SdService &DependencyProvider::getSdService() { return sdService; }
NvsService &DependencyProvider::getNvsService() { return nvsService; }
LedService &DependencyProvider::getLedService() { return ledService; }
I2cService &DependencyProvider::getI2cService() { return i2cService; }
UartService &DependencyProvider::getUartService() { return uartService; }
OneWireService &DependencyProvider::getOneWireService() { return oneWireService; }
TwoWireService &DependencyProvider::getTwoWireService() { return twoWireService; }
InfraredService &DependencyProvider::getInfraredService() { return infraredService; }
IUsbService &DependencyProvider::getUsbService() { return usbService; }
SpiService &DependencyProvider::getSpiService() { return spiService; }
HdUartService &DependencyProvider::getHdUartService() { return hdUartService; }
PinService &DependencyProvider::getPinService() { return pinService; }
WifiService &DependencyProvider::getWifiService() { return wifiService; }
BluetoothService &DependencyProvider::getBluetoothService() { return bluetoothService; }
I2sService &DependencyProvider::getI2sService() { return i2sService; }
SshService &DependencyProvider::getSshService() { return sshService; }
NetcatService &DependencyProvider::getNetcatService() { return netcatService; }
NmapService &DependencyProvider::getNmapService() { return nmapService; }
ICMPService &DependencyProvider::getICMPService() { return icmpService; }
JtagService &DependencyProvider::getJtagService() { return jtagService; }
CanService &DependencyProvider::getCanService() { return canService; }
ModbusService &DependencyProvider::getModbusService() { return modbusService; }
SystemService &DependencyProvider::getSystemService() { return systemService; }
EthernetService &DependencyProvider::getEthernetService() { return ethernetService; }
SubGhzService &DependencyProvider::getSubGhzService() { return subGhzService; }
RfidService &DependencyProvider::getRfidService() { return rfidService; }
Rf24Service &DependencyProvider::getRf24Service() { return rf24Service; }
LittleFsService &DependencyProvider::getLittleFsService() { return littleFsService; }

// Controllers
UartController &DependencyProvider::getUartController() { return uartController; }
I2cController &DependencyProvider::getI2cController() { return i2cController; }
OneWireController &DependencyProvider::getOneWireController() { return oneWireController; }
UtilityController &DependencyProvider::getUtilityController() { return utilityController; }
InfraredController &DependencyProvider::getInfraredController() { return infraredController; }
IUsbController &DependencyProvider::getUsbController() { return usbController; }
HdUartController &DependencyProvider::getHdUartController() { return hdUartController; }
SpiController &DependencyProvider::getSpiController() { return spiController; }
JtagController &DependencyProvider::getJtagController() { return jtagController; }
TwoWireController &DependencyProvider::getTwoWireController() { return twoWireController; }
ThreeWireController &DependencyProvider::getThreeWireController() { return threeWireController; }
DioController &DependencyProvider::getDioController() { return dioController; }
LedController &DependencyProvider::getLedController() { return ledController; }
WifiController &DependencyProvider::getWifiController() { return wifiController; }
BluetoothController &DependencyProvider::getBluetoothController() { return bluetoothController; }
I2sController &DependencyProvider::getI2sController() { return i2sController; }
CanController &DependencyProvider::getCanController() { return canController; }
EthernetController &DependencyProvider::getEthernetController() { return ethernetController; }
SubGhzController &DependencyProvider::getSubGhzController() { return subGhzController; }
RfidController &DependencyProvider::getRfidController() { return rfidController; }
Rf24Controller &DependencyProvider::getRf24Controller() { return rf24Controller; }

// Transformers
TerminalCommandTransformer &DependencyProvider::getCommandTransformer() { return commandTransformer; }
InstructionTransformer &DependencyProvider::getInstructionTransformer() { return instructionTransformer; }
ArgTransformer &DependencyProvider::getArgTransformer() { return argTransformer; }
WebRequestTransformer &DependencyProvider::getWebRequestTransformer() { return webRequestTransformer; }
JsonTransformer &DependencyProvider::getJsonTransformer() { return jsonTransformer; }

// Managers
CommandHistoryManager &DependencyProvider::getCommandHistoryManager() { return commandHistoryManager; }
UserInputManager &DependencyProvider::getUserInputManager() { return userInputManager; }
BinaryAnalyzeManager &DependencyProvider::getBinaryAnalyzeManager() { return binaryAnalyzeManager; }

// Shells
SdCardShell &DependencyProvider::getSdCardShell() { return sdCardShell; }
UniversalRemoteShell &DependencyProvider::getUniversalRemoteShell() { return universalRemoteShell; }
I2cEepromShell &DependencyProvider::getI2cEepromShell() { return i2cEepromShell; }
SpiFlashShell &DependencyProvider::getSpiFlashShell() { return spiFlashShell; }
SpiEepromShell &DependencyProvider::getSpiEepromShell() { return spiEepromShell; }
SmartCardShell &DependencyProvider::getSmartCardShell() { return smartCardShell; }
ThreeWireEepromShell &DependencyProvider::getThreeWireEepromShell() { return threeWireEepromShell; }
IbuttonShell &DependencyProvider::getIbuttonShell() { return ibuttonShell; }
UartAtShell &DependencyProvider::getUartAtShell() { return uartAtShell; }
SysInfoShell &DependencyProvider::getSysInfoShell() { return sysInfoShell; }

// Selectors
HorizontalSelector &DependencyProvider::getHorizontalSelector() { return horizontalSelector; }

// Config
TerminalTypeConfigurator &DependencyProvider::getTerminalTypeConfigurator() { return terminalTypeConfigurator; }

// Disable interfaces
void DependencyProvider::disableAllProtocols()
{
  // getUartService().end();
  // getHdUartService().end();
  // getI2cService().end();
  // getSpiService().end();
  // getI2sService().end();
  // getTwoWireService().end();
}
