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


    // Escape a string for JSON
    static std::string escape(const std::string& s);

    // Create a JSON entry for a file or directory
    static std::string makeEntryJson(const std::string& name, size_t size, bool isDir);

    // Create a JSON listing for a directory
    static std::string makeLsJson(const std::string& dir,
                                  size_t total,
                                  size_t used,
                                  const std::vector<std::string>& names,
                                  const std::vector<size_t>& sizes,
                                  const std::vector<uint8_t>& isDirs);
                                  
private:
};
