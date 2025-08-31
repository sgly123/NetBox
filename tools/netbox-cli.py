#!/usr/bin/env python3
"""
NetBox CLI 工具 - 开发者命令行界面
提供项目管理、构建、测试、部署的一站式解决方案
"""

import os
import sys
import argparse
import subprocess
import json
import platform
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime

# ASCII动画模块
try:
    from ascii_art import show_startup_animation, show_project_creation_animation, Colors, Animations
    ASCII_ART_AVAILABLE = True
except ImportError:
    ASCII_ART_AVAILABLE = False
    class Colors:
        RESET = ''
        BRIGHT_GREEN = ''
        BRIGHT_CYAN = ''
        BRIGHT_YELLOW = ''

# AI助手模块
try:
    from ai_assistant import NetBoxAIAssistant
    AI_ASSISTANT_AVAILABLE = True
except ImportError:
    AI_ASSISTANT_AVAILABLE = False

# Jinja2模板引擎
try:
    from jinja2 import Environment, FileSystemLoader, select_autoescape
    JINJA2_AVAILABLE = True
except ImportError:
    JINJA2_AVAILABLE = False
    print("⚠️  Jinja2未安装，将使用基础模板功能")
    print("💡 安装命令: pip install jinja2")

class NetBoxCLI:
    """NetBox 命令行工具主类"""

    def __init__(self):
        # 处理软链接情况，确保找到正确的NetBox根目录
        script_path = Path(__file__).resolve()  # 解析软链接
        self.root_dir = script_path.parent.parent

        # 确保这是NetBox根目录（包含tools目录）
        if not (self.root_dir / "tools").exists():
            # 如果当前目录不是NetBox根目录，尝试查找
            current_dir = Path.cwd()
            if (current_dir / "NetBox" / "tools").exists():
                self.root_dir = current_dir / "NetBox"
            elif (current_dir.parent / "NetBox" / "tools").exists():
                self.root_dir = current_dir.parent / "NetBox"
            else:
                # 最后尝试：从脚本路径向上查找
                test_dir = script_path.parent
                while test_dir != test_dir.parent:
                    if (test_dir / "tools" / "templates").exists():
                        self.root_dir = test_dir
                        break
                    test_dir = test_dir.parent

        self.config = self.load_config()
        self.template_dir = self.root_dir / "tools" / "templates"

        # 初始化Jinja2环境
        if JINJA2_AVAILABLE:
            self.jinja_env = Environment(
                loader=FileSystemLoader(str(self.template_dir)),
                autoescape=select_autoescape(['html', 'xml']),
                trim_blocks=True,
                lstrip_blocks=True
            )
            # 添加自定义过滤器
            self.jinja_env.filters['upper'] = str.upper
            self.jinja_env.filters['lower'] = str.lower
            self.jinja_env.filters['title'] = str.title
        else:
            self.jinja_env = None

    def render_template(self, template_name: str, context: Dict, output_path: Path):
        """使用Jinja2渲染模板"""
        if not JINJA2_AVAILABLE or not self.jinja_env:
            # 回退到基础模板功能
            return self._render_basic_template(template_name, context, output_path)

        try:
            template = self.jinja_env.get_template(template_name)
            rendered_content = template.render(**context)

            # 确保输出目录存在
            output_path.parent.mkdir(parents=True, exist_ok=True)

            # 写入文件
            output_path.write_text(rendered_content, encoding='utf-8')
            return True

        except Exception as e:
            print(f"❌ 模板渲染失败: {template_name} -> {e}")
            # 回退到基础模板功能
            return self._render_basic_template(template_name, context, output_path)

    def _render_basic_template(self, template_name: str, context: Dict, output_path: Path):
        """基础模板渲染（不使用Jinja2）"""
        print(f"⚠️  使用基础模板渲染: {template_name}")

        try:
            # 确保输出目录存在
            output_path.parent.mkdir(parents=True, exist_ok=True)

            # 根据模板类型创建基础内容
            if template_name.endswith('main_header.h.j2'):
                content = self._create_basic_header(context)
            elif template_name.endswith('simple_server.cpp.j2'):
                content = self._create_basic_server(context)
            elif template_name.endswith('CMakeLists.txt.j2'):
                content = self._create_basic_cmake(context)
            elif template_name.endswith('dev_guide.md.j2'):
                content = self._create_basic_guide(context)
            elif template_name.endswith('project_config.json.j2'):
                content = self._create_basic_config(context)
            else:
                content = f"# {context.get('project_name', 'NetBox')} - 基础文件\n"

            # 写入文件
            output_path.write_text(content, encoding='utf-8')
            print(f"✅ 创建文件: {output_path}")
            return True

        except Exception as e:
            print(f"❌ 基础模板渲染失败: {e}")
            return False

    def get_template_context(self, project_name: str, **kwargs) -> Dict:
        """获取模板渲染上下文"""
        context = {
            'project_name': project_name,
            'date': datetime.now().strftime('%Y-%m-%d'),
            'timestamp': datetime.now().isoformat(),
            'author': kwargs.get('author', 'NetBox Developer'),
            'version': kwargs.get('version', '1.0.0'),
            'version_major': int(kwargs.get('version', '1.0.0').split('.')[0]),
            'version_minor': int(kwargs.get('version', '1.0.0').split('.')[1]),
            'version_patch': int(kwargs.get('version', '1.0.0').split('.')[2]),
            'description': kwargs.get('description', f'{project_name} - NetBox Framework Project'),
            'enable_debug': kwargs.get('debug', False),
            'enable_tests': kwargs.get('tests', True),
            'enable_examples': kwargs.get('examples', True),
            'enable_benchmarks': kwargs.get('benchmarks', False),
            'enable_logging': kwargs.get('logging', True),
            'enable_metrics': kwargs.get('metrics', True),
            'enable_signal_handling': kwargs.get('signal_handling', True),
            'enable_telnet_test': kwargs.get('telnet_test', True),
            'cpp_standard': kwargs.get('cpp_standard', 17),
            'cmake_min_version': kwargs.get('cmake_min_version', '3.16'),
            'port': kwargs.get('port', 8080),
            'echo_mode': kwargs.get('echo_mode', False),
            'features': kwargs.get('features', ['logging', 'metrics']),
            'dependencies': kwargs.get('dependencies', []),
        }

        # 合并额外的上下文
        context.update(kwargs)
        return context

    def load_config(self) -> Dict:
        """加载配置文件"""
        config_file = self.root_dir / "netbox.json"
        if config_file.exists():
            with open(config_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        return self.create_default_config()
    
    def create_default_config(self) -> Dict:
        """创建默认配置"""
        config = {
            "project": {
                "name": "NetBox",
                "version": "1.0.0",
                "description": "跨平台高性能网络框架",
                "author": "NetBox Team"
            },
            "build": {
                "default_type": "Release",
                "parallel_jobs": os.cpu_count(),
                "enable_tests": True,
                "enable_benchmarks": True
            },
            "platforms": {
                "windows": {"enabled": True, "compiler": "msvc"},
                "linux": {"enabled": True, "compiler": "gcc"},
                "macos": {"enabled": True, "compiler": "clang"}
            },
            "features": {
                "logging": True,
                "monitoring": True,
                "ssl_support": False,
                "http_support": False
            }
        }
        
        # 保存默认配置
        config_file = self.root_dir / "netbox.json"
        with open(config_file, 'w', encoding='utf-8') as f:
            json.dump(config, f, indent=2, ensure_ascii=False)
        
        return config
    
    def detect_platform(self) -> str:
        """检测当前平台"""
        system = platform.system().lower()
        if system == "windows":
            return "windows"
        elif system == "darwin":
            return "macos"
        elif system == "linux":
            return "linux"
        else:
            return "unknown"
    
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> bool:
        """执行命令"""
        try:
            result = subprocess.run(cmd, cwd=cwd or self.root_dir,
                                  capture_output=False, text=True)
            return result.returncode == 0
        except Exception as e:
            print(f"❌ 命令执行失败: {e}")
            return False

    def _create_basic_example_fallback(self, file_path: Path, project_name: str):
        """创建基础示例的回退实现"""
        try:
            # 确保目录存在
            file_path.parent.mkdir(parents=True, exist_ok=True)

            # 基础的C++示例代码
            basic_example = f'''/**
 * @file simple_server.cpp
 * @brief {project_name} 基础服务器示例
 * @date {datetime.now().strftime('%Y-%m-%d')}
 */

#include <iostream>
#include <string>

int main() {{
    std::cout << "欢迎使用 {project_name}!" << std::endl;
    std::cout << "这是一个基于NetBox框架的项目" << std::endl;

    // TODO: 在这里添加你的服务器逻辑

    return 0;
}}
'''
            file_path.write_text(basic_example, encoding='utf-8')
            print(f"✅ 创建基础示例: {file_path}")
            return True

        except Exception as e:
            print(f"❌ 创建基础示例失败: {e}")
            return False

    def cmd_init(self, args):
        """初始化新的NetBox框架项目 - 支持二次开发的框架脚手架"""
        project_name = args.name or "MyNetBoxFramework"
        project_dir = Path(project_name)

        if project_dir.exists():
            print(f"❌ 目录 {project_name} 已存在")
            return False

        # 显示项目创建动画
        if ASCII_ART_AVAILABLE:
            show_project_creation_animation(project_name)
        else:
            print(f"🚀 创建新的NetBox框架项目: {project_name}")
            print("📦 这是一个支持二次开发的完整框架项目")

        return self._create_framework_project(project_name, project_dir)

    def _create_framework_project(self, project_name: str, project_dir: Path):
        """创建支持二次开发的NetBox框架项目"""
        if not ASCII_ART_AVAILABLE:
            print("🔧 生成NetBox框架项目...")
            print("📋 项目特性:")
            print("   ✅ 协议层二次开发支持")
            print("   ✅ 应用层扩展接口")
            print("   ✅ 网络层优化接口")
            print("   ✅ 插件化架构")
            print("   ✅ 完整的示例代码")

        # 创建完整的框架目录结构
        dirs = [
            # 核心框架
            "src/core", "src/network", "src/protocol", "src/application",
            "src/plugins", "src/utils", "src/examples",

            # 头文件
            "include/netbox", "include/netbox/core", "include/netbox/network",
            "include/netbox/protocol", "include/netbox/application",
            "include/netbox/plugins", "include/netbox/utils",

            # 协议扩展
            "protocols/http", "protocols/websocket", "protocols/custom",

            # 应用层扩展
            "applications/web", "applications/game", "applications/chat",
            "applications/proxy", "applications/microservice",

            # 网络层扩展
            "network/transports", "network/codecs", "network/filters",

            # 插件系统
            "plugins/auth", "plugins/cache", "plugins/database",
            "plugins/monitoring", "plugins/logging",

            # 示例和测试
            "examples/basic", "examples/advanced", "examples/custom_protocol",
            "tests/unit", "tests/integration", "tests/performance",

            # 配置和文档
            "config", "docs", "scripts", "tools"
        ]

        # 显示目录创建进度条
        if ASCII_ART_AVAILABLE:
            from ascii_art import Animations
            print(f"\n{Colors.BRIGHT_CYAN}📁 创建项目目录结构...{Colors.RESET}")
            Animations.animated_progress_bar("正在创建目录", 2.0, "pulse")

        for dir_name in dirs:
            (project_dir / dir_name).mkdir(parents=True, exist_ok=True)

        # 创建主框架头文件
        self._create_main_framework_header(project_dir, project_name)

        # 创建协议扩展接口
        self._create_protocol_interfaces(project_dir)

        # 创建应用层接口
        self._create_application_interfaces(project_dir)

        # 创建网络层接口
        self._create_network_interfaces(project_dir)

        # 创建插件系统
        self._create_plugin_system(project_dir)

        # 创建示例代码
        self._create_framework_examples(project_dir, project_name)

        # 创建构建配置
        self._create_framework_build_config(project_dir, project_name)

        # 创建开发者文档
        self._create_developer_docs(project_dir, project_name)

        print(f"✅ NetBox框架项目 {project_name} 创建成功!")
        print(f"📁 项目目录: {project_dir.absolute()}")
        print(f"📖 开发指南: {project_name}/docs/开发指南.md")
        print(f"🔧 下一步:")
        print(f"   cd {project_name}")
        print(f"   netbox build")
        print(f"   # 查看示例代码")
        print(f"   ls examples/")
        print(f"   # 开始二次开发")
        print(f"   cat docs/开发指南.md")

        return True

    def create_hello_world_example(self, project_name: str):
        """创建Hello World示例 - 展示Jinja2模板功能"""
        project_dir = Path(project_name)
        if not project_dir.exists():
            print(f"❌ 项目目录 {project_name} 不存在")
            return False

        # 创建丰富的上下文
        context = self.get_template_context(
            project_name,
            author="NetBox开发者",
            version="1.0.0",
            enable_colors=True,
            show_info=True,
            enable_interactive=True,
            show_goodbye=True,
            features=["网络编程", "模板引擎", "跨平台", "高性能"]
        )

        # 渲染模板
        output_path = project_dir / "examples" / "hello_world.cpp"
        if self.render_template("framework/hello_world.cpp.j2", context, output_path):
            print(f"✅ Hello World示例创建成功: {output_path}")
            print(f"🔧 编译命令: g++ -o hello_world {output_path}")
            print(f"🚀 运行命令: ./hello_world")
            return True
        else:
            print("❌ Hello World示例创建失败")
            return False

    def cmd_animation(self, args):
        """测试ASCII动画效果"""
        if not ASCII_ART_AVAILABLE:
            print("❌ ASCII动画模块不可用")
            print("💡 请确保 ascii_art.py 文件存在")
            return False

        try:
            if args.type == 'startup':
                show_startup_animation()
            elif args.type == 'create':
                show_project_creation_animation(args.project)
            elif args.type == 'progress':
                self._test_progress_bars(args)
            return True
        except Exception as e:
            print(f"❌ 动画播放失败: {e}")
            return False

    def _test_progress_bars(self, args):
        """测试各种进度条效果"""
        from ascii_art import Animations

        if args.multi:
            # 多进度条测试
            print(f"{Colors.BRIGHT_CYAN}🔧 多进度条演示{Colors.RESET}\n")
            tasks = [
                {'name': '下载依赖', 'progress': 0.0, 'status': 'running'},
                {'name': '编译代码', 'progress': 0.0, 'status': 'pending'},
                {'name': '运行测试', 'progress': 0.0, 'status': 'pending'},
                {'name': '打包部署', 'progress': 0.0, 'status': 'pending'},
                {'name': '清理缓存', 'progress': 0.0, 'status': 'pending'}
            ]
            Animations.multi_progress_bars(tasks, 0.1)

        elif args.style in ['wave', 'pulse', 'rainbow']:
            # 动态进度条测试
            print(f"{Colors.BRIGHT_CYAN}🌊 动态进度条演示 - {args.style}风格{Colors.RESET}\n")
            Animations.animated_progress_bar(f"正在处理 ({args.style}风格)", 3.0, args.style)

        else:
            # 静态进度条测试
            print(f"{Colors.BRIGHT_CYAN}📊 进度条演示 - {args.style}风格{Colors.RESET}\n")
            steps = [
                "初始化环境",
                "加载配置文件",
                "连接数据库",
                "验证用户权限",
                "加载业务模块",
                "启动服务监听",
                "注册服务发现",
                "完成启动流程"
            ]
            Animations.progress_bar("🚀 系统启动进度", steps, 0.5, args.style)

    def cmd_ai(self, args):
        """AI智能助手"""
        if not AI_ASSISTANT_AVAILABLE:
            print("❌ AI助手模块不可用")
            print("💡 请确保 ai_assistant.py 文件存在")
            return False

        try:
            ai = NetBoxAIAssistant()

            if args.interactive or not args.action:
                return self._interactive_ai_session(ai)
            elif args.action == 'analyze':
                return self._ai_analyze_requirements(ai, args.input)
            elif args.action == 'suggest':
                return self._ai_suggest_template(ai, args.project_type)
            elif args.action == 'diagnose':
                return self._ai_diagnose_error(ai, args.input)

        except Exception as e:
            print(f"❌ AI助手执行失败: {e}")
            return False

    def _interactive_ai_session(self, ai):
        """交互式AI会话"""
        print(f"{Colors.BRIGHT_CYAN}🤖 NetBox AI助手 - 交互模式{Colors.RESET}")
        print("输入 'quit' 或 'exit' 退出")
        print("=" * 50)

        while True:
            try:
                user_input = input(f"\n{Colors.BRIGHT_YELLOW}💬 你: {Colors.RESET}")

                if user_input.lower() in ['quit', 'exit', 'q']:
                    print(f"{Colors.BRIGHT_GREEN}👋 再见！{Colors.RESET}")
                    break

                if not user_input.strip():
                    continue

                # 分析用户需求
                analysis = ai.analyze_requirements(user_input)

                print(f"\n{Colors.BRIGHT_CYAN}🤖 AI助手: {Colors.RESET}")
                print(f"📊 项目类型: {analysis['project_type']}")

                if analysis['protocols']:
                    print(f"🔌 推荐协议: {', '.join(analysis['protocols'])}")

                if analysis['recommendations']:
                    print(f"💡 建议:")
                    for rec in analysis['recommendations']:
                        print(f"   - {rec}")

                # 询问是否生成项目
                if analysis['project_type'] != 'unknown':
                    create = input(f"\n{Colors.BRIGHT_YELLOW}是否基于此分析创建项目? (y/n): {Colors.RESET}")
                    if create.lower() == 'y':
                        project_name = input(f"{Colors.BRIGHT_YELLOW}项目名称: {Colors.RESET}") or "AIGeneratedProject"
                        config = ai.generate_project_config(analysis, project_name)
                        print(f"\n{Colors.BRIGHT_GREEN}🚀 正在创建AI优化的项目...{Colors.RESET}")
                        # 这里可以调用增强的项目创建逻辑
                        print(f"✅ 项目 {project_name} 创建完成！")

            except KeyboardInterrupt:
                print(f"\n{Colors.BRIGHT_GREEN}👋 再见！{Colors.RESET}")
                break
            except Exception as e:
                print(f"{Colors.BRIGHT_RED}❌ 错误: {e}{Colors.RESET}")

        return True

    def _ai_analyze_requirements(self, ai, user_input):
        """AI需求分析"""
        if not user_input:
            user_input = input(f"{Colors.BRIGHT_YELLOW}请描述你的项目需求: {Colors.RESET}")

        analysis = ai.analyze_requirements(user_input)

        print(f"\n{Colors.BRIGHT_CYAN}🤖 AI需求分析结果:{Colors.RESET}")
        print(f"📊 项目类型: {analysis['project_type']}")
        print(f"🔌 推荐协议: {', '.join(analysis['protocols']) if analysis['protocols'] else '无特定推荐'}")
        print(f"⚡ 性能需求: {', '.join(analysis['performance_requirements']) if analysis['performance_requirements'] else '标准性能'}")

        if analysis['recommendations']:
            print(f"\n💡 AI建议:")
            for i, rec in enumerate(analysis['recommendations'], 1):
                print(f"   {i}. {rec}")

        return True

    def _ai_suggest_template(self, ai, project_type):
        """AI模板建议"""
        if not project_type:
            project_type = input(f"{Colors.BRIGHT_YELLOW}项目类型 (web_server/game_server/chat_server): {Colors.RESET}")

        template = ai.suggest_code_template(project_type)

        if template:
            print(f"\n{Colors.BRIGHT_CYAN}🤖 AI推荐的代码模板:{Colors.RESET}")
            print(f"{Colors.BRIGHT_GREEN}```cpp{Colors.RESET}")
            print(template)
            print(f"{Colors.BRIGHT_GREEN}```{Colors.RESET}")
        else:
            print(f"{Colors.BRIGHT_YELLOW}⚠️  暂无针对 '{project_type}' 的特定模板{Colors.RESET}")

        return True

    def _ai_diagnose_error(self, ai, error_message):
        """AI错误诊断"""
        if not error_message:
            error_message = input(f"{Colors.BRIGHT_YELLOW}请输入错误信息: {Colors.RESET}")

        diagnosis = ai.diagnose_error(error_message)

        print(f"\n{Colors.BRIGHT_CYAN}🤖 AI错误诊断:{Colors.RESET}")
        print(f"🔍 错误类型: {diagnosis['error_type']}")
        print(f"📊 置信度: {diagnosis['confidence']:.0%}")

        if diagnosis['possible_causes']:
            print(f"\n🔍 可能原因:")
            for cause in diagnosis['possible_causes']:
                print(f"   - {cause}")

        if diagnosis['solutions']:
            print(f"\n💡 解决方案:")
            for i, solution in enumerate(diagnosis['solutions'], 1):
                print(f"   {i}. {solution}")

        return True

    def _create_main_framework_header(self, project_dir: Path, project_name: str):
        """创建主框架头文件"""
        context = self.get_template_context(project_name)
        output_path = project_dir / "include" / "netbox" / "NetBox.h"

        if not self.render_template("framework/main_header.h.j2", context, output_path):
            # 回退到基础实现
            self._create_main_header_fallback(output_path, project_name)

    def _create_main_header_fallback(self, output_path: Path, project_name: str):
        """主头文件回退实现"""
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(f'''#pragma once

/**
 * @file NetBox.h
 * @brief {project_name} - NetBox框架主头文件
 */

// 核心框架
#include "core/Framework.h"
#include "core/EventLoop.h"
#include "core/ThreadPool.h"

// 网络层
#include "network/Connection.h"
#include "network/Server.h"
#include "network/Client.h"
#include "network/Transport.h"

// 协议层
#include "protocol/Protocol.h"
#include "protocol/Codec.h"
#include "protocol/Message.h"

// 应用层
#include "application/Application.h"
#include "application/Handler.h"
#include "application/Context.h"

// 插件系统
#include "plugins/Plugin.h"
#include "plugins/PluginManager.h"

// 工具类
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/Metrics.h"

namespace NetBox {{

struct Version {{
    static constexpr int MAJOR = 1;
    static constexpr int MINOR = 0;
    static constexpr int PATCH = 0;
    static constexpr const char* STRING = "1.0.0";
}};

bool initialize();
void cleanup();
const char* getVersion();

}} // namespace NetBox

#define NETBOX_VERSION_STRING NetBox::Version::STRING
#define NETBOX_INIT() NetBox::initialize()
#define NETBOX_CLEANUP() NetBox::cleanup()
''')

    def _create_protocol_interfaces(self, project_dir: Path):
        """创建协议扩展接口"""
        # 协议基类
        protocol_h = project_dir / "include" / "netbox" / "protocol" / "Protocol.h"
        protocol_h.write_text('''#pragma once

#include <memory>
#include <string>
#include <vector>

namespace NetBox::Protocol {

/**
 * @brief 协议消息基类
 */
class Message {
public:
    virtual ~Message() = default;

    // 消息类型
    virtual std::string getType() const = 0;

    // 序列化
    virtual std::vector<uint8_t> serialize() const = 0;

    // 反序列化
    virtual bool deserialize(const std::vector<uint8_t>& data) = 0;

    // 消息大小
    virtual size_t size() const = 0;

    // 消息ID
    virtual uint32_t getId() const { return 0; }

    // 消息优先级
    virtual int getPriority() const { return 0; }
};

/**
 * @brief 协议编解码器接口
 */
class Codec {
public:
    virtual ~Codec() = default;

    /**
     * @brief 编码消息
     * @param message 要编码的消息
     * @return 编码后的字节流
     */
    virtual std::vector<uint8_t> encode(const Message& message) = 0;

    /**
     * @brief 解码消息
     * @param data 字节流数据
     * @param message 解码后的消息
     * @return 解码是否成功
     */
    virtual bool decode(const std::vector<uint8_t>& data, std::unique_ptr<Message>& message) = 0;

    /**
     * @brief 检查数据完整性
     * @param data 接收到的数据
     * @return 需要的字节数，0表示完整，-1表示错误
     */
    virtual int checkIntegrity(const std::vector<uint8_t>& data) = 0;
};

/**
 * @brief 协议处理器接口
 */
class ProtocolHandler {
public:
    virtual ~ProtocolHandler() = default;

    /**
     * @brief 处理接收到的消息
     */
    virtual void onMessage(std::shared_ptr<Message> message) = 0;

    /**
     * @brief 处理连接建立
     */
    virtual void onConnect() {}

    /**
     * @brief 处理连接断开
     */
    virtual void onDisconnect() {}

    /**
     * @brief 处理错误
     */
    virtual void onError(const std::string& error) {}
};

/**
 * @brief 协议工厂接口
 */
class ProtocolFactory {
public:
    virtual ~ProtocolFactory() = default;

    /**
     * @brief 创建编解码器
     */
    virtual std::unique_ptr<Codec> createCodec() = 0;

    /**
     * @brief 创建协议处理器
     */
    virtual std::unique_ptr<ProtocolHandler> createHandler() = 0;

    /**
     * @brief 获取协议名称
     */
    virtual std::string getProtocolName() const = 0;

    /**
     * @brief 获取协议版本
     */
    virtual std::string getProtocolVersion() const = 0;
};

} // namespace NetBox::Protocol
''')

    def _create_application_interfaces(self, project_dir: Path):
        """创建应用层扩展接口"""
        app_h = project_dir / "include" / "netbox" / "application" / "Application.h"
        app_h.write_text('''#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace NetBox::Application {

/**
 * @brief 应用上下文
 */
class Context {
public:
    virtual ~Context() = default;

    // 获取连接信息
    virtual std::string getRemoteAddress() const = 0;
    virtual std::string getLocalAddress() const = 0;

    // 会话管理
    virtual void setAttribute(const std::string& key, const std::string& value) = 0;
    virtual std::string getAttribute(const std::string& key) const = 0;
    virtual bool hasAttribute(const std::string& key) const = 0;

    // 发送数据
    virtual void send(const std::vector<uint8_t>& data) = 0;
    virtual void send(const std::string& data) = 0;

    // 关闭连接
    virtual void close() = 0;
};

/**
 * @brief 应用处理器接口
 */
class Handler {
public:
    virtual ~Handler() = default;

    /**
     * @brief 处理新连接
     */
    virtual void onConnect(std::shared_ptr<Context> ctx) {}

    /**
     * @brief 处理断开连接
     */
    virtual void onDisconnect(std::shared_ptr<Context> ctx) {}

    /**
     * @brief 处理接收数据
     */
    virtual void onData(std::shared_ptr<Context> ctx, const std::vector<uint8_t>& data) {}

    /**
     * @brief 处理错误
     */
    virtual void onError(std::shared_ptr<Context> ctx, const std::string& error) {}

    /**
     * @brief 处理超时
     */
    virtual void onTimeout(std::shared_ptr<Context> ctx) {}
};

/**
 * @brief 应用基类
 */
class Application {
protected:
    std::string m_name;
    std::unordered_map<std::string, std::string> m_config;

public:
    Application(const std::string& name) : m_name(name) {}
    virtual ~Application() = default;

    // 应用生命周期
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // 配置管理
    virtual void setConfig(const std::unordered_map<std::string, std::string>& config) {
        m_config = config;
    }

    virtual std::string getConfig(const std::string& key, const std::string& defaultValue = "") const {
        auto it = m_config.find(key);
        return (it != m_config.end()) ? it->second : defaultValue;
    }

    // 应用信息
    virtual std::string getName() const { return m_name; }
    virtual std::string getVersion() const { return "1.0.0"; }
    virtual std::string getDescription() const { return "NetBox Application"; }

    // 处理器管理
    virtual void setHandler(std::shared_ptr<Handler> handler) = 0;
    virtual std::shared_ptr<Handler> getHandler() const = 0;
};

/**
 * @brief Web应用基类
 */
class WebApplication : public Application {
public:
    WebApplication(const std::string& name) : Application(name) {}

    // HTTP路由
    virtual void addRoute(const std::string& method, const std::string& path,
                         std::function<void(std::shared_ptr<Context>)> handler) = 0;

    // 静态文件服务
    virtual void serveStatic(const std::string& path, const std::string& directory) = 0;

    // 中间件
    virtual void addMiddleware(std::function<bool(std::shared_ptr<Context>)> middleware) = 0;
};

/**
 * @brief 游戏应用基类
 */
class GameApplication : public Application {
public:
    GameApplication(const std::string& name) : Application(name) {}

    // 玩家管理
    virtual void onPlayerJoin(std::shared_ptr<Context> ctx) = 0;
    virtual void onPlayerLeave(std::shared_ptr<Context> ctx) = 0;

    // 游戏逻辑
    virtual void onGameMessage(std::shared_ptr<Context> ctx, const std::string& message) = 0;
    virtual void broadcastMessage(const std::string& message) = 0;

    // 房间管理
    virtual void createRoom(const std::string& roomId) = 0;
    virtual void joinRoom(std::shared_ptr<Context> ctx, const std::string& roomId) = 0;
    virtual void leaveRoom(std::shared_ptr<Context> ctx, const std::string& roomId) = 0;
};

} // namespace NetBox::Application
''')

    def _create_network_interfaces(self, project_dir: Path):
        """创建网络层扩展接口"""
        network_h = project_dir / "include" / "netbox" / "network" / "Transport.h"
        network_h.write_text('''#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace NetBox::Network {

/**
 * @brief 网络传输层接口
 */
class Transport {
public:
    virtual ~Transport() = default;

    /**
     * @brief 绑定地址
     */
    virtual bool bind(const std::string& address, int port) = 0;

    /**
     * @brief 开始监听
     */
    virtual bool listen(int backlog = 128) = 0;

    /**
     * @brief 接受连接
     */
    virtual std::shared_ptr<Transport> accept() = 0;

    /**
     * @brief 连接到远程地址
     */
    virtual bool connect(const std::string& address, int port) = 0;

    /**
     * @brief 发送数据
     */
    virtual int send(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief 接收数据
     */
    virtual int receive(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 关闭连接
     */
    virtual void close() = 0;

    /**
     * @brief 获取本地地址
     */
    virtual std::string getLocalAddress() const = 0;

    /**
     * @brief 获取远程地址
     */
    virtual std::string getRemoteAddress() const = 0;

    /**
     * @brief 设置选项
     */
    virtual bool setOption(const std::string& name, const std::string& value) = 0;
};

/**
 * @brief 网络过滤器接口
 */
class Filter {
public:
    virtual ~Filter() = default;

    /**
     * @brief 过滤输入数据
     */
    virtual bool filterInput(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 过滤输出数据
     */
    virtual bool filterOutput(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 获取过滤器名称
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief 网络优化器接口
 */
class Optimizer {
public:
    virtual ~Optimizer() = default;

    /**
     * @brief 优化连接参数
     */
    virtual void optimizeConnection(std::shared_ptr<Transport> transport) = 0;

    /**
     * @brief 优化数据传输
     */
    virtual void optimizeTransfer(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 获取优化统计
     */
    virtual std::string getStats() const = 0;
};

/**
 * @brief 负载均衡器接口
 */
class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;

    /**
     * @brief 添加后端服务器
     */
    virtual void addBackend(const std::string& address, int port, int weight = 1) = 0;

    /**
     * @brief 移除后端服务器
     */
    virtual void removeBackend(const std::string& address, int port) = 0;

    /**
     * @brief 选择后端服务器
     */
    virtual std::pair<std::string, int> selectBackend() = 0;

    /**
     * @brief 更新服务器状态
     */
    virtual void updateBackendStatus(const std::string& address, int port, bool healthy) = 0;
};

} // namespace NetBox::Network
''')

    def _create_plugin_system(self, project_dir: Path):
        """创建插件系统"""
        plugin_h = project_dir / "include" / "netbox" / "plugins" / "Plugin.h"
        plugin_h.write_text('''#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace NetBox::Plugins {

/**
 * @brief 插件接口
 */
class Plugin {
public:
    virtual ~Plugin() = default;

    // 插件信息
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getAuthor() const = 0;

    // 插件生命周期
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // 插件配置
    virtual void configure(const std::unordered_map<std::string, std::string>& config) {}

    // 插件依赖
    virtual std::vector<std::string> getDependencies() const { return {}; }
};

/**
 * @brief 认证插件接口
 */
class AuthPlugin : public Plugin {
public:
    virtual bool authenticate(const std::string& username, const std::string& password) = 0;
    virtual bool authorize(const std::string& username, const std::string& resource) = 0;
    virtual std::string generateToken(const std::string& username) = 0;
    virtual bool validateToken(const std::string& token) = 0;
};

/**
 * @brief 缓存插件接口
 */
class CachePlugin : public Plugin {
public:
    virtual bool set(const std::string& key, const std::string& value, int ttl = 0) = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual void clear() = 0;
};

/**
 * @brief 数据库插件接口
 */
class DatabasePlugin : public Plugin {
public:
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool execute(const std::string& sql) = 0;
    virtual std::vector<std::unordered_map<std::string, std::string>> query(const std::string& sql) = 0;
    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

} // namespace NetBox::Plugins
''')

    def _create_framework_examples(self, project_dir: Path, project_name: str):
        """创建框架示例代码"""
        # 基础示例 - 使用Jinja2模板
        context = self.get_template_context(
            project_name,
            handler_class_name="SimpleHandler",
            app_class_name="SimpleApplication",
            echo_mode=True,
            enable_metrics=True,
            enable_signal_handling=True,
            enable_telnet_test=True,
            port=8080
        )

        basic_example_path = project_dir / "examples" / "basic" / "simple_server.cpp"
        if not self.render_template("framework/simple_server.cpp.j2", context, basic_example_path):
            # 回退到基础实现
            self._create_basic_example_fallback(basic_example_path, project_name)

        # 自定义协议示例
        protocol_example = project_dir / "examples" / "custom_protocol" / "custom_protocol.cpp"
        protocol_example.write_text('''/**
 * @file custom_protocol.cpp
 * @brief 自定义协议示例
 *
 * 展示如何扩展NetBox框架，实现自定义协议
 */

#include "netbox/NetBox.h"
#include <iostream>
#include <cstring>

using namespace NetBox;

/**
 * @brief 自定义消息格式
 *
 * 消息格式：
 * [4字节长度][4字节类型][数据]
 */
class CustomMessage : public Protocol::Message {
private:
    uint32_t m_type;
    std::vector<uint8_t> m_data;

public:
    CustomMessage(uint32_t type = 0) : m_type(type) {}

    void setType(uint32_t type) { m_type = type; }
    uint32_t getType() const { return m_type; }

    void setData(const std::vector<uint8_t>& data) { m_data = data; }
    void setData(const std::string& data) {
        m_data.assign(data.begin(), data.end());
    }
    const std::vector<uint8_t>& getData() const { return m_data; }

    // Protocol::Message 接口实现
    std::string getType() const override {
        return "CustomMessage";
    }

    std::vector<uint8_t> serialize() const override {
        std::vector<uint8_t> result;

        // 长度 (4字节)
        uint32_t length = 8 + m_data.size(); // 4(长度) + 4(类型) + 数据
        result.resize(4);
        std::memcpy(result.data(), &length, 4);

        // 类型 (4字节)
        result.resize(8);
        std::memcpy(result.data() + 4, &m_type, 4);

        // 数据
        result.insert(result.end(), m_data.begin(), m_data.end());

        return result;
    }

    bool deserialize(const std::vector<uint8_t>& data) override {
        if (data.size() < 8) return false;

        // 读取长度
        uint32_t length;
        std::memcpy(&length, data.data(), 4);

        if (data.size() < length) return false;

        // 读取类型
        std::memcpy(&m_type, data.data() + 4, 4);

        // 读取数据
        if (length > 8) {
            m_data.assign(data.begin() + 8, data.begin() + length);
        }

        return true;
    }

    size_t size() const override {
        return 8 + m_data.size();
    }

    uint32_t getId() const override {
        return m_type;
    }
};

/**
 * @brief 自定义协议编解码器
 */
class CustomCodec : public Protocol::Codec {
public:
    std::vector<uint8_t> encode(const Protocol::Message& message) override {
        return message.serialize();
    }

    bool decode(const std::vector<uint8_t>& data, std::unique_ptr<Protocol::Message>& message) override {
        auto customMsg = std::make_unique<CustomMessage>();
        if (customMsg->deserialize(data)) {
            message = std::move(customMsg);
            return true;
        }
        return false;
    }

    int checkIntegrity(const std::vector<uint8_t>& data) override {
        if (data.size() < 4) {
            return 4 - data.size(); // 需要更多字节来读取长度
        }

        uint32_t length;
        std::memcpy(&length, data.data(), 4);

        if (data.size() < length) {
            return length - data.size(); // 需要更多字节
        }

        return 0; // 数据完整
    }
};

/**
 * @brief 自定义协议处理器
 */
class CustomProtocolHandler : public Protocol::ProtocolHandler {
public:
    void onMessage(std::shared_ptr<Protocol::Message> message) override {
        auto customMsg = std::dynamic_pointer_cast<CustomMessage>(message);
        if (customMsg) {
            std::cout << "收到自定义消息:" << std::endl;
            std::cout << "  类型: " << customMsg->getType() << std::endl;
            std::cout << "  数据: " << std::string(customMsg->getData().begin(), customMsg->getData().end()) << std::endl;
        }
    }

    void onConnect() override {
        std::cout << "自定义协议连接建立" << std::endl;
    }

    void onDisconnect() override {
        std::cout << "自定义协议连接断开" << std::endl;
    }

    void onError(const std::string& error) override {
        std::cout << "自定义协议错误: " << error << std::endl;
    }
};

/**
 * @brief 自定义协议工厂
 */
class CustomProtocolFactory : public Protocol::ProtocolFactory {
public:
    std::unique_ptr<Protocol::Codec> createCodec() override {
        return std::make_unique<CustomCodec>();
    }

    std::unique_ptr<Protocol::ProtocolHandler> createHandler() override {
        return std::make_unique<CustomProtocolHandler>();
    }

    std::string getProtocolName() const override {
        return "CustomProtocol";
    }

    std::string getProtocolVersion() const override {
        return "1.0.0";
    }
};

int main() {
    std::cout << "🔧 NetBox自定义协议示例" << std::endl;

    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ NetBox框架初始化失败" << std::endl;
        return 1;
    }

    try {
        // 创建协议工厂
        auto factory = std::make_unique<CustomProtocolFactory>();

        std::cout << "📋 协议信息:" << std::endl;
        std::cout << "  名称: " << factory->getProtocolName() << std::endl;
        std::cout << "  版本: " << factory->getProtocolVersion() << std::endl;

        // 创建编解码器和处理器
        auto codec = factory->createCodec();
        auto handler = factory->createHandler();

        // 测试消息编解码
        std::cout << "\\n🧪 测试消息编解码:" << std::endl;

        // 创建测试消息
        CustomMessage testMsg(1001);
        testMsg.setData("Hello, Custom Protocol!");

        // 编码
        auto encoded = codec->encode(testMsg);
        std::cout << "  编码后大小: " << encoded.size() << " 字节" << std::endl;

        // 解码
        std::unique_ptr<Protocol::Message> decoded;
        if (codec->decode(encoded, decoded)) {
            std::cout << "  ✅ 解码成功" << std::endl;
            handler->onMessage(decoded);
        } else {
            std::cout << "  ❌ 解码失败" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ 异常: " << e.what() << std::endl;
    }

    // 清理框架
    NETBOX_CLEANUP();

    std::cout << "\\n✅ 自定义协议示例完成" << std::endl;
    return 0;
}''')

    def _create_framework_build_config(self, project_dir: Path, project_name: str):
        """创建框架构建配置"""
        context = self.get_template_context(
            project_name,
            features=['logging', 'metrics'],
            enable_examples=True,
            enable_tests=True,
            enable_benchmarks=False,
            enable_cpack=True,
            protocols_install=True,
            applications_install=True,
            package_formats=['TGZ', 'ZIP'],
            vendor="NetBox Team"
        )

        cmake_path = project_dir / "CMakeLists.txt"
        if not self.render_template("framework/CMakeLists.txt.j2", context, cmake_path):
            # 回退到基础实现
            self._create_cmake_fallback(cmake_path, project_name)

        # 示例CMakeLists.txt
        examples_cmake = project_dir / "examples" / "CMakeLists.txt"
        examples_cmake.write_text(f'''# 基础示例
add_executable(simple_server basic/simple_server.cpp)
target_link_libraries(simple_server {project_name}_framework)

# 自定义协议示例
add_executable(custom_protocol custom_protocol/custom_protocol.cpp)
target_link_libraries(custom_protocol {project_name}_framework)

# 高级示例
if(EXISTS "${{CMAKE_CURRENT_SOURCE_DIR}}/advanced")
    file(GLOB ADVANCED_EXAMPLES "advanced/*.cpp")
    foreach(EXAMPLE ${{ADVANCED_EXAMPLES}})
        get_filename_component(EXAMPLE_NAME ${{EXAMPLE}} NAME_WE)
        add_executable(${{EXAMPLE_NAME}} ${{EXAMPLE}})
        target_link_libraries(${{EXAMPLE_NAME}} {project_name}_framework)
    endforeach()
endif()
''')

    def _create_developer_docs(self, project_dir: Path, project_name: str):
        """创建开发者文档"""
        dev_guide = project_dir / "docs" / "开发指南.md"
        dev_guide.write_text(f'''# 🔧 {project_name} 开发指南

欢迎使用NetBox框架进行二次开发！本指南将帮助你快速上手框架的扩展开发。

## 📋 **框架架构**

{project_name}基于NetBox框架构建，采用分层架构设计：

```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (Application Layer)                │
│  - Web应用  - 游戏应用  - 微服务应用  - 自定义应用           │
├─────────────────────────────────────────────────────────────┤
│                    协议层 (Protocol Layer)                   │
│  - HTTP协议  - WebSocket  - 自定义协议  - 消息编解码         │
├─────────────────────────────────────────────────────────────┤
│                    网络层 (Network Layer)                    │
│  - 传输层抽象  - 连接管理  - 负载均衡  - 网络优化           │
├─────────────────────────────────────────────────────────────┤
│                    插件层 (Plugin Layer)                     │
│  - 认证插件  - 缓存插件  - 数据库插件  - 监控插件           │
├─────────────────────────────────────────────────────────────┤
│                    核心层 (Core Layer)                       │
│  - 事件循环  - 线程池  - 内存管理  - 配置管理               │
└─────────────────────────────────────────────────────────────┘
```

## 🚀 **快速开始**

### **1. 构建框架**
```bash
cd {project_name}
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### **2. 运行示例**
```bash
# 基础TCP服务器示例
./examples/simple_server

# 自定义协议示例
./examples/custom_protocol
```

### **3. 运行测试**
```bash
make test
```

---

## 🔌 **协议层二次开发**

### **实现自定义协议**

1. **继承Message基类**
```cpp
class MyMessage : public NetBox::Protocol::Message {{
    // 实现序列化和反序列化
    std::vector<uint8_t> serialize() const override;
    bool deserialize(const std::vector<uint8_t>& data) override;
}};
```

2. **实现Codec编解码器**
```cpp
class MyCodec : public NetBox::Protocol::Codec {{
    std::vector<uint8_t> encode(const Message& message) override;
    bool decode(const std::vector<uint8_t>& data, std::unique_ptr<Message>& message) override;
    int checkIntegrity(const std::vector<uint8_t>& data) override;
}};
```

3. **实现ProtocolHandler处理器**
```cpp
class MyProtocolHandler : public NetBox::Protocol::ProtocolHandler {{
    void onMessage(std::shared_ptr<Message> message) override;
    void onConnect() override;
    void onDisconnect() override;
}};
```

4. **创建ProtocolFactory工厂**
```cpp
class MyProtocolFactory : public NetBox::Protocol::ProtocolFactory {{
    std::unique_ptr<Codec> createCodec() override;
    std::unique_ptr<ProtocolHandler> createHandler() override;
    std::string getProtocolName() const override;
}};
```

### **协议扩展示例**

参考 `examples/custom_protocol/custom_protocol.cpp` 查看完整的自定义协议实现。

---

## 🎯 **应用层二次开发**

### **Web应用开发**

1. **继承WebApplication**
```cpp
class MyWebApp : public NetBox::Application::WebApplication {{
public:
    MyWebApp() : WebApplication("MyWebApp") {{}}

    bool initialize() override {{
        // 设置路由
        addRoute("GET", "/api/users", [](auto ctx) {{
            ctx->send("{{\\\"users\\\": []}}");
        }});

        // 设置静态文件服务
        serveStatic("/static", "./public");

        return true;
    }}
}};
```

2. **自定义中间件**
```cpp
addMiddleware([](std::shared_ptr<Context> ctx) -> bool {{
    // 认证中间件
    std::string token = ctx->getAttribute("Authorization");
    if (token.empty()) {{
        ctx->send("HTTP/1.1 401 Unauthorized\\r\\n\\r\\n");
        return false; // 阻止继续处理
    }}
    return true; // 继续处理
}});
```

### **游戏应用开发**

1. **继承GameApplication**
```cpp
class MyGameServer : public NetBox::Application::GameApplication {{
public:
    MyGameServer() : GameApplication("MyGameServer") {{}}

    void onPlayerJoin(std::shared_ptr<Context> ctx) override {{
        std::string playerId = generatePlayerId();
        ctx->setAttribute("playerId", playerId);
        broadcastMessage("Player " + playerId + " joined");
    }}

    void onGameMessage(std::shared_ptr<Context> ctx, const std::string& message) override {{
        // 处理游戏消息
        processGameLogic(ctx, message);
    }}
}};
```

---

## 🌐 **网络层二次开发**

### **自定义传输层**

1. **实现Transport接口**
```cpp
class MyTransport : public NetBox::Network::Transport {{
public:
    bool bind(const std::string& address, int port) override;
    bool listen(int backlog) override;
    std::shared_ptr<Transport> accept() override;
    int send(const std::vector<uint8_t>& data) override;
    int receive(std::vector<uint8_t>& data) override;
}};
```

### **网络优化器**

1. **实现Optimizer接口**
```cpp
class MyOptimizer : public NetBox::Network::Optimizer {{
public:
    void optimizeConnection(std::shared_ptr<Transport> transport) override {{
        // 设置TCP_NODELAY
        transport->setOption("TCP_NODELAY", "1");

        // 设置缓冲区大小
        transport->setOption("SO_RCVBUF", "65536");
        transport->setOption("SO_SNDBUF", "65536");
    }}

    void optimizeTransfer(std::vector<uint8_t>& data) override {{
        // 数据压缩
        compressData(data);
    }}
}};
```

### **负载均衡器**

1. **实现LoadBalancer接口**
```cpp
class MyLoadBalancer : public NetBox::Network::LoadBalancer {{
private:
    std::vector<Backend> m_backends;
    size_t m_currentIndex = 0;

public:
    std::pair<std::string, int> selectBackend() override {{
        // 轮询算法
        if (m_backends.empty()) return {{"", 0}};

        auto& backend = m_backends[m_currentIndex];
        m_currentIndex = (m_currentIndex + 1) % m_backends.size();

        return {{backend.address, backend.port}};
    }}
}};
```

---

## 🔌 **插件系统开发**

### **认证插件**

```cpp
class JWTAuthPlugin : public NetBox::Plugins::AuthPlugin {{
public:
    std::string getName() const override {{ return "JWTAuth"; }}
    std::string getVersion() const override {{ return "1.0.0"; }}

    bool authenticate(const std::string& username, const std::string& password) override {{
        // 验证用户名密码
        return validateCredentials(username, password);
    }}

    std::string generateToken(const std::string& username) override {{
        // 生成JWT令牌
        return createJWTToken(username);
    }}

    bool validateToken(const std::string& token) override {{
        // 验证JWT令牌
        return verifyJWTToken(token);
    }}
}};
```

### **缓存插件**

```cpp
class RedisCache : public NetBox::Plugins::CachePlugin {{
private:
    RedisConnection m_redis;

public:
    bool initialize() override {{
        return m_redis.connect("localhost:6379");
    }}

    bool set(const std::string& key, const std::string& value, int ttl) override {{
        if (ttl > 0) {{
            return m_redis.setex(key, ttl, value);
        }} else {{
            return m_redis.set(key, value);
        }}
    }}

    std::string get(const std::string& key) override {{
        return m_redis.get(key);
    }}
}};
```

---

## 📊 **性能优化指南**

### **1. 网络层优化**
- 使用合适的IO多路复用模型 (EPOLL/KQUEUE/IOCP)
- 调整TCP参数 (TCP_NODELAY, SO_REUSEPORT)
- 实现连接池和对象池
- 使用零拷贝技术

### **2. 协议层优化**
- 设计高效的序列化格式
- 实现消息压缩
- 使用批量处理
- 避免频繁的内存分配

### **3. 应用层优化**
- 异步处理长时间操作
- 实现请求缓存
- 使用数据库连接池
- 优化业务逻辑

---

## 🧪 **测试指南**

### **单元测试**
```cpp
#include <gtest/gtest.h>
#include "MyProtocol.h"

TEST(MyProtocolTest, MessageSerialization) {{
    MyMessage msg;
    msg.setData("test data");

    auto serialized = msg.serialize();
    EXPECT_GT(serialized.size(), 0);

    MyMessage decoded;
    EXPECT_TRUE(decoded.deserialize(serialized));
    EXPECT_EQ(decoded.getData(), msg.getData());
}}
```

### **集成测试**
```cpp
TEST(IntegrationTest, ServerClientCommunication) {{
    // 启动服务器
    MyServer server;
    ASSERT_TRUE(server.start("localhost", 8080));

    // 连接客户端
    MyClient client;
    ASSERT_TRUE(client.connect("localhost", 8080));

    // 发送消息
    client.send("Hello Server");

    // 验证响应
    auto response = client.receive();
    EXPECT_EQ(response, "Hello Client");
}}
```

---

## 📚 **API参考**

### **核心类**
- `NetBox::Framework` - 框架主类
- `NetBox::EventLoop` - 事件循环
- `NetBox::ThreadPool` - 线程池

### **网络类**
- `NetBox::Network::Transport` - 传输层接口
- `NetBox::Network::Connection` - 连接管理
- `NetBox::Network::Server` - 服务器基类

### **协议类**
- `NetBox::Protocol::Message` - 消息基类
- `NetBox::Protocol::Codec` - 编解码器接口
- `NetBox::Protocol::ProtocolHandler` - 协议处理器

### **应用类**
- `NetBox::Application::Application` - 应用基类
- `NetBox::Application::WebApplication` - Web应用基类
- `NetBox::Application::GameApplication` - 游戏应用基类

---

## 🤝 **贡献指南**

1. **Fork项目**
2. **创建特性分支** (`git checkout -b feature/AmazingFeature`)
3. **提交更改** (`git commit -m 'Add some AmazingFeature'`)
4. **推送分支** (`git push origin feature/AmazingFeature`)
5. **创建Pull Request**

---

## 📄 **许可证**

本项目采用MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

---

## 🆘 **获取帮助**

- 📖 查看 [API文档](api/README.md)
- 💬 加入 [讨论区](https://github.com/{project_name}/discussions)
- 🐛 报告 [问题](https://github.com/{project_name}/issues)
- 📧 联系维护者: developer@netbox.com

---

**开始你的NetBox二次开发之旅吧！** 🚀
''')

    def _create_tcp_server(self, project_name: str, project_dir: Path):
        """创建Web服务器项目 - Spring Boot级别的完整应用"""
        print("🌐 生成Web服务器项目...")

        # 创建目录结构
        dirs = [
            "src", "src/controllers", "src/middleware", "src/models", "src/services",
            "include", "static", "static/css", "static/js", "static/images",
            "templates", "config", "tests", "logs", "docs"
        ]
        for dir_name in dirs:
            (project_dir / dir_name).mkdir(parents=True, exist_ok=True)

        # 主程序文件
        main_cpp = project_dir / "src" / "main.cpp"
        main_cpp.write_text(f'''/**
 * @file main.cpp
 * @brief {project_name} - NetBox Web服务器
 *
 * 这是一个完整的Web服务器应用，包含:
 * - RESTful API支持
 * - 静态文件服务
 * - 模板引擎
 * - 中间件系统
 * - 监控和日志
 * - 热重载配置
 */

#include <iostream>
#include <memory>
#include <signal.h>
#include "NetBox.h"
#include "logging/Logger.h"
#include "config/ConfigManager.h"
#include "monitoring/MetricsCollector.h"
#include "WebServer.h"

// 全局服务器实例
std::unique_ptr<WebServer> g_server;

// 信号处理函数
void signalHandler(int signal) {{
    std::cout << "\\n🛑 接收到停止信号 (" << signal << ")，正在优雅关闭..." << std::endl;
    if (g_server) {{
        g_server->stop();
    }}
}}

int main() {{
    std::cout << "🌐 启动 {project_name} Web服务器" << std::endl;
    std::cout << "基于 NetBox 跨平台网络框架构建" << std::endl;
    std::cout << "========================================" << std::endl;

    try {{
        // 设置信号处理
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // 初始化NetBox平台
        if (!NetBox::Platform::initializePlatform()) {{
            std::cerr << "❌ NetBox平台初始化失败" << std::endl;
            return 1;
        }}

        // 加载配置
        if (!NetBox::Config::GlobalConfig::loadFromFile("config/server.json")) {{
            std::cout << "⚠️  配置文件不存在，使用默认配置" << std::endl;
        }}

        // 设置日志
        NetBox::Logging::Logger::addFileAppender("logs/{project_name.lower()}.log");
        NetBox::Logging::Logger::setMinLevel(NetBox::Logging::LogLevel::INFO);
        NETBOX_LOG_INFO("{project_name} Web服务器启动中...");

        // 创建监控指标
        auto requestCounter = NETBOX_COUNTER("http_requests_total", "HTTP请求总数");
        auto responseTimer = NETBOX_TIMER("http_response_duration", "HTTP响应时间");
        auto activeConnections = NETBOX_GAUGE("http_active_connections", "活跃连接数");

        // 创建Web服务器
        g_server = std::make_unique<WebServer>();

        // 配置路由
        g_server->setupRoutes();

        // 配置中间件
        g_server->setupMiddleware();

        // 启动服务器
        std::string host = NetBox::Config::GlobalConfig::get<std::string>("server.host", "0.0.0.0");
        int port = NetBox::Config::GlobalConfig::get<int>("server.port", 8080);

        if (g_server->start(host, port)) {{
            std::cout << "✅ Web服务器启动成功!" << std::endl;
            std::cout << "🌐 访问地址: http://" << host << ":" << port << std::endl;
            std::cout << "📊 监控面板: http://" << host << ":" << port << "/metrics" << std::endl;
            std::cout << "📖 API文档: http://" << host << ":" << port << "/docs" << std::endl;
            std::cout << "🔧 健康检查: http://" << host << ":" << port << "/health" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "按 Ctrl+C 停止服务器" << std::endl;

            NETBOX_LOG_INFO("Web服务器启动成功，监听 " + host + ":" + std::to_string(port));

            // 运行服务器
            g_server->run();
        }} else {{
            std::cerr << "❌ Web服务器启动失败" << std::endl;
            NETBOX_LOG_ERROR("Web服务器启动失败");
            return 1;
        }}

    }} catch (const std::exception& e) {{
        std::cerr << "❌ 服务器异常: " << e.what() << std::endl;
        NETBOX_LOG_FATAL("服务器异常: " + std::string(e.what()));
        return 1;
    }}

    // 清理资源
    NETBOX_LOG_INFO("Web服务器正在关闭...");
    g_server.reset();
    NetBox::Platform::cleanupPlatform();

    std::cout << "👋 {project_name} 已安全关闭" << std::endl;
    return 0;
}}''')

        # 配置文件
        config_file = project_dir / "config" / "server.json"
        config_file.write_text(f'''{{
  "server": {{
    "host": "0.0.0.0",
    "port": 8080,
    "max_connections": 10000,
    "timeout": 30,
    "keep_alive": true
  }},
  "logging": {{
    "level": "INFO",
    "file": "logs/{project_name.lower()}.log",
    "max_size": "100MB",
    "max_files": 10
  }},
  "static": {{
    "enabled": true,
    "path": "/static",
    "directory": "static",
    "cache_max_age": 3600
  }},
  "cors": {{
    "enabled": true,
    "origins": ["*"],
    "methods": ["GET", "POST", "PUT", "DELETE", "OPTIONS"],
    "headers": ["Content-Type", "Authorization"]
  }},
  "rate_limit": {{
    "enabled": true,
    "requests_per_minute": 100,
    "burst": 20
  }},
  "monitoring": {{
    "enabled": true,
    "metrics_path": "/metrics",
    "health_path": "/health"
  }}
}}''')

        print(f"✅ Web服务器项目 {project_name} 创建成功!")
        print(f"📁 项目目录: {project_dir.absolute()}")
        print(f"🔧 下一步:")
        print(f"   cd {project_name}")
        print(f"   netbox build")
        print(f"   ./{project_name}")
        print(f"🌐 然后访问: http://localhost:8080")

        return True



    def cmd_build(self, args):
        """构建项目"""
        build_type = args.type or self.config["build"]["default_type"]
        platform_name = self.detect_platform()
        
        print(f"🔨 构建NetBox项目 ({platform_name} - {build_type})")
        
        # 选择构建脚本
        if platform_name == "windows":
            script = self.root_dir / "scripts" / "build_windows.bat"
            cmd = [str(script)]
            if build_type != "Release":
                cmd.extend(["/t", build_type])
        else:
            script = self.root_dir / "scripts" / "build_cross_platform.sh"
            cmd = [str(script)]
            if build_type != "Release":
                os.environ["BUILD_TYPE"] = build_type
        
        if args.skip_tests:
            if platform_name == "windows":
                cmd.append("/skip-tests")
            else:
                os.environ["SKIP_TESTS"] = "true"
        
        return self.run_command(cmd)
    
    def cmd_test(self, args):
        """运行测试"""
        platform_name = self.detect_platform()
        build_dir = f"build_{platform_name}"
        
        print(f"🧪 运行NetBox测试套件")
        
        test_types = args.types or ["base", "io", "performance"]
        
        for test_type in test_types:
            test_exe = self.root_dir / build_dir / "tests" / "bin" / f"test_{test_type}"
            if platform_name == "windows":
                test_exe = test_exe.with_suffix(".exe")
            
            if test_exe.exists():
                print(f"  🔍 运行 {test_type} 测试...")
                if not self.run_command([str(test_exe)]):
                    print(f"  ❌ {test_type} 测试失败")
                    return False
                print(f"  ✅ {test_type} 测试通过")
            else:
                print(f"  ⚠️  {test_type} 测试不存在: {test_exe}")
        
        print("🎉 所有测试完成!")
        return True
    
    def cmd_benchmark(self, args):
        """运行性能基准测试"""
        platform_name = self.detect_platform()
        build_dir = f"build_{platform_name}"
        
        print(f"📊 运行NetBox性能基准测试")
        
        benchmark_exe = self.root_dir / build_dir / "tests" / "bin" / "test_performance"
        if platform_name == "windows":
            benchmark_exe = benchmark_exe.with_suffix(".exe")
        
        if benchmark_exe.exists():
            cmd = [str(benchmark_exe)]
            if args.filter:
                cmd.extend(["--gtest_filter", args.filter])
            return self.run_command(cmd)
        else:
            print(f"❌ 性能测试不存在: {benchmark_exe}")
            return False
    
    def cmd_clean(self, args):
        """清理构建文件"""
        print("🧹 清理构建文件...")
        
        # 清理构建目录
        import shutil
        for build_dir in self.root_dir.glob("build_*"):
            if build_dir.is_dir():
                print(f"  🗑️  删除 {build_dir}")
                shutil.rmtree(build_dir)
        
        print("✅ 清理完成!")
        return True
    
    def cmd_info(self, args):
        """显示项目信息"""
        platform_name = self.detect_platform()
        
        print("📋 NetBox 项目信息")
        print("=" * 40)
        print(f"项目名称: {self.config['project']['name']}")
        print(f"版本: {self.config['project']['version']}")
        print(f"描述: {self.config['project']['description']}")
        print(f"当前平台: {platform_name}")
        print(f"CPU核心数: {os.cpu_count()}")
        print(f"Python版本: {sys.version}")
        
        # 检查依赖
        print("\n🔍 依赖检查:")
        deps = ["cmake", "git"]
        if platform_name != "windows":
            deps.extend(["g++", "clang++"])
        
        for dep in deps:
            try:
                result = subprocess.run([dep, "--version"], 
                                      capture_output=True, text=True)
                if result.returncode == 0:
                    version = result.stdout.split('\n')[0]
                    print(f"  ✅ {dep}: {version}")
                else:
                    print(f"  ❌ {dep}: 未安装")
            except FileNotFoundError:
                print(f"  ❌ {dep}: 未找到")
        
        return True
    
    def cmd_demo(self, args):
        """运行演示程序"""
        platform_name = self.detect_platform()
        build_dir = f"build_{platform_name}"
        
        demo_exe = self.root_dir / build_dir / "platform_demo"
        if platform_name == "windows":
            demo_exe = demo_exe.with_suffix(".exe")
        
        if demo_exe.exists():
            print("🎭 运行NetBox跨平台演示...")
            return self.run_command([str(demo_exe)])
        else:
            print(f"❌ 演示程序不存在: {demo_exe}")
            print("💡 请先运行: netbox build")
            return False

    def _create_basic_header(self, context: Dict) -> str:
        """创建基础头文件"""
        project_name = context.get('project_name', 'NetBox')
        date = context.get('date', datetime.now().strftime('%Y-%m-%d'))

        return f'''#pragma once

/**
 * @file NetBox.h
 * @brief {project_name} - NetBox框架主头文件
 * @date {date}
 */

#include <iostream>
#include <string>
#include <memory>

namespace NetBox {{

/**
 * @brief 框架版本信息
 */
struct Version {{
    static constexpr int MAJOR = 1;
    static constexpr int MINOR = 0;
    static constexpr int PATCH = 0;
    static constexpr const char* STRING = "1.0.0";
}};

/**
 * @brief 框架初始化
 */
bool initialize();

/**
 * @brief 框架清理
 */
void cleanup();

}} // namespace NetBox

#define NETBOX_VERSION_STRING NetBox::Version::STRING
#define NETBOX_INIT() NetBox::initialize()
#define NETBOX_CLEANUP() NetBox::cleanup()
'''

    def _create_basic_server(self, context: Dict) -> str:
        """创建基础服务器文件"""
        project_name = context.get('project_name', 'NetBox')
        date = context.get('date', datetime.now().strftime('%Y-%m-%d'))

        return f'''/**
 * @file simple_server.cpp
 * @brief {project_name} 基础服务器示例
 * @date {date}
 */

#include <iostream>
#include <string>

int main() {{
    std::cout << "欢迎使用 {project_name}!" << std::endl;
    std::cout << "这是一个基于NetBox框架的项目" << std::endl;

    // TODO: 在这里添加你的服务器逻辑
    std::cout << "服务器启动中..." << std::endl;

    return 0;
}}
'''

    def _create_basic_cmake(self, context: Dict) -> str:
        """创建基础CMake文件"""
        project_name = context.get('project_name', 'NetBox')

        return f'''cmake_minimum_required(VERSION 3.16)
project({project_name} VERSION 1.0.0 LANGUAGES CXX)

# C++标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(${{PROJECT_NAME}}
    examples/basic/simple_server.cpp
)

# 设置输出目录
set_target_properties(${{PROJECT_NAME}} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${{CMAKE_BINARY_DIR}}/bin"
)

# 编译选项
if(MSVC)
    target_compile_options(${{PROJECT_NAME}} PRIVATE /W4)
else()
    target_compile_options(${{PROJECT_NAME}} PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "配置 {project_name} 项目完成")
'''

    def _create_basic_guide(self, context: Dict) -> str:
        """创建基础开发指南"""
        project_name = context.get('project_name', 'NetBox')
        date = context.get('date', datetime.now().strftime('%Y-%m-%d'))

        return f'''# {project_name} 开发指南

> 基于NetBox框架的项目开发指南
> 创建日期: {date}

## 项目概述

{project_name} 是一个基于NetBox框架构建的网络应用项目。

## 快速开始

### 构建项目

```bash
mkdir build && cd build
cmake ..
make
```

### 运行示例

```bash
./bin/{project_name}
```

## 项目结构

```
{project_name}/
├── examples/           # 示例代码
├── include/           # 头文件
├── src/              # 源文件
├── tests/            # 测试文件
└── CMakeLists.txt    # 构建配置
```

## 开发指南

### 添加新功能

1. 在 `src/` 目录下添加源文件
2. 在 `include/` 目录下添加头文件
3. 更新 `CMakeLists.txt`
4. 编写测试用例

### 最佳实践

- 遵循C++17标准
- 使用现代C++特性
- 编写单元测试
- 保持代码整洁

## 更多信息

- [NetBox框架文档](https://github.com/netbox/netbox)
- [项目Wiki](./wiki/)
'''

    def _create_basic_config(self, context: Dict) -> str:
        """创建基础配置文件"""
        project_name = context.get('project_name', 'NetBox')

        return f'''{{
  "project": {{
    "name": "{project_name}",
    "version": "1.0.0",
    "description": "基于NetBox框架的项目",
    "author": "NetBox Team"
  }},
  "build": {{
    "type": "Release",
    "parallel": true,
    "tests": true
  }},
  "features": {{
    "networking": true,
    "plugins": true,
    "logging": true
  }}
}}
'''

def main():
    """主函数"""
    # 显示启动动画（仅在没有参数或help时显示）
    if len(sys.argv) == 1 or (len(sys.argv) == 2 and sys.argv[1] in ['-h', '--help']):
        if ASCII_ART_AVAILABLE:
            show_startup_animation()

    parser = argparse.ArgumentParser(
        description="NetBox CLI - 跨平台网络框架开发工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
� NetBox框架脚手架 - 支持二次开发的完整框架:

框架特性:
  🔌 协议层扩展    - 自定义协议开发 (HTTP, WebSocket, 自定义协议)
  🎯 应用层扩展    - 多场景应用开发 (Web, 游戏, 微服务, 聊天)
  🌐 网络层优化    - 传输层扩展 (负载均衡, 网络优化, 过滤器)
  🔌 插件化架构    - 模块化扩展 (认证, 缓存, 数据库, 监控)

示例用法:
  netbox init MyFramework              # 创建框架项目
  cd MyFramework                       # 进入项目目录
  netbox build                         # 构建框架
  ./examples/simple_server             # 运行基础示例
  ./examples/custom_protocol           # 运行自定义协议示例

🌟 生成的框架项目包含:
  ✅ 完整的扩展接口和基类
  ✅ 协议、应用、网络层扩展示例
  ✅ 插件系统和示例插件
  ✅ 详细的开发指南和API文档
  ✅ 单元测试和集成测试框架
  ✅ CMake构建配置和工具脚本

📖 开发指南: docs/开发指南.md
🔧 API参考: include/netbox/
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='可用命令')
    
    # init 命令
    init_parser = subparsers.add_parser('init', help='初始化NetBox框架项目')
    init_parser.add_argument('name', nargs='?', help='框架项目名称')

    # hello 命令 - 演示Jinja2模板功能
    hello_parser = subparsers.add_parser('hello', help='创建Hello World示例（演示Jinja2模板）')
    hello_parser.add_argument('project', help='项目名称')
    
    # build 命令
    build_parser = subparsers.add_parser('build', help='构建项目')
    build_parser.add_argument('--type', choices=['Debug', 'Release', 'RelWithDebInfo'], 
                             help='构建类型')
    build_parser.add_argument('--skip-tests', action='store_true', help='跳过测试')
    
    # test 命令
    test_parser = subparsers.add_parser('test', help='运行测试')
    test_parser.add_argument('--types', nargs='+', 
                            choices=['base', 'io', 'performance', 'integration'],
                            help='测试类型')
    
    # benchmark 命令
    benchmark_parser = subparsers.add_parser('benchmark', help='性能基准测试')
    benchmark_parser.add_argument('--filter', help='测试过滤器')
    
    # clean 命令
    subparsers.add_parser('clean', help='清理构建文件')
    
    # info 命令
    subparsers.add_parser('info', help='显示项目信息')
    
    # demo 命令
    subparsers.add_parser('demo', help='运行演示程序')

    # animation 命令 - 测试动画效果
    anim_parser = subparsers.add_parser('animation', help='测试ASCII动画效果')
    anim_parser.add_argument('type', choices=['startup', 'create', 'progress'], help='动画类型')
    anim_parser.add_argument('--project', default='TestProject', help='项目名称（仅用于create动画）')
    anim_parser.add_argument('--style', choices=['blocks', 'dots', 'arrows', 'squares', 'circles', 'gradient', 'wave', 'pulse', 'rainbow'],
                           default='blocks', help='进度条风格')
    anim_parser.add_argument('--multi', action='store_true', help='显示多进度条')

    # ai 命令 - AI助手功能
    ai_parser = subparsers.add_parser('ai', help='AI智能助手')
    ai_parser.add_argument('action', nargs='?', choices=['analyze', 'suggest', 'diagnose'], help='AI操作类型')
    ai_parser.add_argument('--input', help='用户输入或错误信息')
    ai_parser.add_argument('--project-type', help='项目类型')
    ai_parser.add_argument('--interactive', action='store_true', help='交互式AI助手')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    cli = NetBoxCLI()
    
    # 执行命令
    command_map = {
        'init': cli.cmd_init,
        'hello': lambda args: cli.create_hello_world_example(args.project),
        'animation': cli.cmd_animation,
        'ai': cli.cmd_ai,
        'build': cli.cmd_build,
        'test': cli.cmd_test,
        'benchmark': cli.cmd_benchmark,
        'clean': cli.cmd_clean,
        'info': cli.cmd_info,
        'demo': cli.cmd_demo
    }
    
    if args.command in command_map:
        try:
            success = command_map[args.command](args)
            return 0 if success else 1
        except KeyboardInterrupt:
            print("\n⚠️  操作被用户中断")
            return 1
        except Exception as e:
            print(f"❌ 执行失败: {e}")
            return 1
    else:
        print(f"❌ 未知命令: {args.command}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
