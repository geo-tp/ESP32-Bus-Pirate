#pragma once
#include <deque>
#include <esp_http_server.h>
#include <vector>
#include <string>
#include <Inputs/InputKeys.h>
#include <Arduino.h>
#include <esp_log.h>
#include <cstring>

class WebSocketServer {
public:
    WebSocketServer(httpd_handle_t sharedServer);
    void setupRoutes();

    char readCharBlocking();
    char readCharNonBlocking();
    void sendText(const std::string& msg);
    std::string sanitizeUtf8(const std::string& input);

private:
    static esp_err_t wsHandler(httpd_req_t *req);
    httpd_handle_t server;
    static inline std::deque<char> buffer;
    static inline int clientFd = -1; 
};
