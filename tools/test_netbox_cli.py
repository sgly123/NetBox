#!/usr/bin/env python3
"""
NetBox CLI 测试套件
确保脚手架功能的正确性和稳定性
"""

import os
import sys
import unittest
import tempfile
import shutil
import subprocess
from pathlib import Path
from unittest.mock import patch, MagicMock

# 添加tools目录到Python路径
sys.path.insert(0, str(Path(__file__).parent))

try:
    from error_handler import ErrorHandler, NetBoxError, ErrorCode
except ImportError:
    print("警告: 无法导入error_handler模块")
    ErrorHandler = None

class TestNetBoxCLI(unittest.TestCase):
    """NetBox CLI 测试类"""
    
    def setUp(self):
        """测试前准备"""
        self.test_dir = Path(tempfile.mkdtemp())
        self.original_cwd = Path.cwd()
        os.chdir(self.test_dir)
        
        # CLI脚本路径
        self.cli_script = self.original_cwd / "tools" / "netbox-cli-v2.py"
        
    def tearDown(self):
        """测试后清理"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.test_dir, ignore_errors=True)
    
    def run_cli(self, args: list, expect_success: bool = True):
        """运行CLI命令"""
        cmd = [sys.executable, str(self.cli_script)] + args
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if expect_success:
            self.assertEqual(result.returncode, 0, 
                           f"CLI命令失败: {' '.join(args)}\n"
                           f"stdout: {result.stdout}\n"
                           f"stderr: {result.stderr}")
        
        return result
    
    def test_cli_info(self):
        """测试info命令"""
        result = self.run_cli(["info"])
        self.assertIn("NetBox CLI v2.0", result.stdout)
        self.assertIn("版本:", result.stdout)
        self.assertIn("平台:", result.stdout)
    
    def test_cli_help(self):
        """测试帮助命令"""
        result = self.run_cli(["--help"])
        self.assertIn("NetBox CLI v2.0", result.stdout)
        self.assertIn("init", result.stdout)
        self.assertIn("build", result.stdout)
        self.assertIn("test", result.stdout)
    
    def test_project_creation(self):
        """测试项目创建"""
        project_name = "TestProject"
        result = self.run_cli(["init", project_name])
        
        # 检查输出
        self.assertIn("创建成功", result.stdout)
        
        # 检查项目目录
        project_dir = self.test_dir / project_name
        self.assertTrue(project_dir.exists())
        
        # 检查关键文件
        key_files = [
            "src/main.cpp",
            "include/netbox/NetBox.h",
            "CMakeLists.txt",
            "README.md",
            "tests/simple_test.cpp"
        ]
        
        for file_path in key_files:
            full_path = project_dir / file_path
            self.assertTrue(full_path.exists(), f"缺少文件: {file_path}")
            self.assertGreater(full_path.stat().st_size, 0, f"文件为空: {file_path}")
    
    def test_project_build(self):
        """测试项目构建"""
        project_name = "BuildTest"
        
        # 创建项目
        self.run_cli(["init", project_name])
        
        # 进入项目目录
        project_dir = self.test_dir / project_name
        os.chdir(project_dir)
        
        # 检查是否有cmake
        try:
            subprocess.run(["cmake", "--version"], 
                         capture_output=True, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            self.skipTest("CMake未安装，跳过构建测试")
        
        # 构建项目
        result = self.run_cli(["build"])
        
        # 检查构建结果
        build_dir = project_dir / "build"
        self.assertTrue(build_dir.exists())
        
        # 检查可执行文件
        exe_files = list(build_dir.glob("bin/*"))
        self.assertGreater(len(exe_files), 0, "未找到可执行文件")
    
    def test_project_test(self):
        """测试项目测试"""
        project_name = "TestTest"
        
        # 创建和构建项目
        self.run_cli(["init", project_name])
        project_dir = self.test_dir / project_name
        os.chdir(project_dir)
        
        # 检查cmake
        try:
            subprocess.run(["cmake", "--version"], 
                         capture_output=True, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            self.skipTest("CMake未安装，跳过测试")
        
        # 构建项目
        self.run_cli(["build"])
        
        # 运行测试
        result = self.run_cli(["test"])
        self.assertIn("测试通过", result.stdout)
    
    def test_invalid_command(self):
        """测试无效命令"""
        result = self.run_cli(["invalid_command"], expect_success=False)
        self.assertNotEqual(result.returncode, 0)
        # 检查stderr或stdout中的错误信息
        error_output = result.stdout + result.stderr
        self.assertTrue("未知命令" in error_output or "invalid choice" in error_output or "error" in error_output.lower())
    
    def test_project_name_validation(self):
        """测试项目名称验证"""
        # 测试空名称
        result = self.run_cli(["init", ""])
        # 应该使用默认名称或报错
        
        # 测试特殊字符
        result = self.run_cli(["init", "test@project"])
        # 应该能处理或给出警告
    
    def test_existing_directory(self):
        """测试已存在目录的处理"""
        project_name = "ExistingDir"
        
        # 创建目录
        (self.test_dir / project_name).mkdir()
        
        # 尝试创建同名项目
        result = self.run_cli(["init", project_name], expect_success=False)
        self.assertIn("已存在", result.stdout)

class TestErrorHandler(unittest.TestCase):
    """错误处理器测试类"""
    
    def setUp(self):
        """测试前准备"""
        if ErrorHandler is None:
            self.skipTest("ErrorHandler模块不可用")
        
        self.handler = ErrorHandler(verbose=True)
    
    def test_error_handling(self):
        """测试错误处理"""
        # 测试NetBox错误
        error = NetBoxError("测试错误", ErrorCode.GENERAL_ERROR, 
                          suggestions=["建议1", "建议2"])
        
        result = self.handler.handle_error(error, "测试上下文")
        # 应该返回False（无法自动恢复）
        self.assertFalse(result)
    
    def test_warning(self):
        """测试警告"""
        self.handler.warn("测试警告", ["建议1"])
        self.assertEqual(self.handler.warning_count, 1)
    
    def test_success_info(self):
        """测试成功和信息消息"""
        self.handler.success("测试成功")
        self.handler.info("测试信息")
        # 不应该增加错误计数
        self.assertEqual(self.handler.error_count, 0)
    
    def test_command_exists(self):
        """测试命令存在检查"""
        # 测试存在的命令
        self.assertTrue(self.handler._command_exists("python3"))
        
        # 测试不存在的命令
        self.assertFalse(self.handler._command_exists("nonexistent_command_12345"))

class TestTemplateSystem(unittest.TestCase):
    """模板系统测试类"""
    
    def setUp(self):
        """测试前准备"""
        self.templates_dir = Path(__file__).parent / "templates_v2"
    
    def test_template_configs(self):
        """测试模板配置文件"""
        # 检查基础模板
        basic_config = self.templates_dir / "basic" / "template.json"
        if basic_config.exists():
            import json
            with open(basic_config) as f:
                config = json.load(f)
            
            # 检查必需字段
            required_fields = ["name", "version", "description", "level"]
            for field in required_fields:
                self.assertIn(field, config, f"缺少字段: {field}")
        
        # 检查标准模板
        standard_config = self.templates_dir / "standard" / "template.json"
        if standard_config.exists():
            import json
            with open(standard_config) as f:
                config = json.load(f)
            
            self.assertEqual(config["name"], "standard")
            self.assertEqual(config["level"], 2)

class TestIntegration(unittest.TestCase):
    """集成测试类"""
    
    def setUp(self):
        """测试前准备"""
        self.test_dir = Path(tempfile.mkdtemp())
        self.original_cwd = Path.cwd()
        os.chdir(self.test_dir)
        
        self.cli_script = self.original_cwd / "tools" / "netbox-cli-v2.py"
    
    def tearDown(self):
        """测试后清理"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.test_dir, ignore_errors=True)
    
    def test_full_workflow(self):
        """测试完整工作流程"""
        project_name = "FullWorkflowTest"
        
        # 1. 创建项目
        result = subprocess.run([
            sys.executable, str(self.cli_script), "init", project_name
        ], capture_output=True, text=True)
        
        self.assertEqual(result.returncode, 0, f"项目创建失败: {result.stderr}")
        
        # 2. 检查项目结构
        project_dir = self.test_dir / project_name
        self.assertTrue(project_dir.exists())
        
        # 3. 进入项目目录
        os.chdir(project_dir)
        
        # 4. 检查文件内容
        main_cpp = project_dir / "src" / "main.cpp"
        self.assertTrue(main_cpp.exists())
        
        content = main_cpp.read_text()
        self.assertIn(project_name, content)
        self.assertIn("#include", content)
        
        # 5. 检查CMakeLists.txt
        cmake_file = project_dir / "CMakeLists.txt"
        self.assertTrue(cmake_file.exists())
        
        cmake_content = cmake_file.read_text()
        self.assertIn(project_name, cmake_content)
        self.assertIn("cmake_minimum_required", cmake_content)

def run_tests():
    """运行所有测试"""
    # 创建测试套件
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # 添加测试类
    test_classes = [
        TestNetBoxCLI,
        TestErrorHandler,
        TestTemplateSystem,
        TestIntegration
    ]
    
    for test_class in test_classes:
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    
    # 运行测试
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # 返回测试结果
    return result.wasSuccessful()

if __name__ == "__main__":
    print("🧪 开始NetBox CLI测试")
    print("=" * 50)
    
    success = run_tests()
    
    print("=" * 50)
    if success:
        print("✅ 所有测试通过!")
        sys.exit(0)
    else:
        print("❌ 部分测试失败!")
        sys.exit(1)
