/**
 * @file NetBox.h
 * @brief BinTest - NetBox框架头文件
 */

#pragma once

#include <iostream>
#include <string>

// 版本信息
#define NETBOX_VERSION "1.0.0"

// 框架初始化宏
#define NETBOX_INIT() NetBox::initialize()
#define NETBOX_CLEANUP() NetBox::cleanup()

namespace NetBox {
    bool initialize();
    void cleanup();
    
    class Application {
    public:
        Application(const std::string& name);
        virtual ~Application();
        
        virtual bool initialize();
        virtual bool start();
        virtual void stop();
        
    private:
        std::string m_name;
    };
}

// 框架实现
inline bool NetBox::initialize() {
    std::cout << "NetBox框架初始化..." << std::endl;
    return true;
}

inline void NetBox::cleanup() {
    std::cout << "NetBox框架清理..." << std::endl;
}

inline NetBox::Application::Application(const std::string& name) : m_name(name) {
    std::cout << "创建应用: " << m_name << std::endl;
}

inline NetBox::Application::~Application() {
    std::cout << "销毁应用: " << m_name << std::endl;
}

inline bool NetBox::Application::initialize() {
    std::cout << "初始化应用: " << m_name << std::endl;
    return true;
}

inline bool NetBox::Application::start() {
    std::cout << "启动应用: " << m_name << std::endl;
    return true;
}

inline void NetBox::Application::stop() {
    std::cout << "停止应用: " << m_name << std::endl;
}
