#include "JsonTransformer.h"

std::vector<std::string> JsonTransformer::toLines(const std::string& json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        return { "No results found." };
    }

    std::string pretty;
    serializeJsonPretty(doc, pretty);

    std::vector<std::string> out;
    std::string current;
    for (char c : pretty) {
        if (c == '\n') {
            out.push_back(std::move(current));
            current.clear();
        } else if (c != '\r') {
            current.push_back(c);
        }
    }
    if (!current.empty()) out.push_back(std::move(current));
    return out;
}


std::string JsonTransformer::dechunk(const std::string& chunked)
{
    std::string out;
    size_t pos = 0;
    while (pos < chunked.size()) {
        // find end of line
        size_t lineEnd = chunked.find("\r\n", pos);
        if (lineEnd == std::string::npos) break;

        // extract hex size
        std::string lenStr = chunked.substr(pos, lineEnd - pos);
        int chunkSize = strtol(lenStr.c_str(), nullptr, 16);
        if (chunkSize <= 0) break; // 0 => fin

        pos = lineEnd + 2;
        if (pos + chunkSize > chunked.size()) break; // invalide

        // copy chunk
        out.append(chunked, pos, (size_t)chunkSize);

        pos += (size_t)chunkSize;
        // skip \r\n
        if (pos + 2 <= chunked.size() && chunked[pos] == '\r' && chunked[pos+1] == '\n')
            pos += 2;
    }
    return out;
}

std::string JsonTransformer::escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04X", (unsigned)c);
                    out += buf;
                } else {
                    out.push_back((char)c);
                }
        }
    }
    return out;
}

std::string JsonTransformer::makeEntryJson(const std::string& name, size_t size, bool isDir) {
    std::string out;
    out.reserve(name.size() + 48);
    out += "{\"name\":\"";
    out += escape(name);
    out += "\",\"size\":";
    out += std::to_string(size);
    out += ",\"isDir\":";
    out += (isDir ? "true" : "false");
    out += "}";
    return out;
}

std::string JsonTransformer::makeLsJson(const std::string& dir,
                                        size_t total,
                                        size_t used,
                                        const std::vector<std::string>& names,
                                        const std::vector<size_t>& sizes,
                                        const std::vector<uint8_t>& isDirs)
{
    const size_t n = names.size();
    if (sizes.size() != n || isDirs.size() != n) {
        std::string err = "{\"error\":\"mismatched vector sizes\"}";
        return err;
    }

    size_t approx = 64 + dir.size() + n * 48;
    for (auto &nm : names) approx += nm.size();
    std::string out;
    out.reserve(approx);

    out += "{\"dir\":\"";
    out += escape(dir);
    out += "\",\"total\":";
    out += std::to_string(total);
    out += ",\"used\":";
    out += std::to_string(used);
    out += ",\"entries\":[";

    for (size_t i = 0; i < n; ++i) {
        if (i) out += ",";
        out += "{\"name\":\"";
        out += escape(names[i]);
        out += "\",\"size\":";
        out += std::to_string(sizes[i]);
        out += ",\"isDir\":";
        out += (isDirs[i] ? "true" : "false");
        out += "}";
    }

    out += "]}";
    return out;
}