#!/usr/bin/env python3
"""
NetBox 项目代码量统计脚本
"""

import os
import glob

def count_lines_in_file(filepath):
    """统计文件行数"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            return len(f.readlines())
    except:
        return 0

def count_files_by_pattern(pattern, description):
    """按模式统计文件"""
    files = glob.glob(pattern, recursive=True)
    # 排除build目录
    files = [f for f in files if 'build' not in f]
    
    total_lines = 0
    file_count = len(files)
    
    print(f"\n=== {description} ===")
    print(f"文件数量: {file_count}")
    
    if file_count > 0:
        for file in sorted(files):
            lines = count_lines_in_file(file)
            total_lines += lines
            print(f"  {file}: {lines} 行")
        
        print(f"总行数: {total_lines}")
    
    return file_count, total_lines

def main():
    os.chdir('/home/sgly/桌面/NetBox')
    
    print("NetBox 项目代码量统计报告")
    print("=" * 50)
    
    total_files = 0
    total_lines = 0
    
    # C++ 源文件
    cpp_files, cpp_lines = count_files_by_pattern("**/*.cpp", "C++ 源文件 (.cpp)")
    total_files += cpp_files
    total_lines += cpp_lines
    
    # C++ 头文件
    h_files, h_lines = count_files_by_pattern("**/*.h", "C++ 头文件 (.h)")
    total_files += h_files
    total_lines += h_lines
    
    # 主程序
    main_files, main_lines = count_files_by_pattern("main.cpp", "主程序文件")
    total_files += main_files
    total_lines += main_lines
    
    # Python 脚本
    py_files, py_lines = count_files_by_pattern("**/*.py", "Python 脚本 (.py)")
    total_files += py_files
    total_lines += py_lines
    
    # Shell 脚本
    sh_files, sh_lines = count_files_by_pattern("**/*.sh", "Shell 脚本 (.sh)")
    total_files += sh_files
    total_lines += sh_lines
    
    # CMake 文件
    cmake_files, cmake_lines = count_files_by_pattern("**/CMakeLists.txt", "CMake 构建文件")
    total_files += cmake_files
    total_lines += cmake_lines
    
    # 配置文件
    config_files, config_lines = count_files_by_pattern("**/*.yaml", "YAML 配置文件")
    config_files2, config_lines2 = count_files_by_pattern("**/*.txt", "文本配置文件")
    total_files += config_files + config_files2
    total_lines += config_lines + config_lines2
    
    # 文档文件
    doc_files, doc_lines = count_files_by_pattern("**/*.md", "Markdown 文档")
    total_files += doc_files
    total_lines += doc_lines
    
    print(f"\n" + "=" * 50)
    print(f"项目总计:")
    print(f"  总文件数: {total_files}")
    print(f"  总代码行数: {total_lines}")
    
    # 核心代码统计（只包含C++代码）
    core_files = cpp_files + h_files + main_files
    core_lines = cpp_lines + h_lines + main_lines
    
    print(f"\n核心代码统计 (C++):")
    print(f"  核心文件数: {core_files}")
    print(f"  核心代码行数: {core_lines}")
    
    # 校招项目评估
    print(f"\n" + "=" * 50)
    print("校招项目代码量评估:")
    
    if core_lines >= 5000:
        level = "优秀"
        desc = "代码量充足，体现了扎实的编程功底"
    elif core_lines >= 3000:
        level = "良好" 
        desc = "代码量适中，满足校招项目要求"
    elif core_lines >= 2000:
        level = "合格"
        desc = "代码量基本满足要求，建议继续扩展"
    else:
        level = "需要扩展"
        desc = "代码量偏少，建议增加更多功能模块"
    
    print(f"  评估等级: {level}")
    print(f"  评估说明: {desc}")
    print(f"  核心代码: {core_lines} 行")
    print(f"  项目总计: {total_lines} 行")

if __name__ == "__main__":
    main()
