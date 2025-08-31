
#include "config_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// Helper function to trim whitespace from a string
std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return s;
    }
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}

ConfigParser::ConfigParser(const std::string& filename) : filename_(filename) {}

bool ConfigParser::parse() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename_ << std::endl;
        return false;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ";" || line[0] == "#") {
            continue; // Skip empty lines and comments
        }

        if (line[0] == "[" && line.back() == "]") {
            currentSection = line.substr(1, line.length() - 2);
        } else {
            size_t delimiterPos = line.find("=");
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                config_[currentSection][key] = value;
            }
        }
    }
    file.close();
    return true;
}

std::string ConfigParser::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) {
    if (config_.count(section) && config_[section].count(key)) {
        return config_[section][key];
    }
    return defaultValue;
}

bool ConfigParser::getBoolValue(const std::string& section, const std::string& key, bool defaultValue) {
    std::string value = getValue(section, key);
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return defaultValue;
}


