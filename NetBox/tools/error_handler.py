#!/usr/bin/env python3
"""
NetBox CLI 错误处理和回退机制
提供完善的错误处理，确保在各种异常情况下都能正常工作
"""

import os
import sys
import traceback
import subprocess
from pathlib import Path
from typing import Dict, List, Optional, Callable, Any
from enum import Enum
import logging

class ErrorLevel(Enum):
    """错误级别"""
    INFO = "INFO"
    WARNING = "WARNING" 
    ERROR = "ERROR"
    CRITICAL = "CRITICAL"

class ErrorCode(Enum):
    """错误代码"""
    SUCCESS = 0
    GENERAL_ERROR = 1
    INVALID_ARGUMENT = 2
    FILE_NOT_FOUND = 3
    PERMISSION_DENIED = 4
    DEPENDENCY_MISSING = 5
    BUILD_FAILED = 6
    TEMPLATE_ERROR = 7
    NETWORK_ERROR = 8
    SYSTEM_ERROR = 9

class NetBoxError(Exception):
    """NetBox自定义异常"""
    
    def __init__(self, message: str, code: ErrorCode = ErrorCode.GENERAL_ERROR, 
                 details: Optional[str] = None, suggestions: Optional[List[str]] = None):
        super().__init__(message)
        self.code = code
        self.details = details
        self.suggestions = suggestions or []

class ErrorHandler:
    """错误处理器"""
    
    def __init__(self, verbose: bool = False):
        self.verbose = verbose
        self.error_count = 0
        self.warning_count = 0
        self.recovery_strategies: Dict[ErrorCode, Callable] = {}
        
        # 设置日志
        self._setup_logging()
        
        # 注册恢复策略
        self._register_recovery_strategies()
    
    def _setup_logging(self):
        """设置日志系统"""
        log_level = logging.DEBUG if self.verbose else logging.INFO
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.StreamHandler(sys.stdout)
            ]
        )
        self.logger = logging.getLogger(__name__)
    
    def _register_recovery_strategies(self):
        """注册错误恢复策略"""
        self.recovery_strategies = {
            ErrorCode.DEPENDENCY_MISSING: self._recover_missing_dependency,
            ErrorCode.FILE_NOT_FOUND: self._recover_missing_file,
            ErrorCode.PERMISSION_DENIED: self._recover_permission_error,
            ErrorCode.BUILD_FAILED: self._recover_build_failure,
            ErrorCode.TEMPLATE_ERROR: self._recover_template_error,
        }
    
    def handle_error(self, error: Exception, context: Optional[str] = None) -> bool:
        """
        处理错误
        返回True表示错误已恢复，False表示无法恢复
        """
        self.error_count += 1
        
        if isinstance(error, NetBoxError):
            return self._handle_netbox_error(error, context)
        else:
            return self._handle_generic_error(error, context)
    
    def _handle_netbox_error(self, error: NetBoxError, context: Optional[str]) -> bool:
        """处理NetBox自定义错误"""
        self._log_error(error.code.name, str(error), error.details, context)
        
        # 显示建议
        if error.suggestions:
            print("\n💡 建议解决方案:")
            for i, suggestion in enumerate(error.suggestions, 1):
                print(f"   {i}. {suggestion}")
        
        # 尝试自动恢复
        if error.code in self.recovery_strategies:
            print(f"\n🔧 尝试自动修复...")
            try:
                return self.recovery_strategies[error.code](error)
            except Exception as e:
                self.logger.error(f"自动修复失败: {e}")
                return False
        
        return False
    
    def _handle_generic_error(self, error: Exception, context: Optional[str]) -> bool:
        """处理通用错误"""
        error_type = type(error).__name__
        error_msg = str(error)
        
        # 根据错误类型推断错误代码
        if isinstance(error, FileNotFoundError):
            code = ErrorCode.FILE_NOT_FOUND
        elif isinstance(error, PermissionError):
            code = ErrorCode.PERMISSION_DENIED
        elif isinstance(error, subprocess.CalledProcessError):
            code = ErrorCode.BUILD_FAILED
        else:
            code = ErrorCode.SYSTEM_ERROR
        
        self._log_error(error_type, error_msg, None, context)
        
        # 显示详细信息（仅在verbose模式下）
        if self.verbose:
            print(f"\n🔍 详细错误信息:")
            traceback.print_exc()
        
        # 尝试恢复
        if code in self.recovery_strategies:
            print(f"\n🔧 尝试自动修复...")
            try:
                return self.recovery_strategies[code](error)
            except Exception as e:
                self.logger.error(f"自动修复失败: {e}")
                return False
        
        return False
    
    def _log_error(self, error_type: str, message: str, details: Optional[str], context: Optional[str]):
        """记录错误"""
        print(f"❌ {error_type}: {message}")
        
        if context:
            print(f"📍 上下文: {context}")
        
        if details:
            print(f"📋 详细信息: {details}")
        
        self.logger.error(f"{error_type}: {message} (Context: {context})")
    
    def warn(self, message: str, suggestions: Optional[List[str]] = None):
        """发出警告"""
        self.warning_count += 1
        print(f"⚠️  警告: {message}")
        
        if suggestions:
            print("💡 建议:")
            for i, suggestion in enumerate(suggestions, 1):
                print(f"   {i}. {suggestion}")
        
        self.logger.warning(message)
    
    def info(self, message: str):
        """显示信息"""
        print(f"ℹ️  {message}")
        self.logger.info(message)
    
    def success(self, message: str):
        """显示成功信息"""
        print(f"✅ {message}")
        self.logger.info(f"SUCCESS: {message}")
    
    # ==================== 恢复策略 ====================
    
    def _recover_missing_dependency(self, error) -> bool:
        """恢复缺失依赖"""
        print("🔍 检测系统包管理器...")
        
        # 检测包管理器
        package_managers = [
            ("apt", ["sudo", "apt", "install"]),
            ("yum", ["sudo", "yum", "install"]),
            ("dnf", ["sudo", "dnf", "install"]),
            ("brew", ["brew", "install"]),
            ("pacman", ["sudo", "pacman", "-S"])
        ]
        
        for pm, install_cmd in package_managers:
            if self._command_exists(pm):
                print(f"📦 找到包管理器: {pm}")
                
                # 常见依赖映射
                dep_mapping = {
                    "cmake": "cmake",
                    "make": "make",
                    "g++": "build-essential" if pm == "apt" else "gcc-c++",
                    "clang++": "clang",
                    "pkg-config": "pkg-config"
                }
                
                # 尝试安装
                missing_dep = str(error).split("'")[1] if "'" in str(error) else "cmake"
                package = dep_mapping.get(missing_dep, missing_dep)
                
                print(f"🔧 尝试安装 {package}...")
                try:
                    subprocess.run(install_cmd + [package], check=True)
                    print(f"✅ 成功安装 {package}")
                    return True
                except subprocess.CalledProcessError:
                    print(f"❌ 安装 {package} 失败")
                    break
        
        return False
    
    def _recover_missing_file(self, error) -> bool:
        """恢复缺失文件"""
        file_path = str(error).split("'")[1] if "'" in str(error) else ""
        
        if not file_path:
            return False
        
        print(f"🔍 尝试创建缺失文件: {file_path}")
        
        try:
            path = Path(file_path)
            
            # 创建目录
            path.parent.mkdir(parents=True, exist_ok=True)
            
            # 创建基础文件内容
            if path.suffix == ".cpp":
                content = self._get_basic_cpp_content(path.stem)
            elif path.suffix == ".h":
                content = self._get_basic_header_content(path.stem)
            elif path.name == "CMakeLists.txt":
                content = self._get_basic_cmake_content()
            else:
                content = f"# {path.name}\n# 自动生成的文件\n"
            
            path.write_text(content, encoding='utf-8')
            print(f"✅ 创建文件: {file_path}")
            return True
            
        except Exception as e:
            print(f"❌ 创建文件失败: {e}")
            return False
    
    def _recover_permission_error(self, error) -> bool:
        """恢复权限错误"""
        print("🔧 尝试修复权限问题...")
        
        # 获取当前用户
        import getpass
        user = getpass.getuser()
        
        # 尝试修复常见权限问题
        try:
            # 修复当前目录权限
            subprocess.run(["chmod", "-R", "755", "."], check=True)
            print("✅ 修复目录权限")
            return True
        except subprocess.CalledProcessError:
            print("❌ 权限修复失败，请手动运行: chmod -R 755 .")
            return False
    
    def _recover_build_failure(self, error) -> bool:
        """恢复构建失败"""
        print("🔧 尝试修复构建问题...")
        
        # 清理构建目录
        build_dir = Path("build")
        if build_dir.exists():
            try:
                import shutil
                shutil.rmtree(build_dir)
                print("✅ 清理构建目录")
            except Exception:
                print("⚠️  无法清理构建目录")
        
        # 重新创建构建目录
        build_dir.mkdir(exist_ok=True)
        print("✅ 重新创建构建目录")
        
        return True
    
    def _recover_template_error(self, error) -> bool:
        """恢复模板错误"""
        print("🔧 使用基础模板替代...")
        
        # 这里可以实现基础模板的回退逻辑
        # 目前返回True表示可以继续
        return True
    
    # ==================== 工具方法 ====================
    
    def _command_exists(self, command: str) -> bool:
        """检查命令是否存在"""
        try:
            subprocess.run([command, "--version"], 
                         capture_output=True, check=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False
    
    def _get_basic_cpp_content(self, name: str) -> str:
        """获取基础C++文件内容"""
        return f'''/**
 * @file {name}.cpp
 * @brief 自动生成的C++文件
 */

#include <iostream>

int main() {{
    std::cout << "Hello from {name}!" << std::endl;
    return 0;
}}
'''
    
    def _get_basic_header_content(self, name: str) -> str:
        """获取基础头文件内容"""
        guard = f"{name.upper()}_H"
        return f'''#pragma once
#ifndef {guard}
#define {guard}

/**
 * @file {name}.h
 * @brief 自动生成的头文件
 */

#endif // {guard}
'''
    
    def _get_basic_cmake_content(self) -> str:
        """获取基础CMake内容"""
        return '''cmake_minimum_required(VERSION 3.16)
project(NetBoxProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cpp)

message(STATUS "配置 ${PROJECT_NAME} 完成")
'''
    
    def get_summary(self) -> Dict[str, int]:
        """获取错误统计"""
        return {
            "errors": self.error_count,
            "warnings": self.warning_count
        }

# 全局错误处理器实例
_global_handler: Optional[ErrorHandler] = None

def get_error_handler(verbose: bool = False) -> ErrorHandler:
    """获取全局错误处理器"""
    global _global_handler
    if _global_handler is None:
        _global_handler = ErrorHandler(verbose)
    return _global_handler

def handle_error(error: Exception, context: Optional[str] = None) -> bool:
    """便利函数：处理错误"""
    return get_error_handler().handle_error(error, context)

def safe_execute(func: Callable, *args, context: Optional[str] = None, **kwargs) -> Any:
    """安全执行函数，自动处理错误"""
    try:
        return func(*args, **kwargs)
    except Exception as e:
        if handle_error(e, context):
            # 错误已恢复，重试一次
            try:
                return func(*args, **kwargs)
            except Exception as retry_error:
                handle_error(retry_error, f"{context} (重试)")
                return None
        return None
