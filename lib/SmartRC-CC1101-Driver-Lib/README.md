# SmartRC-CC1101-Driver-Lib_V2.5.7

![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date-pre/LSatan/SmartRC-CC1101-Driver-Lib)

Note: Find out about the laws in your country.
Use at your own risk.

Attention! in version 2.5.7 there are changes for the internal transfer functions regarding the gdo pin assignment!

---------------------------------------------
Announcements / other
---------------------------------------------

attention: currently i can only answer questions very belatedly.
try to scour existing problems for a solution if possible.
Problems that have not yet been closed are mostly already solved!
thanks for your understanding

For debug and advanced functions: https://github.com/LSatan/CC1101-Debug-Service-Tool

new library Simu_Remote: https://github.com/LSatan/Simu_Remote_CC1101

---------------------------------------------
Install:
---------------------------------------------

Can be downloaded directly from the Arduino library manager. Just search for cc1101.

---------------------------------------------
Foreword:
---------------------------------------------
First of all, thanks to Elechouse that I can make the modified library accessible to everyone.

Link: http://www.elechouse.com/elechouse/



The library has been redesigned and some improvements have been made.

Among other things, you can now also use the internal send / receive function.

I would be happy to receive your suggestions for further examples from other libraries.

All examples included are listed in the next field.

---------------------------------------------
Containing examples:
---------------------------------------------
1.Elechouse CC1101:
-
Description: CC1101 Internal send / receive examples. Supported modulations 2-FSK, GFSK, ASK/OOK, 4-FSK, MSK.

2.Rc-Switch:
-
<p>Description: Arduino lib to operate 433/315Mhz devices like power outlet sockets.</p>
<p>Link: https://github.com/sui77/rc-switch</p>

3.NewRemoteswitch:
-
<p>Description: This library provides an easy class for Arduino, to send and receive signals used by some common "new style" 433MHz remote control switches.</p>
<p>Link: https://github.com/1technophile/NewRemoteSwitch</p>

4.RemoteSensor library:
-
<p>Description: This library provides an easy class for Arduino, to send and receive signals used by some common weather stations using -remote 433MHz sensors.</p>
<p>Link: https://github.com/mattwire/arduino-dev/tree/master/libraries/RemoteSensor</p>

5.ESPiLight:
-
<p>Description:This Arduino library is a port of the pilight 433.92MHz protocols to the Arduino platform. It was tested with a ESP8266. The aim is to transmit, receive and parse many 433.92MHz protocols, by providing a simple Arduino friendly API. This should help to implement IoT bridges between the 434MHz-RF band and internet protocols.</p>
<p>Link:https://github.com/puuu/ESPiLight</p>

---------------------------------------------
Instructions / Description:
---------------------------------------------
This driver library can be used for many libraries that use a simple RF ASK module,
with the advantages of the cc1101 module.It offers many direct setting options as in 
SmartRF Studio and calculates settings such as MHz directly.

The most important functions at a glance:

ELECHOUSE_cc1101.Init();		//Initialize the cc1101. Must be set first!

ELECHOUSE_cc1101.setBeginEndLogic(value)		// Set the same logic of begin and end the SPI instance, like the original state

ELECHOUSE_cc1101.getBeginEndLogic()				// Gives the actual mode of operation

ELECHOUSE_cc1101.setSPIinstance(&spi_instance);	// Used to share SPI bus with other devices (tft, SDCard or other devices)
												// to share SPI cus with a display (TFT_eSPI), use ELECHOUSE_cc1101.setSPIinstance(&tft.getSPIinstance());
												// if sharing with TFT_eSPI device, is neede to call some SPI Function to "release" the CC1101, like tft.
												// like: tft.drawPixel(0,0,0); after finishing setting the CC1101.
												// if none is set, it will use this lib instance, instead

ELECHOUSE_cc1101.getSPIinstance(); // Gets the SPI Instance to use with other object, like SDCard or other thing

ELECHOUSE_cc1101.setPA(PA);		//Set transmission power.

ELECHOUSE_cc1101.setMHZ(MHZ);		//Set the basic frequency.

ELECHOUSE_cc1101.SetTx();		//Set transmit on. 

ELECHOUSE_cc1101.SetTx(MHZ);		//Sets transmit on and changes the frequency.

ELECHOUSE_cc1101.SetRX();		//Set receive on.

ELECHOUSE_cc1101.SetRx(MHZ);		//Sets receive on and changes the frequency.

ELECHOUSE_cc1101.setRxBW(RXBW);		//Set Receive filter bandwidth		

ELECHOUSE_cc1101.setGDO(GDO0, GDO2); 	//Set Gdo0 (tx) and Gdo2 (rx) for serial transmission function.


ELECHOUSE_cc1101.setSpiPin(SCK, MISO, MOSI, CSN); //custom SPI pins. Set your own Spi Pins.Or to switch between multiple cc1101. Must be set before init and before changing the cc1101.

ELECHOUSE_cc1101.setChannel(chnl); 	//Set Channel from 0 to 255. default = 0(basic frequency).

ELECHOUSE_cc1101.setClb(fband, cal1, cal2); //Optionally enter Offset Callibration. Requirements: Sketch Calibrate_frequency.ino below [CC1101-Debug-Service-Tool](https://github.com/LSatan/CC1101-Debug-Service-Tool/tree/master/Calibrate_frequency).A SDR receiver and SDR software.

New in V2.5.7

ELECHOUSE_cc1101.setGDO0(GDO0);         //Sets Gdo0 for internal transfer function.

ELECHOUSE_cc1101.addGDO0(GDO0, Modul);	//Adds Gdo0 for internal transfer function (multiple cc1101).

ELECHOUSE_cc1101.addGDO(GDO0, GDO2, Modul);  //Adds Gdo0 (tx) and Gdo2 (rx) for serial transmission function (multiple cc1101).

ELECHOUSE_cc1101.addSpiPin(SCK, MISO, MOSI, CSN, Modul);  //Adds Spi Pins for multiple cc1101.

ELECHOUSE_cc1101.setModul(Modul);  //Switches between modules. from 0 to 5. So a maximum of 6.


The new features are not final and can be changed at any time.

All can also be used in a loop and are applied directly.

---------------------------------------------
Wiring:
---------------------------------------------

Notes: A logic level converter is recommended for arduino. It also works well without. Use at your own risk.

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/Nano_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/MEGA_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/Esp8266_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/Esp32_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/TXS0108E_CC1101.png"/>

<img src="https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/img/Wiring_CC1101.png"/>

---------------------------------------------
Donation
---------------------------------------------

If you like the library, I would be happy about a star.

you can support me with a donation.

https://www.paypal.me/LittleSatan666

Thank You!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.7
---------------------------------------------
13.07.2021

Driver Library		:Fixed a bug when initalizing multiple cc1101

Driver Library		:new commandos for easy handling of multiple cc1101

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.6
---------------------------------------------
06.04.2021

Driver Library		:Add NUM_PREAMBLE. Sets the minimum number of preamble bytes to be transmitted.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.5
---------------------------------------------
23.01.2021

Driver Library		:Error correction for CheckRxFifo and CheckReceiveFlag (reception internal CC1101 function).

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.4
---------------------------------------------
23.01.2021

Driver Library		:No big frequency jumps over SetRx(freq); Possible. Fixed! Big thanks to NorthernMan54 for testing!

Driver Library		:double-initaliesing option from V2.5.3 has been removed. Necessary for SetRx(freq); to use.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.3
---------------------------------------------
22.01.2021

Driver Library		:Putting rx tx has no effect if they are already in mode. Prevents unnecessary double-initaliesing.

Driver Library		:new command added: ELECHOUSE_cc1101.goSleep();   //Enter power down mode. For wakeup set RX or TX.

Driver Library		:new command added: ELECHOUSE_cc1101.getMode();   //Return the Mode. Sidle = 0, TX = 1, Rx = 2.

Driver Library		:new command added: ELECHOUSE_cc1101.setSidle();  //Set Rx / Tx Off.

Driver Library		:new command added: ELECHOUSE_cc1101.getCC1101(); //Test Spi connection and return 1 when true.

Examples		:RcSwitch Repeater_cc1101.ino has been shortened.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.2
---------------------------------------------
01.10.2020

Driver Library		:SpiWriteReg, SpiReadReg, SpiWriteBurstReg and SpiReadBurstReg change to public. Allows additional modifications from sketch.

Driver Library		:Setrx no longer has to be set to receive.(internal transmission methods)

Driver Library		:Gdo pins are now set to input / output with set gdo instead of Init.

Driver Library		:Added new sending method. Allows sending without a gdo0 pin.

notes			:The new internal send and receive methods now work completely without an additional gdo pin.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.1
---------------------------------------------
18.08.2020

Driver Library		:When changing from receiving to sending, the program freezes (internal send functions). fixed!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.5.0
---------------------------------------------
16.08.2020

Driver Library		:Rssi was calculated incorrectly.Fixed Thanks to zapquiyou!

Driver Library		:New receiving method for internal examples added. Allows several actions to be carried out in the loop.

Driver Library		:Internal Crc check added for internal examples.

Driver Library		:Simplification to send char added for internal transfer examples.

Driver Library		:A lot of new settings added for internal transmission examples. Everything can be set as in SmartRF Studio and is calculated automatically. An overview: setDeviation, setChsp, setRxBW, setDRate, setSyncMode, setSyncWord, setAdrChk, setAddr, setWhiteData, setPktFormat, setLengthConfig, setPacketLength, setCrc, setCRC_AF, setDcFilterOff, setManchester, setFEC, setPQT, setAppendStatus. description in the examples!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.4.0
---------------------------------------------
07.05.2020

Driver Library		:frequency calculator has been simplified. error-free calculations and less variables.

Driver Library		:Added frequency calibration option.

Driver Library		:Certain frequencies could not be set. Fixed! Big thanks to gusgorman402!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3.5
---------------------------------------------
14.04.2020

Driver Library		:setChsp has been removed to save space. This function is available in the Service / Debug Tool as an extended function.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3.4
---------------------------------------------
01.04.2020

Driver Library		:set AGCCTRL2 from 0x07 to 0xC7. Reception is significantly improved!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3.3
---------------------------------------------
01.04.2020

Driver Library		:set FSCTRL1 from 0x08 to 0x06. for better receive.

Driver Library		:set SpiWriteReg to public. Make it fit for debug tool!

Driver Library		:set SpiStrobe to public. Make it fit for debug tool!

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3.2
---------------------------------------------
24.03.2020

Driver Library		:Set gdo2 from input to output. Errors in libraries that use the pins directly.

examples		:Set gdo2 from setting 2 to 0 in default examples. Gdo2 is not required for these examples.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3.1
---------------------------------------------
18.03.2020

Driver Library		:ESP8266/32 core panik. Fixed! The cc1101 must be initialized before setting options!

examples		:all examples have been adapted.

examples		:internal cc1101 examples have been adapted for esp8266/32.

examples		:ESPiLight examples added.

---------------------------------------------
Changelog: SmartRC-CC1101-Driver-Lib_V2.3
---------------------------------------------
18.03.2020

Driver Library		:"RcSwitch-cc1101-driver-lib" becomes "SmartRC-cc1101-driver-lib".

Pull request		:#20 Removed unnecessary initializations when setting Tx or Rx mode. Agreed / accepted improves speed when changing rx / tx. Special thanks to fleibede.

Pull request		:#19 Added spi state after SPI pins have been set for default pins. The problem was solved differently. Special thanks to fleibede.

Pull request		:#18 Updated AVR types conditional defines. Agreed / accepted. Special thanks to fleibede.

Driver Library		:Frequency calculator certain frequencies were calculated incorrectly. fixed!

Driver Library		:Intelligent Pa Table system added. When changing the band The corresponding table is set via setTx / Rx and the frequency set to basic calibrated. All module internal transmission powers can now be set according to the band.

Driver Library		:Newly calibrated base frequency on chan 0 according to the SmartRF Studio.

Driver Library		:Channel spacing calculator can enter directly the spacing area in khz.

Driver Library		:Modulation setting added for internal transmission methods

Driver Library		:ESP32 reception improvements.

Driver Library		:Direct conversion of the RSSI level in the library.

Driver Library		:Correction of version numbering.

examples		:New examples added "RemoteSensor library".

examples		:New examples added "Elechouse CC1101 default examples"

examples		:Remote Fusion has been removed and given an extra library.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V2.2.1
---------------------------------------------
[Download Release V2.2.1](https://drive.google.com/file/d/1YHFpp2GQC96-GKcg67Tym9wt1E4CBdFc/view?usp=sharing)

20.01.2019

Driver Library		:Spi options have been removed. (Problems with ESP32) FIXED!

notes			:everything was checked again. D-SUN module on esp8266 / 32 and so on.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V2.2
---------------------------------------------
17.01.2019

Version notes		:Spi was reconfigured for fix some issuses.

Driver Library		:HotFix for cc1101 module with "8pins"(example DSun). Transmitting on Esp8266 based bords not work. FIXED!

Driver Library		:Change "SpiReadStatus" to public. Now can read RSSI Level!

Driver Library		:Spi transfer rate was limited to cc1101 max transfer rate (10mhz).

Driver Library		:After using Spi, it will now be disabled. (receive data from serial monitor and rx pin was not working on Esp 8266/32) FIXED!

Add Examples		:ReceiveDemo_Simple_with_RSSI_cc1101.ino (Shows reception strength)

Add Examples		:Frequency_Scanner_cc1101.ino (Scans Transmitting frequency of your remotes. No good results with cc1101-DSun)

Add Examples		:ProtocolAnalyzeDemo_cc1101.ino (for scan unknown protokolls. Requires: link is in sketch!) 

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V2.1
---------------------------------------------
3.1.2019

Driver Library		:Spi corrections for esp32. Spi Pin changes did not work on esp32. FIXED!

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V2.0
---------------------------------------------
18.12.2018

Driver Library		:Own Spi pin settings possible. Allows the use of other microcontrollers. Allows you to use multiple CC1101s. More information in the commands.txt

Add TXT			:driver lib commands and examples.txt

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.9.3
---------------------------------------------
27.11.2018

Driver Library		:setESP8266 has been removed. Pins are set automatically with the init command.

Driver Library		:Arduino mega SPI pins added.

Driver Library		:Added setGDO pins (For libraries that directly address the GDO pins).Like this https://github.com/EinfachArne/Somfy_Remote .
			 Example ELECHOUSE_CC1101.setGDO(2, 4); // (pin2 = GDO0, pin4 = GDO2).

Add Wiring TXT		:WIRING MEGA.txt

Add Wiring JPG		:WIRING MEGA.jpg

LICENSE			:LICENSE.txt Update.

notes			:setGDO is not compatible with RC_Switch / NewRemotSwitch.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.9.2
---------------------------------------------
16.11.2018

Fixes Remotes Fusion	:Receive mode and Repeater mode frequency, changes to the transmitter frequency after pressing on a transmitter. (RemotesFusion.ino)Fixed!

NewRemoteSwitch		:LearnCode_cc1101.ino (corrections).

Added			:Keywords.txt added.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.9.1
---------------------------------------------
07.11.2018

New Sketch		:Clear_EEPROM.ino (if there are problems with the access point password or if you want to use your ESP for other projects).

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.9
---------------------------------------------
18.10.2018

New Sketch		:RemotesFusion.ino.

New Apk´s		:RemotesFusion_V1_en.apk / RemotesFusion_V1_de.apk.

Add Docs		:Docs and manuals for RemotesFusion.

Changes			:channel spacing and channel set was removed from the examples. unnecessary because exact frequency can be set.

notes			:RemotesFusion is an application with which Rc-Switch and NewRemoteSwitch can be controlled at the same time.
			 A universal remote control APP with many possibilities. Only for Esp 8266/32 modules! For more information, watch the docs.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.8
---------------------------------------------
[Download Release V1.8](https://drive.google.com/file/d/1ADKOmW0SrUoomeoDskFrm0J2SdwCLB_O/view?usp=sharing)

02.09.2018

Driver Library		:Added pin settings for esp32.

Examples		:Added pin settings for esp32 examples.

Add Wiring JPG		:Add Wiring JPG esp32.

Add Wiring TXT		:Add Wiring TXT esp32.

Add Wiring JPG		:Add Wiring JPG esp32 for Receive_Send_Decimal_Demo_Simple.ino.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.7.2
---------------------------------------------
[Download Release V1.7.2](https://drive.google.com/file/d/1sfMvQw2JaGARTaYXZevOjB20BFOMS0tw/view?usp=sharing)

16.08.2018

Examples		:All NewRemoteSwitch examples are compatible with ESP. Exception Retransmitter_cc1101.ino.

Driver Library		:Frequency calculator calibrated. Difference -0.01 MHz. (Fixed!) Frequency is now accurate.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.7.1
---------------------------------------------
[Download Release V1.7.1](https://drive.google.com/file/d/1PXa1k0AIDY8bTMxFyyQMbOljijhIn1ke/view?usp=sharing)

10.08.2018

Driver Library		:Now you can Switch the frequency in loop. Examples: ELECHOUSE_cc1101.SetTx(433.92); and ELECHOUSE_cc1101.SetRx(433.92);.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.7
---------------------------------------------
[Download Release V1.7](https://drive.google.com/file/d/1uxACfe1ZUrkL4S3_NPVN0DmUfzbxt6oQ/view?usp=sharing)

09.08.2018

Driver Library		:Now you can enter your base frequency directly.The library calculates the settings automatically. example(433.92) 

Driver Library		:Remove freq2, freq1 and freq0 Settings.

Driver Library		:Removed unnecessary entries. the library shrank from 32,8kb(.cpp) and 9,38kb(.h) to 17,0kb(.cpp) and 7,87kb(.h).

New Example		:(NewRemoteSwitch Library) NewRemoteRepeater.ino. Repeat the Received signal 1to1. Best thanks to Roman for write it. 

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.6.1
---------------------------------------------
[Download Release V1.6.1](https://drive.google.com/file/d/1q8FV5kDnhAj1SMZf6DS0wDcm5xo4E-5-/view?usp=sharing)

05.08.2018

Examples Fix (rc-switch):Fix auto receive pulse. Received pulse don´t transmit. Fixed!(Repeater.ino and Receive_Send_Decimal_Demo_Simple.ino).

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.6
---------------------------------------------
[Download Release V1.6](https://drive.google.com/file/d/1p_iAxh7ZWlNWhFoqMM_tlLTLdyGK07-a/view?usp=sharing)

03.08.2018

Esp8266			:Compatibility for all RC switch examples.

Driver Library		:Add the GDO2 pin for Receive. Change GDO0 to send.

Wiring changes		:Description in jpg's and txt's.

Examples		:All examples have been adjusted.

Examples		:Esp and Arduino are set automatically. 

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.5
---------------------------------------------
[Download Release V1.5](https://drive.google.com/file/d/1XcGPbvI5v2PcpnVBbjtOFwtuJtx3jMK4/view?usp=sharing)

31.07.2018

New			:Supportet NewRemoteSwitch Library. For wireless sockets by trust smart home, Smappee, DI-O Chacon, Intertechno and others.
			 You want to use it? Then you need that https://github.com/1technophile/NewRemoteSwitch
			 
Add Examples		:NewRemoteSwitch Compatible examples.

Driver Library		:You can now switch directly from Tx to Rx in the loop. Without resetting the cc1101.

Add Examples		:New Repeater Example for Rc-switch. For example, to increase the range of your remote control.

Example changes		:Receive_Send_Decimal_Demo_Simple.ino (remove unnecessary commands).

Examples Fix		:Rc-Switch examples fix TypeA, TypeB and TypeD (missing from semicolon to SetTx).

Notes			:NewRemoteSwitch examples Retransmitter_cc1101.ino and LightShow_cc1101.ino only Arduino!!!

Notes			:Rc-switch example Repeater_cc1101.ino only Arduino!!!

Notes			:Rc-switch example Receive_Send_Decimal_Demo_Simple.ino only Arduino!!!

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.4
---------------------------------------------
[Download Release V1.4](https://drive.google.com/file/d/1nAPGeQSutfskirJsng44adB4sXU1YU6A/view?usp=sharing)

26.07.2018

Driver Library		:esp8266 Compatibility (Tested with Wemos d1 mini).

Driver Library		:Change default Receive filter bandwidth to 812khz (max).

Driver Library		:Change default Tx Power to PA10 (max).

Examples		:Add set esp command and pin settings.

Examples		:Diskription change.

Demo Example		:Add auto Recive and Send pulse length. (Receive_Send_Decimal_Demo_Simple.ino)

Add Wiring JPG		:Add Wiring JPG esp8266.

Add Wiring TXT		:Add Wiring TXT esp8266.

Notes			:esp8266 pin D4 don´t work with receive, Transmit and receive is set to D2.

Notes 			:esp8266 don´t work with Receive_Send_Decimal_Demo_Simple.ino (freezes when reset cc1101).

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.3.1
---------------------------------------------
[Download Release V1.3.1](https://drive.google.com/file/d/1iKmagldd14O1Boa9Z_PDpXbHRECYUPdt/view?usp=sharing)

30.03.2018:

Driver Library Fix	:no effect set by transmission power.

Driver Library		:Transmitting power can be adjusted via frequency(example in the sketches).

Examples		:Command Tx Power cleared.

Examples Fix		:set Receive filter bandwidth (command incomplete) Fixed.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.3
---------------------------------------------
[Download Release V1.3](https://drive.google.com/file/d/1q56Qewk8-Aquv1epss1gd7Gc05q4GrbO/view?usp=sharing)

25.03.2018:

Driver Library		:Add Channel List. Now you can set channel numbers 0-255.

Driver Library		:Add bandwidth can be adjusted

Driver Library		:transmission power can be adjusted

Driver Library		:can set your own frequency settings (freq2, freq1, freq0)

Driver Library		:Channel spacing can be set.

Examples		:Add set new commands

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.2 
---------------------------------------------
[Download Release V1.2](https://drive.google.com/file/d/1KLJt8ygszj9rqcttiEMt5zCnJKHc_qe3/view?usp=sharing)

20.03.2018:

Driver Library 		:Add 315Mhz Support.

Driver Library 		:Add Channel command for finetune (80khz steps) from 420mhz - 440mhz / 302mhz - 322mhz.

Examples		:Add set channel command in examples.

---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.1 
---------------------------------------------
[Download Release V1.1](https://drive.google.com/file/d/1uZzcY4uoduiUjFZzXdxA-ucLoA_TtC1f/view?usp=sharing)

19.03.2018:

Driver Library Fix	:Calibrated Frequency from 433.86 to 433.92 - 433.93 Mhz.

Driver Library		:cc1101 reset command added (allows switching between send and receive.)

New Demo Example	:Receive_Send_Decimal_Demo_Simple.ino (Sends last received decimal code)
			 To illustrate the change between RX and TX (press button = send / do not press button = receive)

Add Wiring JPG		:Wiring for Receive_Send_Decimal_Demo_Simple.ino

Add Wiring JPG		:Wiring for Nano / Uno


---------------------------------------------
Changelog: RCSwitch-CC1101-Driver-Lib_V1.0 
---------------------------------------------
[Download Release V1.0](https://drive.google.com/file/d/14538RtiEakZ_8yioXJT32XneheSjDqxi/view?usp=sharing)

18.03.2018:

cc1101 Compatibility for RC-Switch.

cc1101 RC-Switch Compatible examples.
