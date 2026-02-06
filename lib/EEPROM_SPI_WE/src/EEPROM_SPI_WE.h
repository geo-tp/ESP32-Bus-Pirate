/******************************************************************************
* This is a library for SPI EEPROMs like the 25LCxxx, 25Axxx, etc.
*
* In case you like it would be cool if you give it a star.
* 
* If you find bugs, please inform me!
* 
* Written by Wolfgang (Wolle) Ewald
*
* Here you find the detailed documentation:
* https://wolles-elektronikkiste.de/en/eeprom-part-3-external-spi-eeproms   (English)
* https://wolles-elektronikkiste.de/eeprom-teil-3-externe-spi-eeproms       (German)
*
********************************************************************************/

#ifndef EEPROM_SPI_WE_H_
#define EEPROM_SPI_WE_H_

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <SPI.h>

typedef enum EEPROM_WE_PAGE_SIZE {
    EEPROM_PAGE_SIZE_16, EEPROM_PAGE_SIZE_32, EEPROM_PAGE_SIZE_64, EEPROM_PAGE_SIZE_128, EEPROM_PAGE_SIZE_256
} eeprom_pageSize;

typedef enum EEPROM_WE_WRITE_PROTECT {
    PROTECT_NONE, PROTECT_UPPER_QUARTER, PROTECT_UPPER_HALF, PROTECT_ALL
} eeprom_writeProtect;

enum eeprom_size_t {    
    EEPROM_KBITS_2 = 256,
    EEPROM_KBITS_4 = 512,
    EEPROM_KBITS_8 = 1024,
    EEPROM_KBITS_16 = 2028,
    EEPROM_KBITS_32 = 4096,
    EEPROM_KBITS_64 = 8192,
    EEPROM_KBITS_128 = 16384,
    EEPROM_KBITS_256 = 32768,
    EEPROM_KBITS_512 = 65536,
    EEPROM_KBITS_1024 = 131072,
    EEPROM_KBITS_2048 = 262144,
    EEPROM_KBITS_4096 = 524288,
};

class EEPROM_SPI_WE
{
    public:
        static constexpr uint8_t EEP_READ {0x03};  // write
        static constexpr uint8_t EEP_WRITE{0x02};  // read
        static constexpr uint8_t EEP_WREN {0x06};  // write enable
        static constexpr uint8_t EEP_WRDI {0x04};  // write disable
        static constexpr uint8_t EEP_RDSR {0x05};  // read status register
        static constexpr uint8_t EEP_WRSR {0x01};  // write status register
        static constexpr uint8_t EEP_PE   {0x42};  // page erase
        static constexpr uint8_t EEP_SE   {0xD8};  // sector erase
        static constexpr uint8_t EEP_CE   {0xC7};  // chip erase
        static constexpr uint8_t EEP_RDID {0xAB};  // release from deep power down and read electronic signature
        static constexpr uint8_t EEP_DPD  {0xB9};  // deep power down mode
        static constexpr uint8_t EEP_A8   {0x08};  // A8 address bit for small EEPROMs
        static constexpr uint8_t EEP_A9   {0x10};  // A9 address bit for small EEPROMs
        
        EEPROM_SPI_WE(uint16_t cs, uint16_t wp = 255, uint32_t sc = 8000000) : _spi{&SPI}, csPin{cs}, wpPin{wp}, spiClock{sc} {}
        EEPROM_SPI_WE(SPIClass *s, uint16_t cs, uint16_t wp = 255, uint32_t sc = 8000000) : _spi{s}, csPin{cs}, wpPin{wp}, spiClock{sc} {}
        EEPROM_SPI_WE() : _spi{&SPI}, csPin{5}, wpPin{255}, spiClock{8000000} {}
        bool init(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin, uint8_t cs, uint8_t wp);
        bool probe();
        uint32_t getMemorySize(); //neu
        void setPageSize(eeprom_pageSize eps);
        void setMemorySize(eeprom_size_t size); // neu
        void deepPowerDown();
        uint8_t powerUpAndReadID();
        void erasePage(uint32_t addr);
        void eraseSector(uint32_t addr);
        void eraseCompleteEEPROM();
        void write(uint32_t addr, uint8_t val);
        uint8_t read(uint32_t addr);
        void writeProtect(eeprom_writeProtect sectors);
        void protectStatusRegister(bool protect = true);
        
        template <typename T>
        T &get(uint32_t addr, T &t){
            uint8_t *data = (uint8_t *)&t;
            readEEPROM(addr, data, sizeof(T)); 
            return t;
        }

        template <typename T>
        const T &put(uint32_t addr, const T &t){ 
            const uint8_t *data = (const uint8_t *)&t;
            writeEEPROM(addr, data, sizeof(T)); 
            return t;
        }
        
        uint32_t putString(uint32_t addr, String &strToWrite);
        void getString(uint32_t addr, String &strToRead);
        
        void continuousPutEnable(uint32_t addr);
        void continuousPutDisable();
        
        template <typename T>
        const T &continuousPut(const T &t){ 
            const uint8_t *data = (const uint8_t *)&t;
            continuousWriteEEPROM(data, sizeof(T)); 
            return t;
        }
        
        bool isBusy();
        uint8_t eepromReadStatusReg(); 
        void setSPIClockSpeed(unsigned long clock);
        void setSmallEEPROM();
            
    protected:
        SPIClass *_spi;
        SPISettings mySPISettings;
        uint16_t csPin;
        uint16_t wpPin;
		uint32_t spiClock;
        uint16_t pageSize;
        uint32_t contAddr;
        bool smallEEPROM;
        uint32_t memSize;
            
        void eepromWriteEnable();
        void eepromWriteStatusReg(uint8_t cmd);
        void writeEEPROM(uint32_t addr, const uint8_t *buf, uint16_t sizeOfBuf);
        void continuousWriteEEPROM(const uint8_t *buf, uint16_t sizeOfBuf);
        void readEEPROM(uint32_t addr, uint8_t *buf, uint16_t sizeOfBuf);   
};

#endif


