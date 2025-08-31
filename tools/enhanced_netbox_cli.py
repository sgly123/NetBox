#!/usr/bin/env python3
"""
增强版NetBox CLI工具
支持不同模板的init命令
"""

import json
import sys
import argparse
from pathlib import Path
from typing import Dict, Optional

class EnhancedNetBoxCLI:
    """增强版NetBox CLI工具"""
    
    def __init__(self):
        self.config_file = Path(__file__).parent / "template_config.json"
        self.config = self.load_config()
    
    def load_config(self) -> Dict:
        """加载配置文件"""
        if self.config_file.exists():
            with open(self.config_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        else:
            return {"templates": {}}
    
    def list_templates(self):
        """列出可用模板"""
        print("📋 可用模板:")
        print("-" * 40)
        
        for template_id, template in self.config["templates"].items():
            name = template.get("name", template_id)
            description = template.get("description", "无描述")
            features = ", ".join(template.get("features", []))
            
            print(f"🔹 {template_id}: {name}")
            print(f"   描述: {description}")
            print(f"   特性: {features}")
            print()
    
    def cmd_init(self, args):
        """初始化新项目"""
        project_name = args.name or "MyProject"
        template_id = args.template or "default"
        
        if template_id not in self.config["templates"]:
            print(f"❌ 模板 '{template_id}' 不存在")
            print("使用 'netbox list-templates' 查看可用模板")
            return False
        
        template = self.config["templates"][template_id]
        project_dir = Path(project_name)
        
        if project_dir.exists():
            print(f"❌ 目录 {project_name} 已存在")
            return False
        
        print(f"🚀 使用模板 '{template_id}' 创建项目: {project_name}")
        print("=" * 50)
        
        # 创建目录结构
        print("📁 创建目录结构...")
        if not self.create_directory_structure(project_dir):
            return False
        
        # 创建基础文件
        print("📝 创建项目文件...")
        if not self.create_basic_files(project_dir, project_name, template):
            return False
        
        print("=" * 50)
        print(f"✅ 项目 {project_name} 创建成功!")
        print(f"📁 项目目录: {project_dir.absolute()}")
        print(f"🎯 使用模板: {template.get('name', template_id)}")
        
        # 显示模板特性
        features = template.get("features", [])
        if features:
            print(f"🔧 包含特性: {', '.join(features)}")
        
        print()
        print("🔧 下一步:")
        print(f"   cd {project_name}")
        print("   mkdir build && cd build")
        print("   cmake ..")
        print("   make")
        print(f"   ./bin/{project_name}")
        
        return True
    
    def create_directory_structure(self, project_dir: Path) -> bool:
        """创建目录结构"""
        try:
            directories = [
                "src",
                "include/netbox",
                "examples/basic",
                "tests",
                "docs",
                "config",
                "cmake"
            ]
            
            for dir_path in directories:
                (project_dir / dir_path).mkdir(parents=True, exist_ok=True)
            
            return True
        except Exception as e:
            print(f"❌ 创建目录结构失败: {e}")
            return False
    
    def create_basic_files(self, project_dir: Path, project_name: str, template: Dict) -> bool:
        """创建基础文件"""
        try:
            # 创建主程序文件
            self._create_main_source(project_dir, project_name, template)
            
            # 创建头文件
            self._create_main_header(project_dir, project_name, template)
            
            # 创建CMake配置
            self._create_cmake_config(project_dir, project_name, template)
            
            # 创建README
            self._create_readme(project_dir, project_name, template)
            
            # 创建示例文件
            self._create_example_files(project_dir, project_name, template)
            
            # 创建测试文件
            self._create_test_files(project_dir, project_name, template)
            
            return True
        except Exception as e:
            print(f"❌ 创建基础文件失败: {e}")
            return False
    
    def _create_main_source(self, project_dir: Path, project_name: str, template: Dict):
        """创建主程序文件"""
        main_file = project_dir / "src" / "main.cpp"
        
        template_name = template.get("name", "默认模板")
        features = template.get("features", [])
        
        content = f'''/**
 * @file main.cpp
 * @brief {project_name} - 主程序文件
 * @note 使用模板: {template_name}
 */

#include "netbox/NetBox.h"

int main() {{
    std::cout << "🌟 欢迎使用 {project_name}!" << std::endl;
    std::cout << "基于NetBox框架 v" << NETBOX_VERSION << std::endl;
    std::cout << "使用模板: {template_name}" << std::endl;
    
    // 显示模板特性
    std::cout << "🔧 包含特性: ";
    {self._generate_features_code(features)}
    
    // 初始化框架
    if (!NETBOX_INIT()) {{
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }}
    
    // 创建应用
    NetBox::Application app("{project_name}");
    
    if (app.initialize() && app.start()) {{
        std::cout << "✅ 应用启动成功!" << std::endl;
        std::cout << "按Enter键退出..." << std::endl;
        std::cin.get();
        app.stop();
    }}
    
    // 清理框架
    NETBOX_CLEANUP();
    
    return 0;
}}
'''
        
        with open(main_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def _generate_features_code(self, features: list) -> str:
        """生成特性显示代码"""
        if not features:
            return 'std::cout << "无特殊特性" << std::endl;'
        
        feature_list = ', '.join([f'"{feature}"' for feature in features])
        return f'''std::cout << "{', '.join(features)}" << std::endl;'''
    
    def _create_main_header(self, project_dir: Path, project_name: str, template: Dict):
        """创建主头文件"""
        header_file = project_dir / "include" / "netbox" / "NetBox.h"
        
        content = f'''/**
 * @file NetBox.h
 * @brief {project_name} - NetBox框架头文件
 */

#pragma once

#include <iostream>
#include <string>

// 版本信息
#define NETBOX_VERSION "1.0.0"

// 框架初始化宏
#define NETBOX_INIT() NetBox::initialize()
#define NETBOX_CLEANUP() NetBox::cleanup()

namespace NetBox {{
    bool initialize();
    void cleanup();
    
    class Application {{
    public:
        Application(const std::string& name);
        virtual ~Application();
        
        virtual bool initialize();
        virtual bool start();
        virtual void stop();
        
    private:
        std::string m_name;
    }};
}}

// 框架实现
inline bool NetBox::initialize() {{
    std::cout << "NetBox框架初始化..." << std::endl;
    return true;
}}

inline void NetBox::cleanup() {{
    std::cout << "NetBox框架清理..." << std::endl;
}}

inline NetBox::Application::Application(const std::string& name) : m_name(name) {{
    std::cout << "创建应用: " << m_name << std::endl;
}}

inline NetBox::Application::~Application() {{
    std::cout << "销毁应用: " << m_name << std::endl;
}}

inline bool NetBox::Application::initialize() {{
    std::cout << "初始化应用: " << m_name << std::endl;
    return true;
}}

inline bool NetBox::Application::start() {{
    std::cout << "启动应用: " << m_name << std::endl;
    return true;
}}

inline void NetBox::Application::stop() {{
    std::cout << "停止应用: " << m_name << std::endl;
}}
'''
        
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def _create_cmake_config(self, project_dir: Path, project_name: str, template: Dict):
        """创建CMake配置"""
        cmake_file = project_dir / "CMakeLists.txt"
        
        dependencies = template.get("dependencies", ["cmake"])
        deps_list = "\n".join([f'find_package({dep} REQUIRED)' for dep in dependencies if dep != "cmake"])
        
        content = f'''cmake_minimum_required(VERSION 3.10)
project({project_name} VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
{deps_list}

# 添加可执行文件
add_executable({project_name} src/main.cpp)

# 设置包含目录
target_include_directories({project_name} PRIVATE include)

# 链接库
target_link_libraries({project_name} PRIVATE netbox)

# 添加测试
enable_testing()
add_executable({project_name}_test tests/simple_test.cpp)
target_link_libraries({project_name}_test PRIVATE netbox)

# 添加示例
add_executable({project_name}_example examples/basic/simple_example.cpp)
target_link_libraries({project_name}_example PRIVATE netbox)

# 设置输出目录
set_target_properties({project_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/bin
)

set_target_properties({project_name}_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/bin
)

set_target_properties({project_name}_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/bin
)
'''
        
        with open(cmake_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def _create_readme(self, project_dir: Path, project_name: str, template: Dict):
        """创建README文件"""
        readme_file = project_dir / "README.md"
        
        template_name = template.get("name", "默认模板")
        description = template.get("description", "基于NetBox框架的项目")
        features = template.get("features", [])
        features_list = "\n".join([f"- {feature}" for feature in features])
        
        content = f'''# {project_name}

> {description}

## 快速开始

### 构建项目

```bash
mkdir build && cd build
cmake ..
make
```

### 运行程序

```bash
# 运行主程序
./bin/{project_name}

# 运行示例程序
./bin/{project_name}_example

# 运行测试
./bin/{project_name}_test
# 或使用ctest
ctest
```

## 项目结构

```
{project_name}/
├── src/                    # 源代码
│   └── main.cpp           # 主程序
├── include/netbox/        # 头文件
│   └── NetBox.h          # 框架头文件
├── examples/basic/        # 示例代码
│   └── simple_example.cpp # 简单示例
├── tests/                 # 测试代码
│   └── simple_test.cpp   # 基础测试
├── docs/                  # 文档
├── config/               # 配置文件
└── CMakeLists.txt        # 构建配置
```

## 模板信息

- **模板名称**: {template_name}
- **模板描述**: {description}

### 包含特性

{features_list}

## 开发指南

### 添加新功能

1. 在 `src/` 目录下添加源文件
2. 在 `include/netbox/` 目录下添加头文件
3. 更新 `CMakeLists.txt`
4. 编写测试用例

### 扩展应用

继承 `NetBox::Application` 类来创建自定义应用：

```cpp
class MyApp : public NetBox::Application {{
public:
    MyApp() : NetBox::Application("MyApp") {{}}

    bool initialize() override {{
        // 初始化逻辑
        return true;
    }}

    bool start() override {{
        // 启动逻辑
        return true;
    }}
}};
```

## 构建选项

```bash
# Debug构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 并行构建
make -j$(nproc)
```

## 版本信息

- 项目版本: 1.0.0
- NetBox版本: 1.0.0
- C++标准: C++17

---

*由增强版NetBox CLI生成*
'''
        
        with open(readme_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def _create_example_files(self, project_dir: Path, project_name: str, template: Dict):
        """创建示例文件"""
        example_file = project_dir / "examples" / "basic" / "simple_example.cpp"
        
        content = f'''/**
 * @file simple_example.cpp
 * @brief {project_name} - 简单示例程序
 */

#include "netbox/NetBox.h"

int main() {{
    std::cout << "🎯 {project_name} 示例程序" << std::endl;
    
    // 初始化框架
    if (!NETBOX_INIT()) {{
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }}
    
    // 创建示例应用
    NetBox::Application app("{project_name}_Example");
    
    if (app.initialize() && app.start()) {{
        std::cout << "✅ 示例程序运行成功!" << std::endl;
        app.stop();
    }}
    
    // 清理框架
    NETBOX_CLEANUP();
    
    return 0;
}}
'''
        
        with open(example_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def _create_test_files(self, project_dir: Path, project_name: str, template: Dict):
        """创建测试文件"""
        test_file = project_dir / "tests" / "simple_test.cpp"
        
        content = f'''/**
 * @file simple_test.cpp
 * @brief {project_name} - 基础测试程序
 */

#include "netbox/NetBox.h"

int main() {{
    std::cout << "🧪 {project_name} 测试程序" << std::endl;
    
    // 初始化框架
    if (!NETBOX_INIT()) {{
        std::cerr << "❌ 框架初始化失败" << std::endl;
        return 1;
    }}
    
    // 创建测试应用
    NetBox::Application app("{project_name}_Test");
    
    if (app.initialize() && app.start()) {{
        std::cout << "✅ 测试程序运行成功!" << std::endl;
        app.stop();
    }}
    
    // 清理框架
    NETBOX_CLEANUP();
    
    std::cout << "🎉 所有测试通过!" << std::endl;
    return 0;
}}
'''
        
        with open(test_file, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def main(self):
        """主函数"""
        parser = argparse.ArgumentParser(
            description="增强版NetBox CLI - 支持多模板的项目创建工具",
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog="""
示例:
  netbox init MyProject                    # 使用默认模板创建项目
  netbox init MyWebServer --template web_server    # 使用Web服务器模板
  netbox init MyGameServer --template game_server  # 使用游戏服务器模板
  netbox list-templates                    # 列出所有可用模板
            """
        )
        
        subparsers = parser.add_subparsers(dest='command', help='可用命令')
        
        # init命令
        init_parser = subparsers.add_parser('init', help='创建新项目')
        init_parser.add_argument('name', nargs='?', help='项目名称')
        init_parser.add_argument('--template', '-t', 
                               choices=list(self.config["templates"].keys()),
                               default='default',
                               help='选择项目模板')
        
        # list-templates命令
        subparsers.add_parser('list-templates', help='列出所有可用模板')
        
        args = parser.parse_args()
        
        if not args.command:
            parser.print_help()
            return 1
        
        if args.command == 'init':
            return 0 if self.cmd_init(args) else 1
        elif args.command == 'list-templates':
            self.list_templates()
            return 0
        else:
            print(f"❌ 未知命令: {args.command}")
            return 1

if __name__ == "__main__":
    cli = EnhancedNetBoxCLI()
    sys.exit(cli.main()) 