#!/usr/bin/env python3
"""
NetBox 自定义模板应用工具
将自定义的模板信息应用到项目创建中
"""

import json
import sys
from pathlib import Path
from typing import Dict, Optional

class CustomTemplateApplier:
    """自定义模板应用工具"""
    
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
    
    def apply_custom_template(self, template_id: str, custom_config: Dict) -> bool:
        """应用自定义模板配置"""
        try:
            # 更新模板配置
            if template_id not in self.config["templates"]:
                self.config["templates"][template_id] = {}
            
            template = self.config["templates"][template_id]
            
            # 更新基本信息
            template.update({
                "name": custom_config.get("project_name", "自定义模板"),
                "author": custom_config.get("author", "NetBox Team"),
                "version": custom_config.get("version", "1.0.0"),
                "license": custom_config.get("license", "MIT"),
                "features": custom_config.get("features", []),
                "dependencies": custom_config.get("dependencies", []),
                "custom_variables": custom_config.get("custom_variables", {})
            })
            
            # 保存配置
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(self.config, f, indent=2, ensure_ascii=False)
            
            print(f"✅ 自定义模板 '{template_id}' 应用成功!")
            return True
            
        except Exception as e:
            print(f"❌ 应用自定义模板失败: {e}")
            return False
    
    def create_project_with_custom_template(self, project_name: str, template_id: str, custom_config: Dict) -> bool:
        """使用自定义模板创建项目"""
        try:
            # 首先应用自定义模板
            if not self.apply_custom_template(template_id, custom_config):
                return False
            
            # 然后创建项目
            print(f"🚀 使用自定义模板创建项目: {project_name}")
            
            # 这里可以调用netbox init命令
            import subprocess
            result = subprocess.run([
                "netbox", "init", project_name
            ], capture_output=True, text=True)
            
            if result.returncode == 0:
                print(f"✅ 项目 '{project_name}' 创建成功!")
                print(f"📁 项目目录: {Path.cwd() / project_name}")
                
                # 显示项目信息
                self.show_project_info(project_name, custom_config)
                return True
            else:
                print(f"❌ 项目创建失败: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"❌ 创建项目失败: {e}")
            return False
    
    def show_project_info(self, project_name: str, custom_config: Dict):
        """显示项目信息"""
        print(f"\n📋 项目信息:")
        print(f"   项目名称: {project_name}")
        print(f"   作者: {custom_config.get('author', 'Unknown')}")
        print(f"   版本: {custom_config.get('version', '1.0.0')}")
        print(f"   许可证: {custom_config.get('license', 'MIT')}")
        print(f"   公司: {custom_config.get('custom_variables', {}).get('company_name', 'Unknown')}")
        print(f"   描述: {custom_config.get('custom_variables', {}).get('project_description', 'No description')}")
        
        features = custom_config.get('features', [])
        if features:
            print(f"   特性: {', '.join(features)}")
        
        dependencies = custom_config.get('dependencies', [])
        if dependencies:
            print(f"   依赖: {', '.join(dependencies)}")
    
    def load_custom_config_from_file(self, file_path: str) -> Optional[Dict]:
        """从文件加载自定义配置"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception as e:
            print(f"❌ 加载配置文件失败: {e}")
            return None
    
    def save_custom_config_to_file(self, config: Dict, file_path: str) -> bool:
        """保存自定义配置到文件"""
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                json.dump(config, f, indent=2, ensure_ascii=False)
            print(f"✅ 配置已保存到: {file_path}")
            return True
        except Exception as e:
            print(f"❌ 保存配置文件失败: {e}")
            return False
    
    def show_help(self):
        """显示帮助信息"""
        print("""
🎨 NetBox 自定义模板应用工具

用法:
  python apply_custom_template.py [命令] [参数]

命令:
  apply <template_id> <config_file>    应用自定义配置到模板
  create <project_name> <template_id> <config_file>  使用自定义模板创建项目
  save <config> <file>                 保存配置到文件
  load <file>                          从文件加载配置
  help                                 显示此帮助信息

示例:
  python apply_custom_template.py apply default my_config.json
  python apply_custom_template.py create MyProject default my_config.json
  python apply_custom_template.py save '{"project_name":"test"}' test.json
  python apply_custom_template.py load test.json
        """)
    
    def run(self):
        """运行工具"""
        if len(sys.argv) < 2:
            self.show_help()
            return
        
        command = sys.argv[1]
        
        if command == "apply":
            if len(sys.argv) < 4:
                print("❌ 请指定模板ID和配置文件")
                return
            
            template_id = sys.argv[2]
            config_file = sys.argv[3]
            
            custom_config = self.load_custom_config_from_file(config_file)
            if custom_config:
                self.apply_custom_template(template_id, custom_config)
        
        elif command == "create":
            if len(sys.argv) < 5:
                print("❌ 请指定项目名称、模板ID和配置文件")
                return
            
            project_name = sys.argv[2]
            template_id = sys.argv[3]
            config_file = sys.argv[4]
            
            custom_config = self.load_custom_config_from_file(config_file)
            if custom_config:
                self.create_project_with_custom_template(project_name, template_id, custom_config)
        
        elif command == "save":
            if len(sys.argv) < 4:
                print("❌ 请指定配置和文件路径")
                return
            
            config_str = sys.argv[2]
            file_path = sys.argv[3]
            
            try:
                config = json.loads(config_str)
                self.save_custom_config_to_file(config, file_path)
            except json.JSONDecodeError:
                print("❌ 无效的JSON配置")
        
        elif command == "load":
            if len(sys.argv) < 3:
                print("❌ 请指定文件路径")
                return
            
            file_path = sys.argv[2]
            config = self.load_custom_config_from_file(file_path)
            if config:
                print("📋 配置内容:")
                print(json.dumps(config, indent=2, ensure_ascii=False))
        
        elif command == "help":
            self.show_help()
        
        else:
            print(f"❌ 未知命令: {command}")
            self.show_help()

if __name__ == "__main__":
    applier = CustomTemplateApplier()
    applier.run() 