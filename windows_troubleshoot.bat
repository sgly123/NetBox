@echo off
chcp 65001 >nul
echo 🔧 NetBox Windows 故障排除工具
echo ================================

echo.
echo 📋 检查系统环境...
echo.

REM 检查CMake
echo 🔍 检查CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ❌ CMake 未安装或不在PATH中
    echo 💡 解决方案:
    echo    1. 下载CMake: https://cmake.org/download/
    echo    2. 或使用Chocolatey: choco install cmake
    echo.
    goto check_compiler
) else (
    for /f "tokens=3" %%i in ('cmake --version 2^>nul ^| findstr "cmake version"') do (
        echo ✅ CMake 版本: %%i
    )
)

:check_compiler
echo.
echo 🔍 检查C++编译器...

REM 检查Visual Studio
where cl.exe >nul 2>&1
if not errorlevel 1 (
    echo ✅ 找到 MSVC 编译器
    cl.exe 2>&1 | findstr "Microsoft"
    goto check_python
)

REM 检查MinGW
where g++.exe >nul 2>&1
if not errorlevel 1 (
    echo ✅ 找到 MinGW 编译器
    g++ --version | findstr "g++"
    goto check_python
)

echo ❌ 未找到C++编译器
echo 💡 解决方案:
echo    1. 安装Visual Studio Community: https://visualstudio.microsoft.com/
echo    2. 或安装MinGW: https://www.mingw-w64.org/
echo    3. 确保选择了C++开发工具

:check_python
echo.
echo 🔍 检查Python...
python --version >nul 2>&1
if errorlevel 1 (
    python3 --version >nul 2>&1
    if errorlevel 1 (
        echo ❌ Python 未安装
        echo 💡 解决方案: https://www.python.org/downloads/
    ) else (
        echo ✅ Python3 可用
        python3 --version
    )
) else (
    echo ✅ Python 可用
    python --version
)

echo.
echo 📁 检查项目结构...

if not exist CMakeLists.txt (
    echo ❌ 未找到 CMakeLists.txt
    echo 💡 请确保在项目根目录中运行此脚本
    goto end
) else (
    echo ✅ 找到 CMakeLists.txt
)

if not exist src (
    echo ⚠️  未找到 src 目录
) else (
    echo ✅ 找到 src 目录
)

if not exist include (
    echo ⚠️  未找到 include 目录
) else (
    echo ✅ 找到 include 目录
)

echo.
echo 🔧 尝试自动修复构建问题...

REM 清理旧的构建文件
if exist build (
    echo 🧹 清理旧的构建文件...
    rmdir /s /q build
)

REM 创建构建目录
mkdir build
cd build

echo.
echo ⚙️  配置项目...

REM 尝试不同的生成器
echo 🔍 尝试 Visual Studio 生成器...
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if not errorlevel 1 (
    echo ✅ Visual Studio 2022 配置成功
    goto build_project
)

cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
if not errorlevel 1 (
    echo ✅ Visual Studio 2019 配置成功
    goto build_project
)

echo 🔍 尝试 MinGW 生成器...
cmake .. -G "MinGW Makefiles" >nul 2>&1
if not errorlevel 1 (
    echo ✅ MinGW 配置成功
    goto build_project
)

echo 🔍 尝试默认生成器...
cmake .. >nul 2>&1
if not errorlevel 1 (
    echo ✅ 默认生成器配置成功
    goto build_project
)

echo ❌ 所有配置尝试都失败了
echo 💡 请检查上面的错误信息并安装必要的工具
goto end

:build_project
echo.
echo 🔧 构建项目...
cmake --build . --config Release
if errorlevel 1 (
    echo ❌ 构建失败
    echo.
    echo 💡 常见解决方案:
    echo    1. 确保项目路径不包含中文字符
    echo    2. 以管理员身份运行
    echo    3. 检查防病毒软件是否阻止了编译
    echo    4. 确保有足够的磁盘空间
    goto end
)

echo ✅ 构建成功!
echo.
echo 🎯 查找可执行文件...
if exist Release\*.exe (
    echo ✅ 找到可执行文件:
    dir Release\*.exe /b
) else if exist Debug\*.exe (
    echo ✅ 找到可执行文件:
    dir Debug\*.exe /b
) else (
    echo ⚠️  未找到可执行文件，但构建似乎成功了
)

:end
echo.
echo 📋 故障排除完成
echo.
echo 💡 如果问题仍然存在，请:
echo    1. 复制上面的错误信息
echo    2. 检查是否有杀毒软件干扰
echo    3. 尝试在不同的目录中创建项目
echo    4. 确保Windows系统是最新的
echo.
pause
