/**
 * @file simple_test.cpp
 * @brief AnimatedProject - 基础测试程序
 */

#include "netbox/NetBox.h"

int main() {
    std::cout << "🧪 AnimatedProject 测试程序" << std::endl;
    
    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }
    
    // 创建测试应用
    NetBox::Application app("AnimatedProject_Test");
    
    if (app.initialize() && app.start()) {
        std::cout << "✅ 测试程序运行成功!" << std::endl;
        app.stop();
    }
    
    // 清理框架
    NETBOX_CLEANUP();
    
    std::cout << "🎉 所有测试通过!" << std::endl;
    return 0;
}
