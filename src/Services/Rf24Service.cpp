#include "Rf24Service.h"

bool Rf24Service::configure(
        uint8_t csnPin,
        uint8_t cePin,
        uint8_t sckPin,
        uint8_t misoPin,
        uint8_t mosiPin,
        uint32_t spiSpeed
    ) {
    cePin_ = cePin;
    csnPin_ = csnPin;
    sckPin_ = sckPin;
    misoPin_ = misoPin;
    mosiPin_ = mosiPin;
    spiSpeed_ = spiSpeed;

    if (radio_) delete radio_;
    SPI.end();
    delay(10);
    SPI.begin(sckPin_, misoPin_, mosiPin_, csnPin);
    
    radio_ = new RF24(cePin_, csnPin_);
    if (!radio_ || !radio_->begin(&SPI)) return false;
    isInitialized = true;
    return true;
}

void Rf24Service::initRx() {
    if (!isInitialized) return;
    radio_->setAutoAck(false);
    radio_->setCRCLength(RF24_CRC_DISABLED);
    radio_->setDataRate(RF24_1MBPS);
    radio_->setAddressWidth(2);
}

void Rf24Service::setChannel(uint8_t channel) {
    if (isInitialized) {
        radio_->setChannel(channel);
    }
}

uint8_t Rf24Service::getChannel() {
    if (isInitialized) {
        return radio_->getChannel();
    }
    return 0;
}

void Rf24Service::powerUp() {
    if (isInitialized) {
        radio_->powerUp();
    }
}

void Rf24Service::powerDown(bool hard) {
    if (!isInitialized) {
        return;
    }

    if (hard) {
        radio_->powerDown();
        return;
    }
    
    radio_->stopConstCarrier();
}

void Rf24Service::setPowerLevel(rf24_pa_dbm_e level) {
    if (isInitialized) {
        radio_->setPALevel(level);
    }
}

void Rf24Service::setPowerMax() {
    if (isInitialized) {
        radio_->setPALevel(RF24_PA_MAX);
        radio_->startConstCarrier(RF24_PA_MAX, 45);
    }
}

void Rf24Service::startListening() {
    if (isInitialized) {
        radio_->startListening();
    }
}

void Rf24Service::stopListening() {
    if (isInitialized) {
        radio_->stopListening();
    }
}

void Rf24Service::setDataRate(rf24_datarate_e rate) {
    if (isInitialized) {
        radio_->setDataRate(rate);
    }
}

void Rf24Service::setCrcLength(rf24_crclength_e length) {
    if (isInitialized) {
        radio_->setCRCLength(length);
    }
}

void Rf24Service::openWritingPipe(uint64_t address) {
    if (isInitialized) {
        radio_->openWritingPipe(address);
    }
}

void Rf24Service::openReadingPipe(uint8_t number, uint64_t address) {
    if (isInitialized) {
        radio_->openReadingPipe(number, address);
    }
}

bool Rf24Service::send(const void* buf, uint8_t len) {
    if (!isInitialized) return false;
    bool ok = radio_->write(buf, len);
    return ok;
}

bool Rf24Service::available() {
    if (!isInitialized) return false;
    return radio_->available();
}

bool Rf24Service::receive(void* buf, uint8_t len) {
    if (!isInitialized) return false;
    if (radio_->available()) {
        len = radio_->getDynamicPayloadSize();
        radio_->read(buf, len);
        return true;
    }
    return false;
}

bool Rf24Service::isChipConnected() {
    if (!isInitialized) return false;
    return radio_->isChipConnected();
}

bool Rf24Service::testCarrier() {
    if (!isInitialized) return false;
    return radio_->testCarrier();
}

bool Rf24Service::testRpd() {
    if (!isInitialized) return false;
    return radio_->testRPD();
}

void Rf24Service::flushRx() {
    if (!isInitialized) return;
    radio_->flush_rx();
}

void Rf24Service::flushTx() {
    if (!isInitialized) return;
    radio_->flush_tx();
}