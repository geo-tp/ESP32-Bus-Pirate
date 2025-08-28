#include "TelnetService.h"

bool TelnetService::connectTo(const std::string& host, uint16_t port, uint32_t recvTimeoutMs) {
  close(); // just in case

  // DNS resolution
  struct addrinfo hints {};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res = nullptr;
  const std::string portStr = std::to_string(port);
  int gai = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
  if (gai != 0 || !res) {
    _lastError = "DNS failed";
    return false;
  }

  // Socket creation
  _sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (_sock < 0) {
    _lastError = "socket failed";
    freeaddrinfo(res);
    return false;
  }

  // Timeout for recv
  struct timeval tv;
  tv.tv_sec  = static_cast<time_t>(recvTimeoutMs / 1000);
  tv.tv_usec = static_cast<suseconds_t>((recvTimeoutMs % 1000) * 1000);
  setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  // Connect
  if (::connect(_sock, res->ai_addr, res->ai_addrlen) != 0) {
    _lastError = "connect failed";
    ::close(_sock);
    _sock = -1;
    freeaddrinfo(res);
    return false;
  }

  freeaddrinfo(res);
  _rx.reserve(512);
  return true;
}

void TelnetService::close() {
  if (_sock >= 0) {
    ::shutdown(_sock, SHUT_RDWR);
    ::close(_sock);
    _sock = -1;
  }
  _rx.clear();
}

int TelnetService::sendAll(int s, const void* data, size_t len) {
  const uint8_t* p = static_cast<const uint8_t*>(data);
  size_t sentTotal = 0;
  while (sentTotal < len) {
    size_t n = ::send(s, p + sentTotal, len - sentTotal, 0);
    if (n <= 0) return -1;
    sentTotal += n;
  }
  return static_cast<int>(sentTotal);
}

bool TelnetService::writeChar(char c) {
  if (_sock < 0) return false;
  if (c == '\n' || c == '\r') {
    static const char crlf[2] = {'\r','\n'};
    return sendAll(_sock, crlf, 2) == 2;
  }
  return sendAll(_sock, &c, 1) == 1;
}

int TelnetService::writeRaw(const char* data, size_t len) {
  if (_sock < 0 || !data || !len) return -1;
  return sendAll(_sock, data, len);
}

bool TelnetService::writeLine(const std::string& line) {
  if (_sock < 0) return false;
  static const char crlf[2] = {'\r','\n'};
  return sendAll(_sock, line.c_str(), line.size()) == (int)line.size()
      && sendAll(_sock, crlf, 2) == 2;
}

void TelnetService::sendIAC(uint8_t cmd, uint8_t opt) {
  if (_sock < 0) return;
  const uint8_t pkt[3] = { IAC, cmd, opt };
  sendAll(_sock, pkt, sizeof(pkt));
}

void TelnetService::doTelnetNegotiation(const uint8_t* buf, int len, std::string& out) {
  for (int i = 0; i < len; ++i) {
    uint8_t b = buf[i];
    if (b != IAC) { out.push_back(static_cast<char>(b)); continue; }

    if (i + 1 >= len) break;
    uint8_t cmd = buf[++i];

    if (cmd == IAC) { out.push_back(static_cast<char>(IAC)); continue; } // 0xFF 

    if (cmd == WILL || cmd == WONT || cmd == DO || cmd == DONT) {
      if (i + 1 >= len) break;
      uint8_t opt = buf[++i];

      if (cmd == WILL) {
        // Server requests to enable <opt>
        if (opt == OPT_ECHO || opt == OPT_SGA || opt == OPT_BINARY) sendIAC(DO, opt);
        else                                                        sendIAC(DONT, opt);
      } else if (cmd == DO) {
        // Server asks us to enable <opt> -> refuse (WONT)
        sendIAC(WONT, opt);
      }
      // WONT / DONT
      continue;
    }

    if (cmd == SB) {
      // Subnegotiation until IAC SE
      while (i + 1 < len) {
        uint8_t x = buf[++i];
        if (x == IAC && i + 1 < len && buf[i+1] == SE) { i++; break; }
      }
      continue;
    }

    // Other IAC commands: ignored for now
  }
}

void TelnetService::poll() {
  if (_sock < 0) return;

  uint8_t buf[512];
  int n = ::recv(_sock, reinterpret_cast<char*>(buf), sizeof(buf), MSG_DONTWAIT);
  if (n <= 0) return;

  std::string out;
  out.reserve(static_cast<size_t>(n));
  doTelnetNegotiation(buf, n, out);

  if (!out.empty()) _rx.append(out);
}

std::string TelnetService::readOutputNonBlocking() {
  std::string out;
  out.swap(_rx);
  return out;
}
