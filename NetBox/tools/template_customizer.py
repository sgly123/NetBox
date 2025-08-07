#!/usr/bin/env python3
"""
NetBox 模板自定义工具
允许用户交互式地自定义项目模板信息
"""

import json
import os
import sys
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime

class TemplateCustomizer:
    """模板自定义工具"""
    
    def __init__(self):
        self.config_file = Path(__file__).parent / "template_config.json"
        self.config = self.load_config()
    
    def load_config(self) -> Dict:
        """加载配置文件"""
        if self.config_file.exists():
            with open(self.config_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        else:
            return self.create_default_config()
    
    def create_default_config(self) -> Dict:
        """创建默认配置"""
        return {
            "templates": {
                "default": {
                    "name": "默认模板",
                    "description": "标准的NetBox项目模板",
                    "author": "NetBox Team",
                    "version": "1.0.0",
                    "license": "MIT",
                    "features": ["logging", "testing", "examples", "documentation"],
                    "dependencies": ["cmake", "gtest"],
                    "custom_variables": {
                        "company_name": "Your Company",
                        "project_description": "基于NetBox框架的网络应用",
                        "contact_email": "your.email@example.com",
                        "repository_url": "https://github.com/yourusername/yourproject",
                        "keywords": ["network", "cpp", "framework"],
                        "target_platforms": ["linux", "windows", "macos"]
                    }
                }
            },
            "global_settings": {
                "default_template": "default",
                "auto_generate_docs": True,
                "include_tests": True,
                "include_examples": True,
                "git_init": True,
                "create_vscode_config": True
            }
        }
    
    def save_config(self):
        """保存配置"""
        with open(self.config_file, 'w', encoding='utf-8') as f:
            json.dump(self.config, f, indent=2, ensure_ascii=False)
    
    def get_user_input(self, prompt: str, default: str = "") -> str:
        """获取用户输入"""
        if default:
            user_input = input(f"{prompt} (默认: {default}): ").strip()
            return user_input if user_input else default
        else:
            return input(f"{prompt}: ").strip()
    
    def get_yes_no(self, prompt: str, default: bool = True) -> bool:
        """获取是/否输入"""
        default_str = "Y" if default else "N"
        user_input = input(f"{prompt} (Y/n): ").strip().upper()
        if not user_input:
            return default
        return user_input in ["Y", "YES"]
    
    def customize_project_info(self, template_name: str = "default") -> Dict:
        """自定义项目信息"""
        print("🎨 NetBox 项目模板自定义")
        print("=" * 40)
        
        template = self.config["templates"].get(template_name, self.config["templates"]["default"])
        
        # 确保模板有必要的字段
        if "custom_variables" not in template:
            template["custom_variables"] = {}
        
        custom_vars = template["custom_variables"]
        
        # 设置默认值
        defaults = {
            "company_name": "Your Company",
            "project_description": "基于NetBox框架的网络应用",
            "contact_email": "your.email@example.com",
            "repository_url": "https://github.com/yourusername/yourproject",
            "keywords": ["network", "cpp", "framework"],
            "target_platforms": ["linux", "windows", "macos"]
        }
        
        # 确保所有必要的字段都存在
        for key, default_value in defaults.items():
            if key not in custom_vars:
                custom_vars[key] = default_value
        
        # 基本信息
        print("\n📋 基本信息:")
        project_name = self.get_user_input("项目名称")
        author = self.get_user_input("作者", template.get("author", "NetBox Team"))
        version = self.get_user_input("版本", template.get("version", "1.0.0"))
        license_type = self.get_user_input("许可证", template.get("license", "MIT"))
        
        # 公司信息
        print("\n🏢 公司信息:")
        company_name = self.get_user_input("公司名称", custom_vars.get("company_name", "Your Company"))
        contact_email = self.get_user_input("联系邮箱", custom_vars.get("contact_email", "your.email@example.com"))
        
        # 项目描述
        print("\n📝 项目描述:")
        project_description = self.get_user_input(
            "项目描述", 
            custom_vars.get("project_description", "基于NetBox框架的网络应用")
        )
        repository_url = self.get_user_input(
            "仓库地址", 
            custom_vars.get("repository_url", "https://github.com/yourusername/yourproject")
        )
        
        # 技术特性
        print("\n⚙️ 技术特性:")
        features = template.get("features", ["logging", "testing", "examples"]).copy()
        print(f"当前特性: {', '.join(features)}")
        
        add_features = self.get_yes_no("是否添加更多特性?")
        if add_features:
            while True:
                new_feature = input("输入新特性 (回车结束): ").strip()
                if not new_feature:
                    break
                features.append(new_feature)
        
        # 依赖项
        print("\n📦 依赖项:")
        dependencies = template.get("dependencies", ["cmake"]).copy()
        print(f"当前依赖: {', '.join(dependencies)}")
        
        add_deps = self.get_yes_no("是否添加更多依赖?")
        if add_deps:
            while True:
                new_dep = input("输入新依赖 (回车结束): ").strip()
                if not new_dep:
                    break
                dependencies.append(new_dep)
        
        # 自定义变量
        print("\n🔧 自定义变量:")
        custom_vars = custom_vars.copy()
        
        # 根据模板类型添加特定变量
        if template_name == "web_server":
            custom_vars["server_port"] = int(self.get_user_input("服务器端口", str(custom_vars.get("server_port", 8080))))
            custom_vars["max_connections"] = int(self.get_user_input("最大连接数", str(custom_vars.get("max_connections", 1000))))
            custom_vars["enable_ssl"] = self.get_yes_no("启用SSL?", custom_vars.get("enable_ssl", True))
            custom_vars["static_path"] = self.get_user_input("静态文件路径", custom_vars.get("static_path", "./public"))
        
        elif template_name == "game_server":
            custom_vars["max_players_per_room"] = int(self.get_user_input("每房间最大玩家数", str(custom_vars.get("max_players_per_room", 10))))
            custom_vars["tick_rate"] = int(self.get_user_input("游戏帧率", str(custom_vars.get("tick_rate", 60))))
            custom_vars["enable_cheat_detection"] = self.get_yes_no("启用反作弊?", custom_vars.get("enable_cheat_detection", True))
            custom_vars["game_type"] = self.get_user_input("游戏类型", custom_vars.get("game_type", "action"))
        
        elif template_name == "microservice":
            custom_vars["service_name"] = self.get_user_input("服务名称", custom_vars.get("service_name", "my-service"))
            custom_vars["service_port"] = int(self.get_user_input("服务端口", str(custom_vars.get("service_port", 9090))))
            custom_vars["enable_metrics"] = self.get_yes_no("启用指标监控?", custom_vars.get("enable_metrics", True))
            custom_vars["enable_tracing"] = self.get_yes_no("启用链路追踪?", custom_vars.get("enable_tracing", False))
        
        # 构建自定义配置
        custom_config = {
            "project_name": project_name,
            "author": author,
            "version": version,
            "license": license_type,
            "features": features,
            "dependencies": dependencies,
            "custom_variables": {
                **custom_vars,
                "company_name": company_name,
                "contact_email": contact_email,
                "project_description": project_description,
                "repository_url": repository_url,
                "creation_date": datetime.now().strftime("%Y-%m-%d"),
                "creation_year": datetime.now().year
            }
        }
        
        return custom_config
    
    def list_templates(self):
        """列出可用模板"""
        print("📋 可用模板:")
        print("-" * 40)
        
        for template_id, template in self.config["templates"].items():
            print(f"🔹 {template_id}: {template['name']}")
            print(f"   描述: {template['description']}")
            print(f"   特性: {', '.join(template['features'])}")
            print()
    
    def create_custom_template(self):
        """创建自定义模板"""
        print("🎨 创建自定义模板")
        print("=" * 30)
        
        template_id = self.get_user_input("模板ID")
        template_name = self.get_user_input("模板名称")
        description = self.get_user_input("模板描述")
        
        # 选择基础模板
        print("\n选择基础模板:")
        self.list_templates()
        base_template = self.get_user_input("基础模板ID", "default")
        
        if base_template not in self.config["templates"]:
            print("❌ 无效的基础模板")
            return
        
        # 自定义模板信息
        custom_config = self.customize_project_info(base_template)
        
        # 创建新模板
        new_template = {
            "name": template_name,
            "description": description,
            "author": custom_config["author"],
            "version": custom_config["version"],
            "license": custom_config["license"],
            "features": custom_config["features"],
            "dependencies": custom_config["dependencies"],
            "custom_variables": custom_config["custom_variables"]
        }
        
        self.config["templates"][template_id] = new_template
        self.save_config()
        
        print(f"✅ 自定义模板 '{template_id}' 创建成功!")
    
    def edit_template(self, template_id: str):
        """编辑现有模板"""
        if template_id not in self.config["templates"]:
            print(f"❌ 模板 '{template_id}' 不存在")
            return
        
        print(f"✏️ 编辑模板: {template_id}")
        print("=" * 30)
        
        custom_config = self.customize_project_info(template_id)
        
        # 更新模板
        template = self.config["templates"][template_id]
        template.update({
            "author": custom_config["author"],
            "version": custom_config["version"],
            "license": custom_config["license"],
            "features": custom_config["features"],
            "dependencies": custom_config["dependencies"],
            "custom_variables": custom_config["custom_variables"]
        })
        
        self.save_config()
        print(f"✅ 模板 '{template_id}' 更新成功!")
    
    def export_template(self, template_id: str, output_file: str):
        """导出模板配置"""
        if template_id not in self.config["templates"]:
            print(f"❌ 模板 '{template_id}' 不存在")
            return
        
        template = self.config["templates"][template_id]
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(template, f, indent=2, ensure_ascii=False)
        
        print(f"✅ 模板已导出到: {output_file}")
    
    def import_template(self, input_file: str):
        """导入模板配置"""
        if not Path(input_file).exists():
            print(f"❌ 文件不存在: {input_file}")
            return
        
        with open(input_file, 'r', encoding='utf-8') as f:
            template_data = json.load(f)
        
        template_id = self.get_user_input("模板ID")
        template_name = self.get_user_input("模板名称", template_data.get("name", ""))
        
        template_data["name"] = template_name
        self.config["templates"][template_id] = template_data
        self.save_config()
        
        print(f"✅ 模板 '{template_id}' 导入成功!")
    
    def show_help(self):
        """显示帮助信息"""
        print("""
🎨 NetBox 模板自定义工具

用法:
  python template_customizer.py [命令] [参数]

命令:
  list                   列出所有可用模板
  create                 创建新的自定义模板
  edit <template_id>     编辑指定模板
  export <template_id> <file>  导出模板到文件
  import <file>          从文件导入模板
  customize <template_id> 自定义项目信息
  help                   显示此帮助信息

示例:
  python template_customizer.py list
  python template_customizer.py create
  python template_customizer.py edit default
  python template_customizer.py export default my_template.json
  python template_customizer.py import my_template.json
        """)
    
    def run(self):
        """运行工具"""
        if len(sys.argv) < 2:
            self.show_help()
            return
        
        command = sys.argv[1]
        
        if command == "list":
            self.list_templates()
        
        elif command == "create":
            self.create_custom_template()
        
        elif command == "edit":
            if len(sys.argv) < 3:
                print("❌ 请指定模板ID")
                return
            self.edit_template(sys.argv[2])
        
        elif command == "export":
            if len(sys.argv) < 4:
                print("❌ 请指定模板ID和输出文件")
                return
            self.export_template(sys.argv[2], sys.argv[3])
        
        elif command == "import":
            if len(sys.argv) < 3:
                print("❌ 请指定输入文件")
                return
            self.import_template(sys.argv[2])
        
        elif command == "customize":
            if len(sys.argv) < 3:
                print("❌ 请指定模板ID")
                return
            custom_config = self.customize_project_info(sys.argv[2])
            print("\n📋 自定义配置:")
            print(json.dumps(custom_config, indent=2, ensure_ascii=False))
        
        elif command == "help":
            self.show_help()
        
        else:
            print(f"❌ 未知命令: {command}")
            self.show_help()

if __name__ == "__main__":
    customizer = TemplateCustomizer()
    customizer.run() 