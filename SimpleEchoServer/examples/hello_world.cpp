/**
 * @file hello_world.cpp
 * @brief SimpleEchoServer - Hello World示例
 * 
 * 这是一个简单的Hello World程序，展示Jinja2模板的基本用法
 * 
 * @author NetBox开发者
 * @version 1.0.0
 * @date 2025-08-03
 */

#include <iostream>
#include <string>

// ANSI颜色代码
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

int main() {
    std::cout << CYAN << "🌟 " << RESET;
    std::cout << "Hello from SimpleEchoServer!" << std::endl;
    
    std::cout << std::endl;
    std::cout << "📋 项目信息:" << std::endl;
    std::cout << "   名称: SimpleEchoServer" << std::endl;
    std::cout << "   版本: 1.0.0" << std::endl;
    std::cout << "   作者: NetBox开发者" << std::endl;
    std::cout << "   日期: 2025-08-03" << std::endl;
    
    std::cout << std::endl;
    std::cout << GREEN << "✅ " << RESET;
    std::cout << "启用的特性:" << std::endl;
    std::cout << "   - 网络编程" << std::endl;
    std::cout << "   - 模板引擎" << std::endl;
    std::cout << "   - 跨平台" << std::endl;
    std::cout << "   - 高性能" << std::endl;
    
    std::cout << std::endl;
    std::cout << "请输入你的名字: ";
    std::string name;
    std::getline(std::cin, name);
    
    if (!name.empty()) {
        std::cout << YELLOW << "👋 " << RESET;
        std::cout << "你好, " << name << "! 欢迎使用 SimpleEchoServer!" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << MAGENTA << "🚀 " << RESET;
    std::cout << "SimpleEchoServer 运行完成！" << std::endl;
    
    return 0;
}