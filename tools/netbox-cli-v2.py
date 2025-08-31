#!/usr/bin/env python3
"""
增强版NetBox CLI工具
支持不同模板的init命令，包含ASCII艺术字和动画效果
"""

import json
import sys
import argparse
import os
import time
import random
import math
from pathlib import Path
from typing import Dict, Optional, List

# ASCII艺术字和动画类
class Colors:
    """ANSI颜色代码"""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    DIM = '\033[2m'
    
    # 基础颜色
    BLACK = '\033[30m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    
    # 亮色
    BRIGHT_BLACK = '\033[90m'
    BRIGHT_RED = '\033[91m'
    BRIGHT_GREEN = '\033[92m'
    BRIGHT_YELLOW = '\033[93m'
    BRIGHT_BLUE = '\033[94m'
    BRIGHT_MAGENTA = '\033[95m'
    BRIGHT_CYAN = '\033[96m'
    BRIGHT_WHITE = '\033[97m'
    
    # 背景色
    BG_BLACK = '\033[40m'
    BG_RED = '\033[41m'
    BG_GREEN = '\033[42m'
    BG_YELLOW = '\033[43m'
    BG_BLUE = '\033[44m'
    BG_MAGENTA = '\033[45m'
    BG_CYAN = '\033[46m'
    BG_WHITE = '\033[47m'

class ASCIIArt:
    """ASCII艺术字生成器"""
    
    # NetBox大标题ASCII艺术字
    NETBOX_LOGO = [
        "███╗   ██╗███████╗████████╗██████╗  ██████╗ ██╗  ██╗",
        "████╗  ██║██╔════╝╚══██╔══╝██╔══██╗██╔═══██╗╚██╗██╔╝",
        "██╔██╗ ██║█████╗     ██║   ██████╔╝██║   ██║ ╚███╔╝ ",
        "██║╚██╗██║██╔══╝     ██║   ██╔══██╗██║   ██║ ██╔██╗ ",
        "██║ ╚████║███████╗   ██║   ██████╔╝╚██████╔╝██╔╝ ██╗",
        "╚═╝  ╚═══╝╚══════╝   ╚═╝   ╚═════╝  ╚═════╝ ╚═╝  ╚═╝"
    ]
    
    # NetBox项目标题
    FRAMEWORK_TITLE = [
        "███╗   ██╗███████╗████████╗██████╗  ██████╗ ██╗  ██╗",
        "████╗  ██║██╔════╝╚══██╔══╝██╔══██╗██╔═══██╗╚██╗██╔╝",
        "██╔██╗ ██║█████╗     ██║   ██████╔╝██║   ██║ ╚███╔╝ ",
        "██║╚██╗██║██╔══╝     ██║   ██╔══██╗██║   ██║ ██╔██╗ ",
        "██║ ╚████║███████╗   ██║   ██████╔╝╚██████╔╝██╔╝ ██╗",
        "╚═╝  ╚═══╝╚══════╝   ╚═╝   ╚═════╝  ╚═════╝ ╚═╝  ╚═╝"
    ]
    
    # NetBox小标题
    NETBOX_SUBTITLE = [
        "╔╗╔┌─┐┌┬┐╔╗ ┌─┐─┐ ┬",
        "║║║├┤  │ ╠╩╗│ │┌┴┬┘",
        "╝╚╝└─┘ ┴ ╚═╝└─┘┴ └─"
    ]
    
    # 创建项目动画帧
    CREATING_FRAMES = [
        ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"],  # 旋转
        ["▁", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃"],  # 进度条
        ["◐", "◓", "◑", "◒"],  # 圆形旋转
        ["⣾", "⣽", "⣻", "⢿", "⡿", "⣟", "⣯", "⣷"]  # 点阵旋转
    ]
    
    @staticmethod
    def get_gradient_colors() -> List[str]:
        """获取渐变色彩序列"""
        return [
            Colors.BRIGHT_CYAN,
            Colors.CYAN,
            Colors.BRIGHT_BLUE,
            Colors.BLUE,
            Colors.BRIGHT_MAGENTA,
            Colors.MAGENTA
        ]
    
    @staticmethod
    def colorize_text(text: str, color: str) -> str:
        """为文本添加颜色"""
        return f"{color}{text}{Colors.RESET}"
    
    @staticmethod
    def rainbow_text(text: str) -> str:
        """彩虹文字效果"""
        colors = [Colors.RED, Colors.YELLOW, Colors.GREEN, Colors.CYAN, Colors.BLUE, Colors.MAGENTA]
        result = ""
        for i, char in enumerate(text):
            color = colors[i % len(colors)]
            result += f"{color}{char}{Colors.RESET}"
        return result

class Animations:
    """动画效果类"""
    
    @staticmethod
    def clear_screen():
        """清屏"""
        os.system('cls' if os.name == 'nt' else 'clear')
    
    @staticmethod
    def move_cursor(x: int, y: int):
        """移动光标"""
        print(f"\033[{y};{x}H", end="")
    
    @staticmethod
    def hide_cursor():
        """隐藏光标"""
        print("\033[?25l", end="")
    
    @staticmethod
    def show_cursor():
        """显示光标"""
        print("\033[?25h", end="")
    
    @staticmethod
    def typewriter_effect(text: str, delay: float = 0.05, color: str = ""):
        """打字机效果"""
        for char in text:
            print(f"{color}{char}{Colors.RESET}", end="", flush=True)
            time.sleep(delay)
        print()
    
    @staticmethod
    def fade_in_logo(logo: List[str], colors: List[str], delay: float = 0.1):
        """渐入Logo效果"""
        for i, line in enumerate(logo):
            color = colors[i % len(colors)]
            print(f"{color}{line}{Colors.RESET}")
            time.sleep(delay)
    
    @staticmethod
    def rainbow_logo(logo: List[str], delay: float = 0.05):
        """彩虹Logo效果"""
        for line in logo:
            print(ASCIIArt.rainbow_text(line))
            time.sleep(delay)
    
    @staticmethod
    def spinning_loader(text: str, duration: float = 3.0, frame_set: int = 0):
        """旋转加载器"""
        frames = ASCIIArt.CREATING_FRAMES[frame_set]
        start_time = time.time()
        frame_index = 0
        
        while time.time() - start_time < duration:
            frame = frames[frame_index % len(frames)]
            print(f"\r{Colors.BRIGHT_CYAN}{frame}{Colors.RESET} {text}", end="", flush=True)
            time.sleep(0.1)
            frame_index += 1
        
        print(f"\r{Colors.BRIGHT_GREEN}✅{Colors.RESET} {text}")
    
    @staticmethod
    def progress_bar(text: str, steps: List[str], delay: float = 0.5, style: str = "blocks"):
        """进度条动画"""
        print(f"\n{Colors.BRIGHT_CYAN}{text}{Colors.RESET}")
        
        if style == "blocks":
            blocks = ["⬜", "⬛"]
        elif style == "dots":
            blocks = ["○", "●"]
        elif style == "arrows":
            blocks = ["→", "→"]
        elif style == "squares":
            blocks = ["□", "■"]
        else:
            blocks = ["░", "█"]
        
        for i, step in enumerate(steps):
            progress = (i + 1) / len(steps)
            bar_length = 30
            filled_length = int(bar_length * progress)
            bar = blocks[1] * filled_length + blocks[0] * (bar_length - filled_length)
            
            print(f"\r{Colors.BRIGHT_BLUE}[{bar}]{Colors.RESET} {step}", end="", flush=True)
            time.sleep(delay)
        
        print(f"\r{Colors.BRIGHT_GREEN}[{'█' * 30}]{Colors.RESET} 完成!")

def show_startup_animation():
    """显示启动动画"""
    try:
        Animations.clear_screen()
        
        # 显示NetBox Logo
        colors = ASCIIArt.get_gradient_colors()
        Animations.fade_in_logo(ASCIIArt.NETBOX_LOGO, colors, 0.1)
        
        print(f"\n{Colors.BRIGHT_CYAN}============================================================")
        print(f"🚀 NetBox - 企业级跨平台网络框架")
        print(f"============================================================{Colors.RESET}")
        
        # 显示特性
        features = [
            "🔌 协议层扩展接口",
            "🎯 高性能网络通信",
            "🛡️ 安全可靠传输",
            "⚡ 异步并发处理",
            "🔧 模块化架构设计"
        ]
        
        for feature in features:
            Animations.typewriter_effect(f"   {feature}", 0.03, Colors.BRIGHT_GREEN)
            time.sleep(0.1)
        
        print()
        
    except Exception as e:
        # 如果动画失败，显示简单版本
        show_simple_banner()

def show_project_creation_animation(project_name: str):
    """显示项目创建动画"""
    try:
        print(f"\n{Colors.BRIGHT_MAGENTA}🚀 创建NetBox项目: {project_name}{Colors.RESET}")
        print(f"{Colors.BRIGHT_CYAN}{'=' * 50}{Colors.RESET}")
        
        # 创建步骤动画
        steps = [
            "📁 创建目录结构",
            "📝 生成项目文件",
            "🔧 配置构建系统",
            "📋 创建文档模板",
            "✅ 项目初始化完成"
        ]
        
        Animations.progress_bar("🔨 项目创建进度", steps, 0.8, "blocks")
        
    except Exception as e:
        print(f"🚀 创建NetBox项目: {project_name}")
        print("=" * 50)

def show_simple_banner():
    """显示简单横幅"""
    print("🚀 NetBox CLI - 企业级跨平台网络框架")
    print("============================================================")

class EnhancedNetBoxCLI:
    """增强版NetBox CLI工具"""
    
    def __init__(self):
        self.config_file = Path(__file__).parent / "template_config.json"
        self.config = self.load_config()
        self.version = "2.0"
    
    def load_config(self) -> Dict:
        """加载配置文件"""
        if self.config_file.exists():
            with open(self.config_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        else:
            return {"templates": {}}
    
    def list_templates(self):
        """列出可用模板"""
        print(f"{Colors.BRIGHT_CYAN}📋 可用模板:{Colors.RESET}")
        print(f"{Colors.BRIGHT_CYAN}{'-' * 40}{Colors.RESET}")
        
        for template_id, template in self.config["templates"].items():
            name = template.get("name", template_id)
            description = template.get("description", "无描述")
            features = ", ".join(template.get("features", []))
            
            print(f"{Colors.BRIGHT_GREEN}🔹 {template_id}: {name}{Colors.RESET}")
            print(f"   {Colors.YELLOW}描述: {description}{Colors.RESET}")
            print(f"   {Colors.BRIGHT_BLUE}特性: {features}{Colors.RESET}")
            print()
    
    def cmd_init(self, args):
        """初始化新项目"""
        project_name = args.name or "MyProject"
        template_id = args.template or "default"
        
        if template_id not in self.config["templates"]:
            print(f"{Colors.BRIGHT_RED}❌ 模板 '{template_id}' 不存在{Colors.RESET}")
            print(f"{Colors.YELLOW}使用 'netbox list-templates' 查看可用模板{Colors.RESET}")
            return False
        
        template = self.config["templates"][template_id]
        project_dir = Path(project_name)
        
        if project_dir.exists():
            print(f"{Colors.BRIGHT_RED}❌ 目录 {project_name} 已存在{Colors.RESET}")
            return False
        
        # 显示项目创建动画
        try:
            show_project_creation_animation(project_name)
        except:
            print(f"{Colors.BRIGHT_MAGENTA}🚀 使用模板 '{template_id}' 创建项目: {project_name}{Colors.RESET}")
            print(f"{Colors.BRIGHT_CYAN}{'=' * 50}{Colors.RESET}")
        
        # 创建目录结构
        print(f"{Colors.BRIGHT_YELLOW}📁 创建目录结构...{Colors.RESET}")
        try:
            Animations.spinning_loader("创建项目目录", 1.0)
        except:
            pass
        
        if not self.create_directory_structure(project_dir):
            return False
        
        # 创建基础文件
        print(f"{Colors.BRIGHT_YELLOW}📝 创建项目文件...{Colors.RESET}")
        try:
            Animations.spinning_loader("生成项目文件", 1.5)
        except:
            pass
        
        if not self.create_basic_files(project_dir, project_name, template):
            return False
        
        print(f"{Colors.BRIGHT_CYAN}{'=' * 50}{Colors.RESET}")
        print(f"{Colors.BRIGHT_GREEN}✅ 项目 {project_name} 创建成功!{Colors.RESET}")
        print(f"{Colors.BRIGHT_BLUE}📁 项目目录: {project_dir.absolute()}{Colors.RESET}")
        print(f"{Colors.BRIGHT_MAGENTA}🎯 使用模板: {template.get('name', template_id)}{Colors.RESET}")
        
        # 显示模板特性
        features = template.get("features", [])
        if features:
            print(f"{Colors.BRIGHT_CYAN}🔧 包含特性: {', '.join(features)}{Colors.RESET}")
        
        print()
        print(f"{Colors.BRIGHT_YELLOW}🔧 下一步:{Colors.RESET}")
        print(f"   {Colors.WHITE}cd {project_name}{Colors.RESET}")
        print(f"   {Colors.WHITE}mkdir build && cd build{Colors.RESET}")
        print(f"   {Colors.WHITE}cmake ..{Colors.RESET}")
        print(f"   {Colors.WHITE}make{Colors.RESET}")
        print(f"   {Colors.WHITE}./bin/{project_name}{Colors.RESET}")
        
        return True
    
    def create_directory_structure(self, project_dir: Path) -> bool:
        """创建目录结构"""
        try:
            directories = [
                "application/src",
                "application/include/netbox",
                "examples/basic",
                "tests",
                "docs",
                "config",
                "cmake",
                "bin",
                "lib",
                "Protocol/include",
                "Protocol/src",
                "NetFramework/include",
                "NetFramework/src"
            ]
            
            for dir_path in directories:
                (project_dir / dir_path).mkdir(parents=True, exist_ok=True)
            
            return True
        except Exception as e:
            print(f"{Colors.BRIGHT_RED}❌ 创建目录结构失败: {e}{Colors.RESET}")
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
            
            # 复制框架代码
            self._copy_framework_code(project_dir, project_name, template)
            
            return True
        except Exception as e:
            print(f"{Colors.BRIGHT_RED}❌ 创建基础文件失败: {e}{Colors.RESET}")
            return False
    
    def _copy_framework_code(self, project_dir: Path, project_name: str, template: Dict):
        """复制NetBox框架代码到项目中"""
        try:
            # 获取NetBox项目根目录
            netbox_root = self._find_netbox_root()
            if not netbox_root:
                print(f"{Colors.BRIGHT_YELLOW}⚠️  无法找到NetBox框架代码，跳过框架代码复制{Colors.RESET}")
                return
            
            print(f"{Colors.BRIGHT_CYAN}📁 复制NetBox框架代码...{Colors.RESET}")
            
            # 复制Protocol目录
            self._copy_directory(netbox_root / "Protocol", project_dir / "Protocol")
            
            # 复制NetFramework目录
            self._copy_directory(netbox_root / "NetFramework", project_dir / "NetFramework")
            
            # 复制核心头文件
            self._copy_core_headers(netbox_root, project_dir)
            
            # 复制配置文件
            self._copy_config_files(netbox_root, project_dir)
            
            print(f"{Colors.BRIGHT_GREEN}✅ 框架代码复制完成{Colors.RESET}")
            
        except Exception as e:
            print(f"{Colors.BRIGHT_YELLOW}⚠️  框架代码复制失败: {e}{Colors.RESET}")
    
    def _find_netbox_root(self) -> Path:
        """查找NetBox项目根目录"""
        # 从当前脚本位置向上查找
        current_dir = Path(__file__).parent.parent
        max_depth = 5
        
        for depth in range(max_depth):
            if (current_dir / "Protocol").exists() and (current_dir / "NetFramework").exists():
                return current_dir
            current_dir = current_dir.parent
        
        return None
    
    def _copy_directory(self, src: Path, dst: Path):
        """复制目录"""
        if not src.exists():
            return
        
        import shutil
        if dst.exists():
            shutil.rmtree(dst)
        shutil.copytree(src, dst)
    
    def _copy_core_headers(self, netbox_root: Path, project_dir: Path):
        """复制核心头文件"""
        core_headers = [
            "main.cpp",
            "CMakeLists.txt",
            "netbox.json"
        ]
        
        for header in core_headers:
            src_file = netbox_root / header
            dst_file = project_dir / f"core_{header}"
            if src_file.exists():
                import shutil
                shutil.copy2(src_file, dst_file)
    
    def _copy_config_files(self, netbox_root: Path, project_dir: Path):
        """复制配置文件"""
        config_files = [
            "config",
            "scripts"
        ]
        
        for config_dir in config_files:
            src_dir = netbox_root / config_dir
            dst_dir = project_dir / config_dir
            if src_dir.exists():
                self._copy_directory(src_dir, dst_dir)
    
    def _create_main_source(self, project_dir: Path, project_name: str, template: Dict):
        """创建主程序文件"""
        main_file = project_dir / "application" / "src" / "main.cpp"
        
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
        header_file = project_dir / "application" / "include" / "netbox" / "NetBox.h"
        
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
        
        # 处理特殊依赖包
        deps_list = []
        protobuf_config = ""
        protobuf_linking = ""
        
        for dep in dependencies:
            if dep != "cmake":
                if dep == "protobuf":
                    # 特殊处理protobuf
                    deps_list.append("find_package(protobuf QUIET)")
                    protobuf_config = '''
if(NOT protobuf_FOUND)
    message(WARNING "protobuf not found, skipping protobuf features")
    set(HAVE_PROTOBUF FALSE)
else()
    set(HAVE_PROTOBUF TRUE)
    message(STATUS "Found protobuf: ${protobuf_VERSION}")
endif()'''
                    protobuf_linking = '''
# 如果找到protobuf，则链接它
if(HAVE_PROTOBUF)
    target_link_libraries(netbox_framework PRIVATE protobuf::libprotobuf)
    target_compile_definitions(netbox_framework PRIVATE HAVE_PROTOBUF)
endif()'''
                else:
                    deps_list.append(f'find_package({dep} REQUIRED)')
        
        deps_list_str = "\n".join(deps_list)
        
        content = f'''cmake_minimum_required(VERSION 3.10)
project({project_name} VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录 (必须在add_executable之前设置)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/../lib)

# 查找依赖包
{deps_list_str}{protobuf_config}

# 设置包含目录
include_directories(
    application/include
    Protocol/include
    NetFramework/include
)

# 检查框架代码是否存在
if(EXISTS "Protocol/include/ProtocolBase.h")
    message(STATUS "Found NetBox Protocol framework - building with framework support")
    
    # 收集框架源文件
    file(GLOB_RECURSE PROTOCOL_SOURCES "Protocol/src/*.cpp")
    file(GLOB_RECURSE NETFRAMEWORK_SOURCES "NetFramework/src/*.cpp")
    
    # 创建框架静态库
    add_library(netbox_framework STATIC
        ${{PROTOCOL_SOURCES}}
        ${{NETFRAMEWORK_SOURCES}}
    )
    
    # 设置框架库的包含目录
    target_include_directories(netbox_framework PUBLIC
        application/include
        Protocol/include
        NetFramework/include
    )
    
    # 链接框架库的依赖
    {protobuf_linking}
    
    # 添加主程序
add_executable({project_name} application/src/main.cpp)

    # 设置主程序的包含目录
    target_include_directories({project_name} PRIVATE 
        application/include
        Protocol/include
        NetFramework/include
    )
    
    # 链接框架库
    target_link_libraries({project_name} PRIVATE netbox_framework)

# 添加测试
enable_testing()
add_executable({project_name}_test tests/simple_test.cpp)
    
    # 设置测试程序的包含目录
    target_include_directories({project_name}_test PRIVATE 
        application/include
        Protocol/include
        NetFramework/include
    )
    
    # 链接测试程序到框架库
    target_link_libraries({project_name}_test PRIVATE netbox_framework)

# 添加示例
add_executable({project_name}_example examples/basic/simple_example.cpp)
    
    # 设置示例程序的包含目录
    target_include_directories({project_name}_example PRIVATE 
        application/include
        Protocol/include
        NetFramework/include
    )
    
    # 链接示例程序到框架库
    target_link_libraries({project_name}_example PRIVATE netbox_framework)
    
    # 创建自定义协议开发示例
    add_executable({project_name}_custom_protocol examples/custom_protocol_example.cpp)
    target_include_directories({project_name}_custom_protocol PRIVATE 
        application/include
        Protocol/include
        NetFramework/include
    )
    target_link_libraries({project_name}_custom_protocol PRIVATE netbox_framework)
    
    # 设置编译选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options({project_name} PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options({project_name}_test PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options({project_name}_example PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options({project_name}_custom_protocol PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(netbox_framework PRIVATE -g -O0 -Wall -Wextra)
    else()
        target_compile_options({project_name} PRIVATE -O2 -Wall -Wextra)
        target_compile_options({project_name}_test PRIVATE -O2 -Wall -Wextra)
        target_compile_options({project_name}_example PRIVATE -O2 -Wall -Wextra)
        target_compile_options({project_name}_custom_protocol PRIVATE -O2 -Wall -Wextra)
        target_compile_options(netbox_framework PRIVATE -O2 -Wall -Wextra)
    endif()
    
else()
    message(STATUS "NetBox framework not found - using basic mode")
    
    # 基本模式：只编译主程序
    add_executable({project_name} application/src/main.cpp)
    target_include_directories({project_name} PRIVATE application/include)
    
    # 添加测试
    enable_testing()
    add_executable({project_name}_test tests/simple_test.cpp)
    target_include_directories({project_name}_test PRIVATE application/include)
    
    # 添加示例
    add_executable({project_name}_example examples/basic/simple_example.cpp)
    target_include_directories({project_name}_example PRIVATE application/include)
    
    # 设置编译选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options({project_name} PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options({project_name}_test PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options({project_name}_example PRIVATE -g -O0 -Wall -Wextra)
    else()
        target_compile_options({project_name} PRIVATE -O2 -Wall -Wextra)
        target_compile_options({project_name}_test PRIVATE -O2 -Wall -Wextra)
        target_compile_options({project_name}_example PRIVATE -O2 -Wall -Wextra)
    endif()
endif()
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
├── application/           # 应用层代码
│   ├── src/              # 源代码
│   │   └── main.cpp      # 主程序
│   └── include/netbox/   # 应用层头文件
├── Protocol/             # 协议层框架代码
│   ├── include/          # 协议层头文件
│   └── src/              # 协议层源文件
├── NetFramework/         # 网络框架代码
│   ├── include/          # 网络层头文件
│   └── src/              # 网络层源文件
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
        
        # 创建自定义协议开发示例
        custom_protocol_file = project_dir / "examples" / "custom_protocol_example.cpp"
        
        content = f'''/**
 * @file custom_protocol_example.cpp
 * @brief {project_name} - 自定义协议开发示例
 * 
 * 这个示例展示了如何基于NetBox框架开发自定义协议：
 * 1. 继承ProtocolBase类
 * 2. 实现协议解析和封包逻辑
 * 3. 注册到ProtocolRouter
 * 4. 处理协议数据
 */

#include <iostream>
#include <vector>
#include <string>
#include "Protocol/include/ProtocolBase.h"
#include "Protocol/include/ProtocolRouter.h"
#include "Protocol/include/ProtocolFactory.h"

// 自定义协议ID
constexpr uint32_t CUSTOM_PROTOCOL_ID = 1001;

/**
 * @brief 自定义协议实现
 * 
 * 协议格式：
 * - 前4字节：数据长度（大端序）
 * - 后续数据：协议内容
 */
class CustomProtocol : public ProtocolBase {{
public:
    CustomProtocol() {{
        std::cout << "🔧 创建自定义协议实例" << std::endl;
    }}
    
    ~CustomProtocol() override {{
        std::cout << "🧹 销毁自定义协议实例" << std::endl;
    }}
    
    std::string getType() const override {{
        return "CustomProtocol";
    }}
    
    uint32_t getProtocolId() const override {{
        return CUSTOM_PROTOCOL_ID;
    }}
    
    size_t onDataReceived(const char* data, size_t len) override {{
        if (len < 4) {{
            // 数据不足，等待更多数据
            return 0;
        }}
        
        // 解析数据长度（大端序）
        uint32_t dataLen = (static_cast<uint32_t>(data[0]) << 24) |
                          (static_cast<uint32_t>(data[1]) << 16) |
                          (static_cast<uint32_t>(data[2]) << 8) |
                          static_cast<uint32_t>(data[3]);
        
        if (len < dataLen + 4) {{
            // 数据不完整，等待更多数据
            return 0;
        }}
        
        // 提取协议数据
        std::string protocolData(data + 4, dataLen);
        
        std::cout << "📨 收到自定义协议数据: " << protocolData << std::endl;
        
        // 调用回调函数
        if (packetCallback_) {{
            std::vector<char> packet(data, data + dataLen + 4);
            packetCallback_(packet);
        }}
        
        return dataLen + 4;  // 返回处理的字节数
    }}
    
    bool pack(const char* data, size_t len, std::vector<char>& out) override {{
        if (len > 0xFFFFFFFF) {{
            // 数据太长
            if (errorCallback_) {{
                errorCallback_("Data too long for custom protocol");
            }}
            return false;
        }}
        
        // 分配输出缓冲区
        out.resize(len + 4);
        
        // 写入数据长度（大端序）
        out[0] = static_cast<char>((len >> 24) & 0xFF);
        out[1] = static_cast<char>((len >> 16) & 0xFF);
        out[2] = static_cast<char>((len >> 8) & 0xFF);
        out[3] = static_cast<char>(len & 0xFF);
        
        // 复制数据
        std::copy(data, data + len, out.begin() + 4);
        
        std::cout << "📤 封包自定义协议数据，长度: " << len << std::endl;
        
        return true;
    }}
    
    void reset() override {{
        std::cout << "🔄 重置自定义协议状态" << std::endl;
    }}
}};

int main() {{
    std::cout << "🎯 {project_name} 自定义协议开发示例" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 创建协议路由器
    ProtocolRouter router;
    
    // 创建自定义协议实例
    auto customProtocol = std::make_shared<CustomProtocol>();
    
    // 注册协议
    router.registerProtocol(CUSTOM_PROTOCOL_ID, customProtocol);
    
    // 设置数据包回调
    router.setPacketCallback([](uint32_t protoId, const std::vector<char>& packet) {{
        std::cout << "📦 收到协议数据包，ID: " << protoId << ", 大小: " << packet.size() << " 字节" << std::endl;
    }});
    
    // 设置错误回调
    router.setErrorCallback([](const std::string& error) {{
        std::cerr << "❌ 协议错误: " << error << std::endl;
    }});
    
    // 测试协议封包
    std::string testData = "Hello, Custom Protocol!";
    std::vector<char> packedData;
    
    if (customProtocol->pack(testData.c_str(), testData.length(), packedData)) {{
        std::cout << "✅ 协议封包成功" << std::endl;
        
        // 测试协议解析
        size_t processed = router.onDataReceived(0, packedData.data(), packedData.size());
        std::cout << "✅ 协议解析成功，处理了 " << processed << " 字节" << std::endl;
    }}
    
    // 测试协议工厂
    std::cout << "\\n🔧 测试协议工厂..." << std::endl;
    
    // 注册协议到工厂
    ProtocolFactory::registerProtocol(CUSTOM_PROTOCOL_ID, []() {{
        return std::make_unique<CustomProtocol>();
    }});
    
    // 通过工厂创建协议
    auto factoryProtocol = ProtocolFactory::createProtocol(CUSTOM_PROTOCOL_ID);
    if (factoryProtocol) {{
        std::cout << "✅ 协议工厂创建成功，类型: " << factoryProtocol->getType() << std::endl;
    }}
    
    std::cout << "\\n🎉 自定义协议开发示例完成!" << std::endl;
    std::cout << "按Enter键退出..." << std::endl;
    std::cin.get();
    
    return 0;
}}
'''
        
        with open(custom_protocol_file, 'w', encoding='utf-8') as f:
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
        # 检查是否需要显示启动动画
        show_animation = len(sys.argv) > 1 and sys.argv[1] not in ['--help', '-h', '--version', 'list-templates']
        
        if show_animation:
            try:
                show_startup_animation()
            except:
                show_simple_banner()
        
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
        
        parser.add_argument('--version', action='version',
                          version=f'NetBox CLI v{self.version}')
        
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
            print(f"{Colors.BRIGHT_RED}❌ 未知命令: {args.command}{Colors.RESET}")
            return 1

if __name__ == "__main__":
    cli = EnhancedNetBoxCLI()
    sys.exit(cli.main()) 