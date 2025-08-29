#include "ModbusService.h"

ModbusService* ModbusService::s_self = nullptr;
ModbusService::ModbusService()  { s_self = this; }
ModbusService::~ModbusService() { if (s_self == this) s_self = nullptr; }

bool ModbusService::setTarget(const std::string& hostOrIp, uint16_t port) {
  IPAddress ip;
  if (!resolveIPv4(hostOrIp, ip)) return false;
  _host = ip;
  _port = port ? port : 502;
  _mb.reset(new ModbusClientTCPasync(_host, _port));

  if (_mb) {
    _mb->onDataHandler(&ModbusService::s_onData);
    _mb->onErrorHandler(&ModbusService::s_onError);
    _mb->setTimeout(_timeoutMs);
    _mb->setIdleTimeout(_idleCloseMs);
    _mb->setMaxInflightRequests(_maxInflight);
  }
  return true;
}

void ModbusService::begin(uint32_t reqTimeoutMs, uint32_t idleCloseMs, uint32_t maxInflight) {
  _timeoutMs   = reqTimeoutMs;
  _idleCloseMs = idleCloseMs;
  _maxInflight = maxInflight;

  if (_mb) {
    _mb->onDataHandler(&ModbusService::s_onData);
    _mb->onErrorHandler(&ModbusService::s_onError);
    _mb->setTimeout(_timeoutMs);
    _mb->setIdleTimeout(_idleCloseMs);
    _mb->setMaxInflightRequests(_maxInflight);
  }
}

// FC01 - Read Coils
Error ModbusService::readCoils(uint8_t unit, uint16_t addr0, uint16_t qty) {
  if (!_mb) return INVALID_SERVER;
  return _mb->addRequest(millis(), unit, READ_COIL, addr0, qty);
}

// FC02 - Read Discrete Inputs
Error ModbusService::readDiscreteInputs(uint8_t unit, uint16_t addr0, uint16_t qty) {
  if (!_mb) return INVALID_SERVER;
  return _mb->addRequest(millis(), unit, READ_DISCR_INPUT, addr0, qty);
}

// FC03 - Read Holding Registers
Error ModbusService::readHolding(uint8_t unit, uint16_t addr0, uint16_t qty) {
  if (!_mb) return INVALID_SERVER;
  return _mb->addRequest(millis(), unit, READ_HOLD_REGISTER, addr0, qty);
}

// FC04 - Read Input Registers
Error ModbusService::readInputRegisters(uint8_t unit, uint16_t addr0, uint16_t qty) {
  if (!_mb) return INVALID_SERVER;
  return _mb->addRequest(millis(), unit, READ_INPUT_REGISTER, addr0, qty);
}

// FC05 - Write Single Coil
Error ModbusService::writeSingleCoil(uint8_t unit, uint16_t addr0, bool on) {
  if (!_mb) return INVALID_SERVER;
  const uint16_t val = on ? 0xFF00 : 0x0000;
  return _mb->addRequest(millis(), unit, WRITE_COIL, addr0, val);
}

// FC06 - Write Single Holding Register
Error ModbusService::writeHoldingSingle(uint8_t unit, uint16_t addr0, uint16_t value) {
  if (!_mb) return INVALID_SERVER;
  return _mb->addRequest(millis(), unit, WRITE_HOLD_REGISTER, addr0, value);
}

// FC10/0x16 - Write Multiple Holding Registers
Error ModbusService::writeHoldingMultiple(uint8_t unit, uint16_t addr0, const std::vector<uint16_t>& values) {
  if (!_mb) return INVALID_SERVER;
  std::vector<uint16_t> tmp(values.begin(), values.end());
  const uint16_t qty     = static_cast<uint16_t>(tmp.size());
  const uint8_t  byteCnt = static_cast<uint8_t>(qty * 2);
  return _mb->addRequest(millis(), unit, WRITE_MULT_REGISTERS, addr0, qty, byteCnt, tmp.data());
}

// FC0F - Write Multiple Coils
Error ModbusService::writeMultipleCoils(uint8_t unit, uint16_t addr0,
                                              const std::vector<uint8_t>& packedBytes,
                                              uint16_t coilQty) {
  if (!_mb) return INVALID_SERVER;
  std::vector<uint8_t> tmp(packedBytes.begin(), packedBytes.end());
  const uint8_t byteCnt = static_cast<uint8_t>(tmp.size());
  return _mb->addRequest(millis(), unit, WRITE_MULT_COILS, addr0, coilQty, byteCnt, tmp.data());
}

void ModbusService::s_onData(ModbusMessage resp, uint32_t token) {
  if (s_self) s_self->onData(resp, token);
}

void ModbusService::s_onError(Error error, uint32_t token) {
  if (s_self) s_self->onError(error, token);
}

void ModbusService::onData(ModbusMessage& resp, uint32_t token) {
  Reply r;
  r.fc = resp.getFunctionCode();
  r.raw.assign(resp.begin(), resp.end()); // debug

  if (r.fc & 0x80) {
    r.ok = false;
    if (resp.size() >= 2) r.exception = resp[1];
  } else {
    r.ok = true;

    // Read Holding/Input Registers
    if (r.fc == READ_HOLD_REGISTER || r.fc == READ_INPUT_REGISTER) {
      if (!parseFC03or04(resp, r.regs)) {
        r.ok = false; r.error = "parse FC03/04 failed";
      }

    // Read Coils / Discrete Inputs
    } else if (r.fc == READ_COIL || r.fc == READ_DISCR_INPUT) {
      if (!parseFC01or02(resp, r.coilBytes, r.byteCount)) {
        r.ok = false; r.error = "parse FC01/02 failed";
      }
    }
    // other FCs...
  }

  if (_onReply) _onReply(r, token);
}

void ModbusService::onError(Error error, uint32_t token) {
  if (_onError) _onError(error, token);

  Reply r;
  r.ok = false;
  ModbusError me(error);
  r.error = (const char*)me;

  if (_onReply) _onReply(r, token);
}

bool ModbusService::resolveIPv4(const std::string& host, IPAddress& outIp) {
  addrinfo hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
  addrinfo* res = nullptr;
  if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || !res) return false;
  auto* sin = reinterpret_cast<sockaddr_in*>(res->ai_addr);
  outIp = IPAddress(sin->sin_addr.s_addr);
  freeaddrinfo(res);
  return true;
}

bool ModbusService::parseFC03or04(ModbusMessage& msg, std::vector<uint16_t>& outRegs) {
  if (msg.size() < 3) return false;
  uint8_t fc = msg.getFunctionCode();
  if (fc != 0x03 && fc != 0x04) return false;
  uint8_t bc = msg[2];
  if (msg.size() < static_cast<size_t>(3 + bc)) return false;
  if (bc % 2) return false;

  outRegs.resize(bc / 2);
  for (size_t i = 0; i < outRegs.size(); ++i) {
    outRegs[i] = static_cast<uint16_t>((msg[3 + 2*i] << 8) | msg[3 + 2*i + 1]);
  }
  return true;
}

bool ModbusService::parseFC01or02(ModbusMessage& msg,
                                  std::vector<uint8_t>& outBytes,
                                  uint8_t& outByteCount) {
  if (msg.size() < 3) return false;
  uint8_t fc = msg.getFunctionCode();
  if (fc != READ_COIL && fc != READ_DISCR_INPUT) return false;

  uint8_t bc = msg[2];
  if (bc == 0) return false;
  if (msg.size() < static_cast<size_t>(3 + bc)) return false;

  outByteCount = bc;
  outBytes.assign(msg.begin() + 3, msg.begin() + 3 + bc);  // LSB first
  return true;
}