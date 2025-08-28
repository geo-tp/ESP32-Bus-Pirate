#pragma once
#include <cstdint>
#include <string>

extern "C" {
  #include <lwip/sockets.h>
  #include <lwip/inet.h>
  #include <lwip/netdb.h>
}

class TelnetService {
public:
  // Connect to a Telnet server (synchronous)
  bool connectTo(const std::string& host, uint16_t port, uint32_t recvTimeoutMs = 3000);

  void close();
  bool isConnected() const { return _sock >= 0; }

  // Send a character
  bool writeChar(char c);

  // Send raw data
  int  writeRaw(const char* data, size_t len);

  // Send a line
  bool writeLine(const std::string& line);

  // Read the socket
  void poll();

  // Retrieve and clear the text buffer accumulated by poll()
  std::string readOutputNonBlocking();

  // Get the last error message if connection failed
  const std::string& lastError() const { return _lastError; }

private:
  // Telnet constants
  static constexpr uint8_t IAC   = 255;
  static constexpr uint8_t DONT  = 254;
  static constexpr uint8_t DO    = 253;
  static constexpr uint8_t WONT  = 252;
  static constexpr uint8_t WILL  = 251;
  static constexpr uint8_t SB    = 250;
  static constexpr uint8_t SE    = 240;

  // Options
  static constexpr uint8_t OPT_BINARY = 0;
  static constexpr uint8_t OPT_ECHO   = 1;
  static constexpr uint8_t OPT_SGA    = 3;

  int sendAll(int s, const void* data, size_t len);

  void doTelnetNegotiation(const uint8_t* buf, int len, std::string& out);

  void sendIAC(uint8_t cmd, uint8_t opt);

private:
  int         _sock      = -1;
  std::string _rx;
  std::string _lastError;
};
