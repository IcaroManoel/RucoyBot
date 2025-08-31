#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <map>

class ConfigParser {
public:
    ConfigParser(const std::string& filename);
    bool parse();
    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "");
    bool getBoolValue(const std::string& section, const std::string& key, bool defaultValue = false);

private:
    std::string filename_;
    std::map<std::string, std::map<std::string, std::string>> config_;
};

#endif // CONFIG_PARSER_H


