#include "HttpServer.h"


void HttpServer::setupRoutes() {
    // Page HTML
    static httpd_uri_t root_uri;
    root_uri.uri = "/";
    root_uri.method = HTTP_GET;
    root_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleRootRequest(req);
    };
    root_uri.user_ctx = this;
    httpd_register_uri_handler(server, &root_uri);

    // CSS
    static httpd_uri_t css_uri;
    css_uri.uri = "/style.css";
    css_uri.method = HTTP_GET;
    css_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleCssRequest(req);
    };
    css_uri.user_ctx = this;
    httpd_register_uri_handler(server, &css_uri);

    // JavaScript
    static httpd_uri_t js_uri;
    js_uri.uri = "/scripts.js";
    js_uri.method = HTTP_GET;
    js_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleJsRequest(req);
    };
    js_uri.user_ctx = this;
    httpd_register_uri_handler(server, &js_uri);

    // Mount for routes LittleFS
    littleFsService.begin(/*formatIfFail=*/true, /*readOnly=*/false);
    
    // GET /littlefs/list?dir=/chemin
    static httpd_uri_t lfs_ls_uri;
    lfs_ls_uri.uri = "/littlefs/list";
    lfs_ls_uri.method = HTTP_GET;
    lfs_ls_uri.user_ctx = this;
    lfs_ls_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleLittlefsList(req);
    };
    httpd_register_uri_handler(server, &lfs_ls_uri);

    // POST /littlefs/upload?file=<filename>
    static httpd_uri_t lfs_up_uri;
    lfs_up_uri.uri = "/littlefs/upload";
    lfs_up_uri.method = HTTP_POST;
    lfs_up_uri.user_ctx = this;
    lfs_up_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleLittlefsUpload(req);
    };
    httpd_register_uri_handler(server, &lfs_up_uri);

    // DELETE /littlefs/delete?file=<filename>
    static httpd_uri_t lfs_del_uri;
    lfs_del_uri.uri = "/littlefs/delete";
    lfs_del_uri.method = HTTP_DELETE;
    lfs_del_uri.user_ctx = this;
    lfs_del_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleLittlefsDelete(req);
    };
    httpd_register_uri_handler(server, &lfs_del_uri);

    // GET /littlefs/download?file=<filename>
    static httpd_uri_t lfs_dl_uri;
    lfs_dl_uri.uri = "/littlefs/download";
    lfs_dl_uri.method = HTTP_GET;
    lfs_dl_uri.user_ctx = this;
    lfs_dl_uri.handler = [](httpd_req_t *req) -> esp_err_t {
        HttpServer* self = static_cast<HttpServer*>(req->user_ctx);
        return self->handleLittlefsDownload(req);
    };
    httpd_register_uri_handler(server, &lfs_dl_uri);

    // Note: We can't have more than 8 handlers (ws + 7 above)
}

esp_err_t HttpServer::handleRootRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)index_html, strlen((const char*)index_html));
}

esp_err_t HttpServer::handleCssRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    return httpd_resp_send(req, (const char*)style_css, strlen((const char*)style_css));
}

esp_err_t HttpServer::handleJsRequest(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/javascript");
    return httpd_resp_send(req, (const char*)scripts_js, strlen((const char*)scripts_js));
}

esp_err_t HttpServer::handleLittlefsList(httpd_req_t *req) {
    // Parse ?dir=/...
    std::string dir = "/";
    int qlen = httpd_req_get_url_query_len(req);
    if (qlen > 0) {
        std::vector<char> query(qlen + 1, '\0');
        if (httpd_req_get_url_query_str(req, query.data(), query.size()) == ESP_OK) {
            char val[256];
            if (httpd_query_key_value(query.data(), "dir", val, sizeof(val)) == ESP_OK) {
                dir = val;
            }
        }
    }

    // Not found
    if (!littleFsService.isDir(dir)) {
        httpd_resp_set_type(req, "application/json; charset=utf-8");
        httpd_resp_set_hdr(req, "Cache-Control", "no-store");
        const std::string payload =
            std::string("{\"error\":\"dir not found\",\"dir\":\"") +
            JsonTransformer::escape(dir) + "\"}";
        httpd_resp_set_status(req, "404 Not Found");
        return httpd_resp_send(req, payload.c_str(), HTTPD_RESP_USE_STRLEN);
    }

    // Espace total/used
    size_t total = 0, used = 0;
    littleFsService.getSpace(total, used);

    // Entries
    auto lfsEntries = littleFsService.list(dir);
    std::vector<std::string> names; names.reserve(lfsEntries.size());
    std::vector<size_t>      sizes; sizes.reserve(lfsEntries.size());
    std::vector<uint8_t>     isDirs; isDirs.reserve(lfsEntries.size());

    for (const auto& e : lfsEntries) {
        names.push_back(e.name);
        sizes.push_back(e.size);
        isDirs.push_back(e.isDir ? 1u : 0u);
    }

    // JSON
    std::string payload = JsonTransformer::makeLsJson(dir, total, used, names, sizes, isDirs);
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    return httpd_resp_send(req, payload.c_str(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t HttpServer::handleLittlefsDelete(httpd_req_t *req) {
    // Parse ?name=<file>
    std::string name;
    int qlen = httpd_req_get_url_query_len(req);
    if (qlen > 0) {
        std::vector<char> query(qlen + 1, '\0');
        if (httpd_req_get_url_query_str(req, query.data(), query.size()) == ESP_OK) {
            char val[256];
            if (httpd_query_key_value(query.data(), "file", val, sizeof(val)) == ESP_OK) {
                name = val;
            }
        }
    }

    // Json header
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    // Validate name
    if (!littleFsService.isSafeRootFileName(name)) {
        httpd_resp_set_status(req, "400 Bad Request");
        std::string payload = std::string("{\"error\":\"invalid name\",\"name\":\"") +
                              JsonTransformer::escape(name) + "\"}";
        return httpd_resp_send(req, payload.c_str(), HTTPD_RESP_USE_STRLEN);
    }

    // Construct path
    std::string path = "/" + name;

    // Attempt delete
    bool rc = littleFsService.removeFile(path);
    if (!rc) {
        httpd_resp_set_status(req, "404 Not Found");
        std::string payload = std::string("{\"error\":\"file not found or cannot delete\",\"name\":\"") +
                              JsonTransformer::escape(name) + "\"}";
        return httpd_resp_send(req, payload.c_str(), HTTPD_RESP_USE_STRLEN);
    }

    // OK
    const char* ok = "{\"ok\":true}";

    return httpd_resp_send(req, ok, HTTPD_RESP_USE_STRLEN);
}

esp_err_t HttpServer::handleLittlefsDownload(httpd_req_t* req) {
    std::string name;
    bool forceDownload = false;

    // Get ?file=<name>
    int qlen = httpd_req_get_url_query_len(req);
    if (qlen > 0) {
        std::vector<char> query(qlen + 1, '\0');
        if (httpd_req_get_url_query_str(req, query.data(), query.size()) == ESP_OK) {
            char val[256];
            if (httpd_query_key_value(query.data(), "file", val, sizeof(val)) == ESP_OK) {
                name = val;
            }
            if (httpd_query_key_value(query.data(), "dl", val, sizeof(val)) == ESP_OK) {
                forceDownload = (strcmp(val, "1") == 0 || strcasecmp(val, "true") == 0);
            }
        }
    }

    if (name.empty()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'file' parameter");
        return ESP_FAIL;
    }
    if (!littleFsService.isSafeRootFileName(name)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad filename");
        return ESP_FAIL;
    }

    // Remove /
    std::string baseName = name;
    auto pos = baseName.find_last_of("/\\");
    if (pos != std::string::npos) {
        baseName = baseName.substr(pos + 1);
    }

    // Force download headers
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    std::string cd = std::string("attachment; filename=\"") + baseName + "\"";
    httpd_resp_set_hdr(req, "Content-Disposition", cd.c_str());
    httpd_resp_set_hdr(req, "Content-Transfer-Encoding", "binary");
    httpd_resp_set_type(req, "application/octet-stream");
    
    // Stream the file
    const std::string userPath = "/" + name;
    fs::File f = LittleFS.open(userPath.c_str(), "r");
    const size_t CHUNK = 1024;
    std::unique_ptr<uint8_t[]> buf(new (std::nothrow) uint8_t[CHUNK]);
    while (true) {
        int n = f.read(buf.get(), CHUNK);
        if (n < 0) { f.close(); return -1; }
        if (n == 0) break;
        if (httpd_resp_send_chunk(req, reinterpret_cast<const char*>(buf.get()), n) != ESP_OK) {
            f.close();
            return -1;
        }
    }
    f.close();
    httpd_resp_send_chunk(req, nullptr, 0);

    return 0; // ESP_OK
}

esp_err_t HttpServer::handleLittlefsUpload(httpd_req_t* req) {
    // Get ?file=<name>
    std::string name;
    int qlen = httpd_req_get_url_query_len(req);
    if (qlen > 0) {
        std::vector<char> query(qlen + 1, '\0');
        if (httpd_req_get_url_query_str(req, query.data(), query.size()) == ESP_OK) {
            char val[256];
            if (httpd_query_key_value(query.data(), "file", val, sizeof(val)) == ESP_OK) {
                name = sanitizeUploadFilename(val);
            }
        }
    }

    // Validate name
    if (name.empty() || !littleFsService.isSafeRootFileName(name)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or bad ?file");
        return ESP_FAIL;
    }

    // Open file
    const std::string path = "/" + name;
    fs::File out = LittleFS.open(path.c_str(), "w");
    if (!out) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot open file");
        return ESP_FAIL;
    }

    // Init buffer
    const size_t CHUNK = 1024;
    std::unique_ptr<uint8_t[]> buf(new (std::nothrow) uint8_t[CHUNK]);
    if (!buf) {
        out.close();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OOM");
        return ESP_FAIL;
    }

    // Read all data in chunks
    int remaining = req->content_len;
    while (remaining > 0) {
        int to_read = remaining > (int)CHUNK ? (int)CHUNK : remaining;
        int n = httpd_req_recv(req, (char*)buf.get(), to_read);
        if (n <= 0) { out.close(); httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Recv error"); return ESP_FAIL; }
        if (out.write(buf.get(), n) != (size_t)n) { out.close(); httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Write error"); return ESP_FAIL; }
        remaining -= n;
    }
    out.close();

    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");

    return httpd_resp_send(req, "{\"ok\":true}", HTTPD_RESP_USE_STRLEN);
}

std::string HttpServer::urlDecode(const char* s) {
    // Decode %XX and + to space
    std::string out;
    for (const char* p = s; *p; ++p) {
        if (*p == '+') out.push_back(' ');
        else if (*p == '%' && std::isxdigit((unsigned char)p[1]) && std::isxdigit((unsigned char)p[2])) {
            auto hex = [](char c){ c=std::toupper((unsigned char)c); return std::isdigit((unsigned char)c)? c-'0' : c-'A'+10; };
            out.push_back(char((hex(p[1])<<4) | hex(p[2])));
            p += 2;
        } else out.push_back(*p);
    }

    return out;
}

std::string HttpServer::sanitizeUploadFilename(const char* raw) {
    std::string name = urlDecode(raw);

    // Only keep the base name (remove path)
    if (auto pos = name.find_last_of("/\\"); pos != std::string::npos)
        name = name.substr(pos + 1);

    // Replace all whitespace (spaces, etc) with _
    std::string out; out.reserve(name.size());
    bool prevUnderscore = false;
    for (unsigned char c : name) {
        if (std::isspace(c) || c == 0xA0 /*NBSP*/ ) {
            if (!prevUnderscore) { out.push_back('_'); prevUnderscore = true; }
        } else {
            out.push_back((char)c);
            prevUnderscore = false;
        }
    }
    // trim _ from start and end
    while (!out.empty() && out.front() == '_') out.erase(out.begin());
    while (!out.empty() && out.back()  == '_') out.pop_back();

    return out;
}
