#include "util/ConfigReader.h"
#include <fstream>
#include <sstream>

bool ConfigReader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // 去除行尾注释
        auto commentPos = value.find('#');
        if (commentPos != std::string::npos) {
            value = value.substr(0, commentPos);
        }
        
        // 去除首尾空白字符
        while (!value.empty() && std::isspace(value.front())) {
            value.erase(0, 1);
        }
        while (!value.empty() && std::isspace(value.back())) {
            value.pop_back();
        }
        
        config_[key] = value;
    }
    return true;
}

std::string ConfigReader::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = config_.find(key);
    return it != config_.end() ? it->second : defaultValue;
}

int ConfigReader::getInt(const std::string& key, int defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {}
    }
    return defaultValue;
} 