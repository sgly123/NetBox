#include "../include/ReadConfig.h"
#include <iostream>
#include <fstream>
#include <sstream>

ReadConfig::ReadConfig(const std::string &fileName)
:m_fileName(fileName) {
    loadFile(fileName);
}

std::string ReadConfig::getConfigName(const std::string &name)const 
{
if(!isLoad) return "";
auto it = config_map.find(name);
if (it != config_map.end())
{
    return it->second;
}
else {
    return "";
}
/*
return (it != config_map.end())?it->second:"";
*/
}

int ReadConfig::setConfigValue(const std::string &name, const std::string &value) 
{

if (!isLoad) return -1;
config_map[name] = value;
return writeFile(m_fileName);

} 

void ReadConfig::loadFile(const std::string &fileName) {
    std::ifstream file(fileName.empty() ? m_fileName : fileName);
    if (!file.is_open()) {
        isLoad = false;
        return;
    }
    std::string line;
    while (getline(file, line)) {
        size_t POS = line.find('#');
        if (POS != std::string::npos) {
            continue;
        }
        
        line = deleteSpace(line);

        if (line.empty()) continue;
        
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;
        std::string key = deleteSpace(line.substr(0, eq_pos));
        std::string value = deleteSpace(line.substr(eq_pos + 1));
        if (!key.empty() && !value.empty()) {
            config_map[key] = value;
        }
    }
    isLoad = true;
}

int  ReadConfig::writeFile(const std::string &filename) {
    std::ofstream file(filename.empty()? m_fileName:filename);
    if (!file.is_open()) return -1;

    for (auto& [key,value]:config_map)
    {
        file << key << "=" << value <<"\n";
    }
    return 0;
}

std::string ReadConfig::deleteSpace(const std::string &S) {
    auto start = S.begin();
    while (start != S.end() && std::isspace(*start)) {
        start++;
    }
    auto end = S.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}