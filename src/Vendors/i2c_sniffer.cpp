/**
 * @AUTHOR Ákos Szabó (Whitehawk Tailor) - aaszabo@gmail.com
 * 
 * This is an I2C sniffer that logs traffic on I2C BUS.
 * 
 * It is not part of the I2C BUS. It is neither a Master, nor a Slave and puts no data to the lines.
 * It just listens and logs the communication.
 * 
 * Two pins as imput are attached to SDC and SDA lines.
 * Since the I2C communications runs on 400kHz so,
 * the tool that runs this program should be fast.
 * This was tested on an ESP32 bord Heltec WiFi Lora32 v2
 * ESP32 core runs on 240MHz.
 * It means there are 600 ESP32 cycles during one I2C clock tick.
 *
 * 
 * The program uses interrupts to detect
 * the raise edge of the SCL - bit transfer 
 * the falling edge of SDA if SCL is HIGH- START
 * the raise edge of SDA if SCL id HIGH - STOP 
 * 
 * In the interrupt routines there is just a few line of code
 * that mainly sets the status and stores the incoming bits.
 * Otherwise the program gets timeout panic in interrupt handler and
 * restart the CPU.
 * 
 * 
 * REWORKED for ESP32 Bus Pirate
 * 
 *                   https://github.com/WhitehawkTailor/I2C-sniffer/
 */

#include "i2c_sniffer.h"
#include <Arduino.h>
#include "driver/gpio.h"

// --- Minimal internal notes (added): ISR pushes compact events into an event ring.
// The main context converts events into ASCII and exposes them via read()/available().

static uint8_t sniffer_scl_pin = 1; // override by i2c_sniffer_begin()
static uint8_t sniffer_sda_pin = 2;

#define I2C_IDLE 0
#define I2C_TRX  2

// ---- Event tags (4 bits) ----
#define TAG_START 0x1
#define TAG_STOP  0x2
#define TAG_DATA  0x3
#define TAG_ADDR  0x4
#define TAG_ACK   0x5

static inline uint16_t PACK_EVENT(uint16_t tag, uint16_t value) {
    return (uint16_t)((tag << 12) | (value & 0x0FFF));
}
static inline uint8_t  EVENT_TAG(uint16_t ev) { return (uint8_t)((ev >> 12) & 0x0F); }
static inline uint16_t EVENT_VAL(uint16_t ev) { return (uint16_t)(ev & 0x0FFF); }

// ---- Event ring (ISR -> main) ----
#define EVENT_RING_ORDER 11
#define EVENT_RING_SIZE  (1u << EVENT_RING_ORDER)   // 2048 events
#define EVENT_RING_MASK  (EVENT_RING_SIZE - 1u)
static volatile uint16_t eventRing[EVENT_RING_SIZE];
static volatile uint16_t eventW = 0;
static volatile uint16_t eventR = 0;

// ---- Output char ring (main -> user read()) ----
#define CHAR_RING_ORDER 13
#define CHAR_RING_SIZE  (1u << CHAR_RING_ORDER)     // 8192 chars
#define CHAR_RING_MASK  (CHAR_RING_SIZE - 1u)
static volatile char charRing[CHAR_RING_SIZE];
static volatile uint16_t charW = 0;
static volatile uint16_t charR = 0;

// ---- I2C state (ISR) ----
static volatile uint8_t i2cStatus = I2C_IDLE;
static volatile uint8_t bitCount = 0;
static volatile uint8_t currentByte = 0;
static volatile uint16_t byteCountInFrame = 0;
static volatile uint8_t expectingAck = 0;

// Optional counters (kept, cheap)
static volatile uint16_t falseStart = 0;
static volatile uint16_t sclUpCnt = 0;
static volatile uint16_t sdaUpCnt = 0;
static volatile uint16_t sdaDownCnt = 0;

// ---- Helpers (ISR-safe) ----
static inline void IRAM_ATTR push_event(uint16_t ev) {
    uint16_t next = (uint16_t)((eventW + 1u) & EVENT_RING_MASK);
    if (next == eventR) { // full -> drop oldest
        eventR = (uint16_t)((eventR + 1u) & EVENT_RING_MASK);
    }
    eventRing[eventW] = ev;
    eventW = next;
}

static inline uint8_t IRAM_ATTR fast_gpio_read(uint8_t pin) {
    return (uint8_t)gpio_get_level((gpio_num_t)pin);
}

// ---- ISR handlers ----
void IRAM_ATTR i2cTriggerOnRaisingSCL() {
    sclUpCnt++;

    if (i2cStatus == I2C_IDLE) {
        falseStart++;
    }

    if (expectingAck) {
        uint8_t ackBit = fast_gpio_read(sniffer_sda_pin); // 0 = ACK, 1 = NACK
        push_event(PACK_EVENT(TAG_ACK, (uint16_t)ackBit));
        expectingAck = 0;
        bitCount = 0;
        currentByte = 0;
        return;
    }

    uint8_t bit = fast_gpio_read(sniffer_sda_pin);
    currentByte = (uint8_t)((currentByte << 1) | (bit & 0x01));
    bitCount++;

    if (bitCount >= 8) {
        uint16_t tag = (byteCountInFrame == 0) ? TAG_ADDR : TAG_DATA;
        push_event(PACK_EVENT(tag, currentByte));
        byteCountInFrame++;
        expectingAck = 1;
    }
}

void IRAM_ATTR i2cTriggerOnChangeSDA() {
    uint8_t s1 = fast_gpio_read(sniffer_sda_pin);
    uint8_t s2 = fast_gpio_read(sniffer_sda_pin);
    if (s1 != s2) s1 = s2;

    if (s1) {
        sdaUpCnt++;
        uint8_t scl = fast_gpio_read(sniffer_scl_pin);
        if (i2cStatus != I2C_IDLE && scl == 1) {
            push_event(PACK_EVENT(TAG_STOP, 0));
            i2cStatus = I2C_IDLE;
            bitCount = 0;
            currentByte = 0;
            expectingAck = 0;
            byteCountInFrame = 0;
        }
    } else {
        sdaDownCnt++;
        uint8_t scl = fast_gpio_read(sniffer_scl_pin);
        if (i2cStatus == I2C_IDLE && scl == 1) {
            push_event(PACK_EVENT(TAG_START, 0));
            i2cStatus = I2C_TRX;
            bitCount = 0;
            currentByte = 0;
            expectingAck = 0;
            byteCountInFrame = 0;
        }
    }
}

// ---- API ----
void i2c_sniffer_begin(uint8_t scl, uint8_t sda) {
    sniffer_scl_pin = scl;
    sniffer_sda_pin = sda;
}

static void reset_state_and_buffers() {
    noInterrupts();
    // I2C state
    i2cStatus = I2C_IDLE;
    bitCount = 0;
    currentByte = 0;
    byteCountInFrame = 0;
    expectingAck = 0;
    // rings
    eventW = eventR = 0;
    charW  = charR  = 0;
    interrupts();
}

void i2c_sniffer_setup() {
    pinMode(sniffer_scl_pin, INPUT_PULLUP);
    pinMode(sniffer_sda_pin, INPUT_PULLUP);
    reset_state_and_buffers();
    attachInterrupt(digitalPinToInterrupt(sniffer_scl_pin), i2cTriggerOnRaisingSCL, RISING);
    attachInterrupt(digitalPinToInterrupt(sniffer_sda_pin), i2cTriggerOnChangeSDA, CHANGE);
}

void i2c_sniffer_stop() {
    detachInterrupt(digitalPinToInterrupt(sniffer_scl_pin));
    detachInterrupt(digitalPinToInterrupt(sniffer_sda_pin));
    noInterrupts();
    i2cStatus = I2C_IDLE;
    interrupts();
}

// ---- Output char ring helpers (main context) ----
static inline void char_push(char c) {
    uint16_t next = (uint16_t)((charW + 1u) & CHAR_RING_MASK);
    if (next == charR) { // full -> drop oldest
        charR = (uint16_t)((charR + 1u) & CHAR_RING_MASK);
    }
    charRing[charW] = c;
    charW = next;
}

static void text_push_str(const char* s) {
    while (*s) { char_push(*s++); }
}

static void text_push_hex8(uint8_t v) {
    const char* hex = "0123456789ABCDEF";
    char_push('0'); char_push('x');
    char_push(hex[(v >> 4) & 0x0F]);
    char_push(hex[(v     ) & 0x0F]);
}

// ---- Convert pending events into ASCII (called on demand) ----
// Format example:
//   [S]                      (START)
//   ADDR 0x3C W <ACK>
//   0x00 <ACK> 0xAB <ACK>  (DATA ...)
//   [P]\n                    (STOP + newline)
static void pump_events_to_text() {
    // Process a bounded number of events per call to avoid long critical sections
    // (added) keeps latency low while ensuring steady drain.
    const uint16_t MAX_EVENTS_PER_PUMP = 128;
    uint16_t processed = 0;

    while (processed < MAX_EVENTS_PER_PUMP) {
        uint16_t ev;
        noInterrupts();
        if (eventR == eventW) { interrupts(); break; }
        ev = eventRing[eventR];
        eventR = (uint16_t)((eventR + 1u) & EVENT_RING_MASK);
        interrupts();

        uint8_t tag = EVENT_TAG(ev);
        uint16_t val = EVENT_VAL(ev);

        switch (tag) {
            case TAG_START:
                text_push_str("[S] ");
                break;

            case TAG_STOP:
                text_push_str("[P]");
                char_push('\r\n'); // newline to separate frames
                break;

            case TAG_ADDR: {
                uint8_t b = (uint8_t)val;
                uint8_t addr = (uint8_t)((b >> 1) & 0x7F);
                uint8_t rw   = (uint8_t)(b & 0x01);
                text_push_str("ADDR ");
                text_push_hex8(addr);
                char_push(' ');
                char_push(rw ? 'R' : 'W');
                char_push(' ');
                break;
            }

            case TAG_DATA: {
                uint8_t b = (uint8_t)val;
                text_push_hex8(b);
                char_push(' ');
                break;
            }

            case TAG_ACK:
                text_push_str((val == 0) ? "<ACK> " : "<NACK> ");
                break;

            default:
                // ignore
                break;
        }
        processed++;
    }
}

bool i2c_sniffer_available() {
    // If no chars ready, try pumping events into ASCII
    if (charR == charW) {
        pump_events_to_text();
    }
    return (charR != charW);
}

char i2c_sniffer_read() {
    // Ensure at least one char is present
    if (charR == charW) {
        pump_events_to_text();
    }
    char out = '\0';
    if (charR != charW) {
        out = charRing[charR];
        charR = (uint16_t)((charR + 1u) & CHAR_RING_MASK);
    }
    return out;
}

void i2c_sniffer_reset_buffer() {
    reset_state_and_buffers();
}
