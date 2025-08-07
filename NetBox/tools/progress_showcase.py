#!/usr/bin/env python3
"""
进度条样式展示
展示NetBox脚手架的各种进度条效果
"""

import time
import sys
from ascii_art import Animations, Colors

def showcase_all_progress_styles():
    """展示所有进度条样式"""
    print(f"{Colors.BRIGHT_CYAN}🎨 NetBox脚手架进度条样式展示{Colors.RESET}\n")
    
    styles = [
        ("blocks", "方块风格"),
        ("dots", "圆点风格"), 
        ("arrows", "箭头风格"),
        ("squares", "方形风格"),
        ("circles", "圆形风格"),
        ("gradient", "渐变风格")
    ]
    
    steps = ["初始化", "配置", "构建", "测试", "完成"]
    
    for style, description in styles:
        print(f"{Colors.BRIGHT_YELLOW}📊 {description} ({style}){Colors.RESET}")
        Animations.progress_bar(f"正在处理 - {description}", steps, 0.3, style)
        print()
        time.sleep(0.5)

def showcase_dynamic_progress():
    """展示动态进度条"""
    print(f"{Colors.BRIGHT_MAGENTA}🌊 动态进度条效果展示{Colors.RESET}\n")
    
    dynamic_styles = [
        ("wave", "波浪效果"),
        ("pulse", "脉冲效果"), 
        ("rainbow", "彩虹效果")
    ]
    
    for style, description in dynamic_styles:
        print(f"{Colors.BRIGHT_CYAN}✨ {description} ({style}){Colors.RESET}")
        Animations.animated_progress_bar(f"正在处理 - {description}", 2.0, style)
        print()
        time.sleep(0.5)

def showcase_multi_progress():
    """展示多进度条"""
    print(f"{Colors.BRIGHT_GREEN}📊 多任务进度条展示{Colors.RESET}\n")
    
    # 模拟真实的构建过程
    tasks = [
        {'name': '下载依赖包', 'progress': 0.0, 'status': 'running'},
        {'name': '编译源代码', 'progress': 0.0, 'status': 'pending'},
        {'name': '运行单元测试', 'progress': 0.0, 'status': 'pending'},
        {'name': '生成文档', 'progress': 0.0, 'status': 'pending'},
        {'name': '打包发布', 'progress': 0.0, 'status': 'pending'}
    ]
    
    # 手动控制进度更新
    Animations.hide_cursor()
    Animations.clear_screen()
    
    # 模拟任务执行
    for i in range(100):
        Animations.move_cursor(1, 1)
        
        # 更新任务进度
        if i < 20:
            tasks[0]['progress'] = min(i / 20, 1.0)
            if tasks[0]['progress'] >= 1.0:
                tasks[0]['status'] = 'completed'
                tasks[1]['status'] = 'running'
        elif i < 40:
            tasks[1]['progress'] = min((i - 20) / 20, 1.0)
            if tasks[1]['progress'] >= 1.0:
                tasks[1]['status'] = 'completed'
                tasks[2]['status'] = 'running'
        elif i < 60:
            tasks[2]['progress'] = min((i - 40) / 20, 1.0)
            if tasks[2]['progress'] >= 1.0:
                tasks[2]['status'] = 'completed'
                tasks[3]['status'] = 'running'
        elif i < 80:
            tasks[3]['progress'] = min((i - 60) / 20, 1.0)
            if tasks[3]['progress'] >= 1.0:
                tasks[3]['status'] = 'completed'
                tasks[4]['status'] = 'running'
        else:
            tasks[4]['progress'] = min((i - 80) / 20, 1.0)
            if tasks[4]['progress'] >= 1.0:
                tasks[4]['status'] = 'completed'
        
        # 显示进度条
        print(f"{Colors.BRIGHT_GREEN}🔧 NetBox项目构建进度{Colors.RESET}\n")
        
        for task in tasks:
            name = task['name']
            progress = task.get('progress', 0.0)
            status = task.get('status', 'pending')
            
            # 进度条
            bar_length = 30
            filled_length = int(bar_length * progress)
            bar = "█" * filled_length + "░" * (bar_length - filled_length)
            
            # 状态颜色和图标
            if status == 'completed':
                color = Colors.BRIGHT_GREEN
                icon = "✅"
            elif status == 'running':
                color = Colors.BRIGHT_CYAN
                icon = "🔄"
            elif status == 'error':
                color = Colors.BRIGHT_RED
                icon = "❌"
            else:
                color = Colors.DIM
                icon = "⏳"
            
            percentage = int(progress * 100)
            print(f"{icon} {color}{name:<15}{Colors.RESET} [{Colors.BRIGHT_BLUE}{bar}{Colors.RESET}] {percentage:3d}%")
        
        time.sleep(0.1)
        
        # 清屏准备下一帧
        if i < 99:
            for _ in range(7):  # 清除7行
                print("\033[1A\033[K", end="")
    
    print(f"\n{Colors.BRIGHT_GREEN}🎉 所有任务完成! NetBox项目构建成功!{Colors.RESET}")
    Animations.show_cursor()

def main():
    """主函数"""
    if len(sys.argv) > 1:
        if sys.argv[1] == "static":
            showcase_all_progress_styles()
        elif sys.argv[1] == "dynamic":
            showcase_dynamic_progress()
        elif sys.argv[1] == "multi":
            showcase_multi_progress()
        else:
            print("用法: python3 progress_showcase.py [static|dynamic|multi]")
    else:
        # 完整展示
        print(f"{Colors.BOLD}{Colors.BRIGHT_CYAN}🚀 NetBox脚手架进度条完整展示{Colors.RESET}\n")
        
        print("1️⃣  静态进度条样式...")
        time.sleep(1)
        showcase_all_progress_styles()
        
        print("\n" + "="*60 + "\n")
        
        print("2️⃣  动态进度条效果...")
        time.sleep(1)
        showcase_dynamic_progress()
        
        print("\n" + "="*60 + "\n")
        
        print("3️⃣  多任务进度条...")
        time.sleep(1)
        showcase_multi_progress()
        
        print(f"\n{Colors.BRIGHT_MAGENTA}✨ 进度条展示完成! NetBox脚手架拥有丰富的进度条效果!{Colors.RESET}")

if __name__ == "__main__":
    main()
