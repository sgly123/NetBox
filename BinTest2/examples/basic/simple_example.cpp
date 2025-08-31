/**
 * @file simple_example.cpp
 * @brief BinTest2 - 简单示例程序
 */

#include "netbox/NetBox.h"

int main() {
    std::cout << "🎯 BinTest2 示例程序" << std::endl;
    
    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }
    
    // 创建示例应用
    NetBox::Application app("BinTest2_Example");
    
    if (app.initialize() && app.start()) {
        std::cout << "✅ 示例程序运行成功!" << std::endl;
        app.stop();
    }
    
    // 清理框架
    NETBOX_CLEANUP();
    
    return 0;
}
