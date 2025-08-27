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
