#pragma once

#include <string>
#include <vector>
#include <ArduinoJson.h>

class JsonTransformer {
public:
    // Transform JSON to a vector of formatted lines
    std::vector<std::string> toLines(const std::string& json);

    // Remove chunked transfer encoding
    std::string dechunk(const std::string& chunked);

private:
};
