@echo off
chcp 65001 >nul
echo 🚀 NetBox 快速构建脚本
echo ========================

REM 检查是否在项目目录中
if not exist CMakeLists.txt (
    echo ❌ 未找到CMakeLists.txt，请在项目根目录中运行
    pause
    exit /b 1
)

REM 清理旧构建
if exist build (
    echo 🧹 清理旧构建...
    rmdir /s /q build
)

REM 创建构建目录
mkdir build
cd build

echo ⚙️  配置项目...

REM 尝试Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if not errorlevel 1 (
    echo ✅ 使用 Visual Studio 2022
    goto build
)

REM 尝试Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
if not errorlevel 1 (
    echo ✅ 使用 Visual Studio 2019
    goto build
)

REM 尝试默认生成器
cmake .. >nul 2>&1
if not errorlevel 1 (
    echo ✅ 使用默认生成器
    goto build
)

echo ❌ 配置失败，请检查开发环境
echo 💡 需要安装Visual Studio或MinGW
pause
exit /b 1

:build
echo 🔧 构建项目...
cmake --build . --config Release --parallel

if errorlevel 1 (
    echo ❌ 构建失败
    pause
    exit /b 1
)

echo ✅ 构建成功!

REM 查找可执行文件
if exist Release\*.exe (
    echo 🎯 可执行文件位于: build\Release\
    dir Release\*.exe /b
) else if exist Debug\*.exe (
    echo 🎯 可执行文件位于: build\Debug\
    dir Debug\*.exe /b
) else (
    echo ⚠️  未找到可执行文件
)

echo.
echo 🎉 完成! 现在可以运行程序了
pause
