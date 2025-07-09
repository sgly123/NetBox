/*
配置文件读取类 ReadConfig
功能: 读取文件中的配置参数
by sgly 2025.7.1
*/
#ifndef BASETOOL_READ_CONFIG_H_
#define BASETOOL_READ_CONFIG_H_

#include <unordered_map>
#include <string>

class ReadConfig
{
private:
    
    void loadFile(const std::string &filename);
    // 读取并解析文件内容
    
    int writeFile(const std::string &filename);
    // 配置信息写入文件

    static std::string deleteSpace(const std::string &S);
    // 删除字符串前后空格

    bool isLoad;
    std::unordered_map<std::string,std::string> config_map;
    std::string m_fileName; 
public:

    ReadConfig(const std::string &filename);
    //构造函数初始化读取配置文件

    ~ReadConfig() = default;

    std::string getConfigName(const std::string &name) const;
    // 获取参数配置的参数
    
    int setConfigValue(const std::string &name, const std::string &value);
    // 设置文件中参数的值
};



#endif // BASETOOL_READ_CONFIG_H_