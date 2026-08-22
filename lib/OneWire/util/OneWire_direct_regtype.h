#ifndef OneWire_Direct_RegType_h
#define OneWire_Direct_RegType_h

#include <stdint.h>

// Platform specific I/O register type

#if defined(__AVR__)
#define IO_REG_TYPE uint8_t

#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK66FX1M0__) || defined(__MK64FX512__)
#define IO_REG_TYPE uint8_t

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
#define IO_REG_TYPE uint32_t

#elif defined(__MKL26Z64__)
#define IO_REG_TYPE uint8_t

#elif defined(__SAM3X8E__) || defined(__SAM3A8C__) || defined(__SAM3A4C__)
#define IO_REG_TYPE uint32_t

#elif defined(__PIC32MX__)
#define IO_REG_TYPE uint32_t

#elif defined(ARDUINO_ARCH_ESP8266)
#define IO_REG_TYPE uint32_t

#elif defined(ARDUINO_ARCH_ESP32)
#define IO_REG_TYPE uint32_t
#define IO_REG_MASK_ATTR

#elif defined(ARDUINO_ARCH_STM32)
#define IO_REG_TYPE uint32_t

#elif defined(__SAMD21G18A__)
#define IO_REG_TYPE uint32_t

#elif defined(__ASR6501__)
#define IO_REG_TYPE uint32_t

#elif defined(RBL_NRF51822)
#define IO_REG_TYPE uint32_t

#elif defined(__arc__) /* Arduino101/Genuino101 specifics */
#define IO_REG_TYPE uint32_t

#elif defined(__MBED__)
#include "DigitalInOut.h"
#define IO_REG_TYPE mbed::DigitalInOut*

#elif defined(__riscv)
#define IO_REG_TYPE uint32_t

#else
#define IO_REG_TYPE unsigned int

#endif
#endif
