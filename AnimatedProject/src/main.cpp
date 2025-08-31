/**
 * @file main.cpp
 * @brief AnimatedProject - 主程序文件
 * @note 使用模板: MyCustomWebServer
 */

#include "netbox/NetBox.h"

int main() {
    std::cout << "🌟 欢迎使用 AnimatedProject!" << std::endl;
    std::cout << "基于NetBox框架 v" << NETBOX_VERSION << std::endl;
    std::cout << "使用模板: MyCustomWebServer" << std::endl;
    
    // 显示模板特性
    std::cout << "🔧 包含特性: ";
    std::cout << "http_server, websocket, ssl_support, static_files" << std::endl;
    
    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }
    
    // 创建应用
    NetBox::Application app("AnimatedProject");
    
    if (app.initialize() && app.start()) {
        std::cout << "✅ 应用启动成功!" << std::endl;
        std::cout << "按Enter键退出..." << std::endl;
        std::cin.get();
        app.stop();
    }
    
    // 清理框架
    NETBOX_CLEANUP();
    
    return 0;
}
