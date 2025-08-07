#!/usr/bin/env python3
"""
ASCII艺术字和动画模块
为NetBox提供酷炫的启动动画和项目创建动画
"""

import time
import sys
import random
import math
from typing import List, Optional

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
    
    # NetBox项目标题 (与主Logo相同)
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
        """给文本添加颜色"""
        return f"{color}{text}{Colors.RESET}"
    
    @staticmethod
    def rainbow_text(text: str) -> str:
        """彩虹色文本"""
        colors = ASCIIArt.get_gradient_colors()
        result = ""
        for i, char in enumerate(text):
            if char != ' ':
                color = colors[i % len(colors)]
                result += f"{color}{char}{Colors.RESET}"
            else:
                result += char
        return result

class Animations:
    """动画效果类"""
    
    @staticmethod
    def clear_screen():
        """清屏"""
        print("\033[2J\033[H", end="")
    
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
        """Logo淡入效果"""
        Animations.hide_cursor()
        
        # 逐行显示
        for i, line in enumerate(logo):
            color = colors[i % len(colors)]
            print(f"{color}{line}{Colors.RESET}")
            time.sleep(delay)
        
        Animations.show_cursor()
    
    @staticmethod
    def rainbow_logo(logo: List[str], delay: float = 0.05):
        """彩虹Logo效果"""
        Animations.hide_cursor()
        colors = ASCIIArt.get_gradient_colors()
        
        for line in logo:
            rainbow_line = ""
            for i, char in enumerate(line):
                if char != ' ' and char != '╚' and char != '═' and char != '╝':
                    color = colors[i % len(colors)]
                    rainbow_line += f"{color}{char}{Colors.RESET}"
                else:
                    rainbow_line += char
            print(rainbow_line)
            time.sleep(delay)
        
        Animations.show_cursor()
    
    @staticmethod
    def spinning_loader(text: str, duration: float = 3.0, frame_set: int = 0):
        """旋转加载动画"""
        frames = ASCIIArt.CREATING_FRAMES[frame_set]
        Animations.hide_cursor()
        
        start_time = time.time()
        frame_index = 0
        
        while time.time() - start_time < duration:
            frame = frames[frame_index % len(frames)]
            print(f"\r{Colors.BRIGHT_CYAN}{frame}{Colors.RESET} {text}", end="", flush=True)
            time.sleep(0.1)
            frame_index += 1
        
        print(f"\r{Colors.BRIGHT_GREEN}✅{Colors.RESET} {text}")
        Animations.show_cursor()
    
    @staticmethod
    def progress_bar(text: str, steps: List[str], delay: float = 0.5, style: str = "blocks"):
        """增强的进度条动画"""
        Animations.hide_cursor()

        # 不同风格的进度条字符
        styles = {
            "blocks": {"filled": "█", "empty": "░", "partial": ["▏", "▎", "▍", "▌", "▋", "▊", "▉"]},
            "dots": {"filled": "●", "empty": "○", "partial": ["◔", "◑", "◕"]},
            "arrows": {"filled": "▶", "empty": "▷", "partial": ["▷"]},
            "squares": {"filled": "■", "empty": "□", "partial": ["▣", "▤", "▥", "▦", "▧", "▨", "▩"]},
            "circles": {"filled": "⬤", "empty": "⬜", "partial": ["◐", "◓", "◑", "◒"]},
            "gradient": {"filled": "█", "empty": " ", "partial": ["▁", "▂", "▃", "▄", "▅", "▆", "▇"]}
        }

        style_chars = styles.get(style, styles["blocks"])
        total_steps = len(steps)
        bar_length = 40

        for i, step in enumerate(steps):
            # 计算进度
            progress = (i + 1) / total_steps
            filled_length = int(bar_length * progress)
            percentage = int(progress * 100)

            # 创建渐变色进度条
            bar_parts = []
            colors = [Colors.BRIGHT_RED, Colors.BRIGHT_YELLOW, Colors.BRIGHT_GREEN, Colors.BRIGHT_CYAN, Colors.BRIGHT_BLUE, Colors.BRIGHT_MAGENTA]

            for j in range(bar_length):
                if j < filled_length:
                    color = colors[min(j * len(colors) // bar_length, len(colors) - 1)]
                    bar_parts.append(f"{color}{style_chars['filled']}{Colors.RESET}")
                else:
                    bar_parts.append(f"{Colors.DIM}{style_chars['empty']}{Colors.RESET}")

            bar = "".join(bar_parts)

            # 显示进度条
            print(f"\r{Colors.BRIGHT_CYAN}{text}{Colors.RESET}")
            print(f"[{bar}] {Colors.BRIGHT_WHITE}{percentage:3d}%{Colors.RESET} - {Colors.BRIGHT_YELLOW}{step}{Colors.RESET}")

            if i < total_steps - 1:
                time.sleep(delay)
                # 清除当前行
                print("\033[2A\033[K\033[K", end="")

        Animations.show_cursor()

    @staticmethod
    def animated_progress_bar(text: str, duration: float = 5.0, style: str = "wave"):
        """动态进度条动画"""
        Animations.hide_cursor()

        start_time = time.time()
        bar_length = 50

        while time.time() - start_time < duration:
            elapsed = time.time() - start_time
            progress = min(elapsed / duration, 1.0)

            if style == "wave":
                # 波浪效果
                bar_parts = []
                for i in range(bar_length):
                    pos_progress = i / bar_length
                    if pos_progress <= progress:
                        # 使用sin函数创建波浪效果
                        wave_offset = int(3 * abs(math.sin(elapsed * 3 + i * 0.3)))
                        wave_chars = ["▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"]
                        char = wave_chars[min(wave_offset, len(wave_chars) - 1)]
                        color = Colors.BRIGHT_CYAN if wave_offset > 4 else Colors.CYAN
                        bar_parts.append(f"{color}{char}{Colors.RESET}")
                    else:
                        bar_parts.append(f"{Colors.DIM}░{Colors.RESET}")

            elif style == "pulse":
                # 脉冲效果
                pulse_intensity = abs(math.sin(elapsed * 4))
                filled_length = int(bar_length * progress)

                bar_parts = []
                for i in range(bar_length):
                    if i < filled_length:
                        if pulse_intensity > 0.7:
                            bar_parts.append(f"{Colors.BRIGHT_WHITE}█{Colors.RESET}")
                        elif pulse_intensity > 0.3:
                            bar_parts.append(f"{Colors.BRIGHT_CYAN}█{Colors.RESET}")
                        else:
                            bar_parts.append(f"{Colors.CYAN}█{Colors.RESET}")
                    else:
                        bar_parts.append(f"{Colors.DIM}░{Colors.RESET}")

            elif style == "rainbow":
                # 彩虹效果
                colors = [Colors.BRIGHT_RED, Colors.BRIGHT_YELLOW, Colors.BRIGHT_GREEN,
                         Colors.BRIGHT_CYAN, Colors.BRIGHT_BLUE, Colors.BRIGHT_MAGENTA]
                filled_length = int(bar_length * progress)

                bar_parts = []
                for i in range(bar_length):
                    if i < filled_length:
                        color_index = (i + int(elapsed * 10)) % len(colors)
                        color = colors[color_index]
                        bar_parts.append(f"{color}█{Colors.RESET}")
                    else:
                        bar_parts.append(f"{Colors.DIM}░{Colors.RESET}")

            bar = "".join(bar_parts)
            percentage = int(progress * 100)

            print(f"\r{Colors.BRIGHT_CYAN}{text}{Colors.RESET}")
            print(f"[{bar}] {Colors.BRIGHT_WHITE}{percentage:3d}%{Colors.RESET}", end="")

            time.sleep(0.05)
            if progress < 1.0:
                print("\033[1A\033[K", end="")

        print(f"\n{Colors.BRIGHT_GREEN}✅ 完成!{Colors.RESET}")
        Animations.show_cursor()

    @staticmethod
    def multi_progress_bars(tasks: List[dict], update_interval: float = 0.1):
        """多个进度条同时显示"""
        Animations.hide_cursor()
        Animations.clear_screen()

        total_tasks = len(tasks)
        completed_tasks = 0

        while completed_tasks < total_tasks:
            Animations.move_cursor(1, 1)

            for i, task in enumerate(tasks):
                name = task['name']
                progress = task.get('progress', 0.0)
                status = task.get('status', 'pending')

                # 进度条
                bar_length = 30
                filled_length = int(bar_length * progress)
                bar = "█" * filled_length + "░" * (bar_length - filled_length)

                # 状态颜色
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
                print(f"{icon} {color}{name:<20}{Colors.RESET} [{Colors.BRIGHT_BLUE}{bar}{Colors.RESET}] {percentage:3d}%")

            # 更新进度（这里是示例，实际使用时需要外部更新）
            for task in tasks:
                if task.get('status') == 'running' and task.get('progress', 0) < 1.0:
                    task['progress'] = min(task.get('progress', 0) + 0.02, 1.0)
                    if task['progress'] >= 1.0:
                        task['status'] = 'completed'
                        completed_tasks += 1

            time.sleep(update_interval)

        print(f"\n{Colors.BRIGHT_GREEN}🎉 所有任务完成!{Colors.RESET}")
        Animations.show_cursor()
    
    @staticmethod
    def matrix_effect(duration: float = 2.0):
        """矩阵雨效果"""
        Animations.hide_cursor()
        Animations.clear_screen()
        
        width = 80
        height = 20
        
        # 初始化矩阵
        matrix = [[' ' for _ in range(width)] for _ in range(height)]
        
        start_time = time.time()
        while time.time() - start_time < duration:
            # 随机添加字符
            for _ in range(5):
                x = random.randint(0, width - 1)
                y = random.randint(0, height - 1)
                matrix[y][x] = random.choice("▓▒░")
            
            # 显示矩阵
            Animations.move_cursor(1, 1)
            for row in matrix:
                line = ''.join(row)
                print(f"{Colors.BRIGHT_GREEN}{line}{Colors.RESET}")
            
            time.sleep(0.1)
            
            # 清除一些字符
            for _ in range(3):
                x = random.randint(0, width - 1)
                y = random.randint(0, height - 1)
                matrix[y][x] = ' '
        
        Animations.clear_screen()
        Animations.show_cursor()

def show_startup_animation():
    """显示启动动画"""
    try:
        Animations.clear_screen()
        
        # 矩阵效果
        print(f"{Colors.BRIGHT_GREEN}初始化NetBox...{Colors.RESET}")
        Animations.matrix_effect(1.5)
        
        # 显示Logo
        print(f"\n{Colors.BOLD}{Colors.BRIGHT_CYAN}欢迎使用{Colors.RESET}")
        Animations.rainbow_logo(ASCIIArt.NETBOX_LOGO, 0.03)
        
        print(f"\n{Colors.BRIGHT_YELLOW}{'=' * 60}{Colors.RESET}")
        Animations.typewriter_effect(
            "🚀 NetBox - 企业级跨平台网络框架",
            0.03,
            Colors.BRIGHT_WHITE
        )
        print(f"{Colors.BRIGHT_YELLOW}{'=' * 60}{Colors.RESET}\n")
        
        # 特性展示
        features = [
            "🔌 协议层扩展接口",
            "🎯 应用层扩展接口", 
            "🌐 网络层优化接口",
            "🔌 插件化架构",
            "🎨 Jinja2模板引擎"
        ]
        
        for feature in features:
            Animations.typewriter_effect(f"   {feature}", 0.02, Colors.BRIGHT_CYAN)
            time.sleep(0.1)
        
        print()
        
    except KeyboardInterrupt:
        Animations.show_cursor()
        print(f"\n{Colors.BRIGHT_RED}启动被中断{Colors.RESET}")

def show_project_creation_animation(project_name: str):
    """显示项目创建动画"""
    try:
        print(f"\n{Colors.BOLD}{Colors.BRIGHT_MAGENTA}创建项目: {project_name}{Colors.RESET}")
        Animations.fade_in_logo(ASCIIArt.FRAMEWORK_TITLE, ASCIIArt.get_gradient_colors(), 0.05)
        
        # 创建步骤
        steps = [
            "初始化NetBox项目结构",
            "生成NetBox核心文件",
            "创建协议扩展接口",
            "设置应用层接口",
            "配置网络层接口",
            "初始化插件系统",
            "生成示例代码",
            "创建CMake构建配置",
            "生成项目文档"
        ]
        
        print(f"\n{Colors.BRIGHT_YELLOW}正在创建项目...{Colors.RESET}\n")

        # 使用增强的进度条，随机选择风格
        styles = ["blocks", "gradient", "circles", "squares"]
        selected_style = random.choice(styles)
        Animations.progress_bar("🔧 项目生成进度", steps, 0.4, selected_style)
        
        # 成功动画
        print(f"\n{Colors.BRIGHT_GREEN}{'🎉' * 20}{Colors.RESET}")
        Animations.typewriter_effect(
            f"✅ 项目 {project_name} 创建成功!", 
            0.05, 
            Colors.BRIGHT_GREEN
        )
        print(f"{Colors.BRIGHT_GREEN}{'🎉' * 20}{Colors.RESET}\n")
        
    except KeyboardInterrupt:
        Animations.show_cursor()
        print(f"\n{Colors.BRIGHT_RED}创建被中断{Colors.RESET}")

if __name__ == "__main__":
    # 测试动画
    if len(sys.argv) > 1:
        if sys.argv[1] == "startup":
            show_startup_animation()
        elif sys.argv[1] == "create":
            project_name = sys.argv[2] if len(sys.argv) > 2 else "TestProject"
            show_project_creation_animation(project_name)
    else:
        show_startup_animation()
        time.sleep(1)
        show_project_creation_animation("MyAwesomeProject")
