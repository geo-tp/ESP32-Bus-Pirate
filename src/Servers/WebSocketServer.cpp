#include "WebSocketServer.h"


static const char* TAG = "WebSocketServer";

WebSocketServer::WebSocketServer(httpd_handle_t sharedServer)
    : server(sharedServer) {}

void WebSocketServer::setupRoutes() {
    static httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSocketServer::wsHandler,
        .user_ctx = this,
        .is_websocket = true
    };

    httpd_register_uri_handler(server, &ws_uri);
}

esp_err_t WebSocketServer::wsHandler(httpd_req_t *req) {
    WebSocketServer* self = static_cast<WebSocketServer*>(req->user_ctx);
    
    if (req->method == HTTP_GET) {
        clientFd = httpd_req_to_sockfd(req);  // Capture le socket client
        return ESP_OK;
    }
    
    
    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = nullptr;

    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    frame.payload = (uint8_t*)malloc(frame.len + 1);
    if (!frame.payload) return ESP_ERR_NO_MEM;

    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK) {
        free(frame.payload);
        return ret;
    }
    frame.payload[frame.len] = '\0';
    
    // Push chars one by one into buffer
    for (size_t i = 0; i < frame.len; ++i) {
        self->buffer.push_back(((char*)frame.payload)[i]);
    }

    free(frame.payload);
    return ESP_OK;
}

char WebSocketServer::readCharBlocking() {
    while (buffer.empty()) {
        delay(10);
    }
    char c = buffer.front();
    buffer.pop_front();
    return c;
}

char WebSocketServer::readCharNonBlocking() {
    if (buffer.empty()) return KEY_NONE;

    char c = buffer.front();
    buffer.pop_front();

    return c;
}

void WebSocketServer::sendText(const std::string& msg) {
    if (clientFd < 0) return;

    // Sanitize UTF8
    std::string safeMsg = sanitizeUtf8(msg);

    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*) safeMsg.c_str();
    ws_pkt.len = safeMsg.length();

    httpd_ws_send_frame_async(server, clientFd, &ws_pkt);
}

std::string WebSocketServer::sanitizeUtf8(const std::string& input) {
    std::string output;
    size_t i = 0;

    while (i < input.size()) {
        unsigned char c = input[i];

        if (c <= 0x7F) {  // ASCII
            output += c;
            i++;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < input.size() &&
                   (input[i+1] & 0xC0) == 0x80) {
            output += input.substr(i, 2);
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < input.size() &&
                   (input[i+1] & 0xC0) == 0x80 &&
                   (input[i+2] & 0xC0) == 0x80) {
            output += input.substr(i, 3);
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < input.size() &&
                   (input[i+1] & 0xC0) == 0x80 &&
                   (input[i+2] & 0xC0) == 0x80 &&
                   (input[i+3] & 0xC0) == 0x80) {
            output += input.substr(i, 4);
            i += 4;
        } else {
            // Invalid byte or sequence, skip it
            i++;
        }
    }

    return output;
}