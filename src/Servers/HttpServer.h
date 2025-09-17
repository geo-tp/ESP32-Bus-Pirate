#pragma once

#include <Arduino.h>
#include <cstring>
#include <string>
#include <esp_http_server.h>
#include "Services/LittleFsService.h"
#include "Transformers/JsonTransformer.h"
#include "../webui/index.h"
#include "../webui/scripts.h"
#include "../webui/style.h"

class HttpServer {
public:
    HttpServer(httpd_handle_t sharedServer, LittleFsService& fsService, JsonTransformer& jsonTransformer)
        : server(sharedServer), littleFsService(fsService), jsonTransformer(jsonTransformer) {} ;
    void setupRoutes();

private:
    httpd_handle_t server;
    LittleFsService& littleFsService;
    JsonTransformer& jsonTransformer;
    esp_err_t handleRootRequest(httpd_req_t *req);
    esp_err_t handleCssRequest(httpd_req_t *req);
    esp_err_t handleJsRequest(httpd_req_t *req);
    esp_err_t handleLittlefsList(httpd_req_t *req);
    esp_err_t handleLittlefsDelete(httpd_req_t *req);
    esp_err_t handleLittlefsDownload(httpd_req_t* req);
    esp_err_t handleLittlefsUpload(httpd_req_t* req);

    std::string urlDecode(const char* s);
    std::string sanitizeUploadFilename(const char* raw);
};
