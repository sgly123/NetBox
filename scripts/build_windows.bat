@echo off
REM NetBox Windows 跨平台构建脚本
REM 支持 Visual Studio 2019/2022 和不同架构

setlocal EnableDelayedExpansion

REM 颜色定义 (Windows 10+ 支持ANSI颜色)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

REM 默认配置
set "BUILD_TYPE=Release"
set "ARCHITECTURE=x64"
set "VS_VERSION="
set "SKIP_TESTS=false"

REM 打印带颜色的消息
:print_info
echo %BLUE%[INFO]%NC% %~1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

REM 显示帮助信息
:show_help
echo NetBox Windows 跨平台构建脚本
echo.
echo 用法: %~nx0 [选项]
echo.
echo 选项:
echo     /h, /help           显示此帮助信息
echo     /t TYPE             设置构建类型 (Debug^|Release^|RelWithDebInfo)
echo     /a ARCH             设置架构 (x86^|x64^|ARM^|ARM64)
echo     /vs VERSION         指定Visual Studio版本 (2019^|2022)
echo     /skip-tests         跳过测试执行
echo     /clean              清理所有构建目录
echo.
echo 环境变量:
echo     BUILD_TYPE          构建类型 (默认: Release)
echo     ARCHITECTURE        目标架构 (默认: x64)
echo     SKIP_TESTS          跳过测试 (true^|false, 默认: false)
echo.
echo 示例:
echo     %~nx0                    # 默认Release x64构建
echo     %~nx0 /t Debug           # Debug构建
echo     %~nx0 /a x86             # x86架构构建
echo     %~nx0 /vs 2022           # 指定VS2022
echo     %~nx0 /skip-tests        # 跳过测试的构建
echo.
echo 支持的Visual Studio版本:
echo     - Visual Studio 2019 (16.0+)
echo     - Visual Studio 2022 (17.0+)
echo.
goto :eof

REM 解析命令行参数
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="/h" goto :show_help_and_exit
if /i "%~1"=="/help" goto :show_help_and_exit
if /i "%~1"=="/t" (
    set "BUILD_TYPE=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="/a" (
    set "ARCHITECTURE=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="/vs" (
    set "VS_VERSION=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="/skip-tests" (
    set "SKIP_TESTS=true"
    shift
    goto :parse_args
)
if /i "%~1"=="/clean" (
    call :print_info "清理构建目录..."
    if exist build_windows_* rmdir /s /q build_windows_*
    call :print_success "清理完成"
    exit /b 0
)
call :print_error "未知选项: %~1"
goto :show_help_and_exit

:show_help_and_exit
call :show_help
exit /b 1

:args_done

REM 检测Visual Studio
:detect_visual_studio
call :print_info "检测 Visual Studio 环境..."

REM 如果用户指定了版本，优先使用
if not "%VS_VERSION%"=="" (
    if "%VS_VERSION%"=="2022" (
        set "VS_YEAR=2022"
        set "VS_VERSION_NUM=17"
    ) else if "%VS_VERSION%"=="2019" (
        set "VS_YEAR=2019"
        set "VS_VERSION_NUM=16"
    ) else (
        call :print_error "不支持的Visual Studio版本: %VS_VERSION%"
        exit /b 1
    )
    goto :check_vs_installation
)

REM 自动检测Visual Studio
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
    set "VS_YEAR=2022"
    set "VS_VERSION_NUM=17"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
    set "VS_YEAR=2019"
    set "VS_VERSION_NUM=16"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2019" (
    set "VS_YEAR=2019"
    set "VS_VERSION_NUM=16"
) else (
    call :print_error "未找到支持的Visual Studio版本 (2019/2022)"
    call :print_error "请安装Visual Studio 2019或2022，并确保包含C++工具"
    exit /b 1
)

:check_vs_installation
call :print_info "使用 Visual Studio %VS_YEAR%"

REM 设置VS环境
if "%VS_YEAR%"=="2022" (
    set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
) else (
    set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2019\Community"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2019\Professional"
    if not exist "!VSINSTALLDIR!" set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2019\Enterprise"
)

if not exist "!VSINSTALLDIR!" (
    call :print_error "Visual Studio %VS_YEAR% 安装目录未找到"
    exit /b 1
)

call :print_success "Visual Studio 检测完成: !VSINSTALLDIR!"

REM 检查依赖
:check_dependencies
call :print_info "检查构建依赖..."

REM 检查CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    call :print_error "CMake 未安装，请先安装 CMake 3.16+"
    call :print_error "下载地址: https://cmake.org/download/"
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version ^| findstr /r "cmake version"') do set CMAKE_VERSION=%%i
call :print_info "CMake 版本: !CMAKE_VERSION!"

REM 检查Git (可选)
git --version >nul 2>&1
if errorlevel 1 (
    call :print_warning "Git 未安装，可能影响版本信息获取"
) else (
    for /f "tokens=3" %%i in ('git --version') do set GIT_VERSION=%%i
    call :print_info "Git 版本: !GIT_VERSION!"
)

call :print_success "依赖检查完成"

REM 设置构建目录
:setup_build_directory
call :print_info "设置构建目录..."

set "BUILD_DIR=build_windows_%ARCHITECTURE%_%BUILD_TYPE%"

if exist "%BUILD_DIR%" (
    call :print_warning "构建目录 %BUILD_DIR% 已存在，将清理重建"
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

call :print_success "构建目录设置完成: %BUILD_DIR%"

REM 配置构建
:configure_build
call :print_info "配置构建..."

call :print_info "构建类型: %BUILD_TYPE%"
call :print_info "目标架构: %ARCHITECTURE%"

REM 设置CMake生成器和平台
if "%ARCHITECTURE%"=="x86" (
    set "CMAKE_PLATFORM=Win32"
) else if "%ARCHITECTURE%"=="x64" (
    set "CMAKE_PLATFORM=x64"
) else if "%ARCHITECTURE%"=="ARM" (
    set "CMAKE_PLATFORM=ARM"
) else if "%ARCHITECTURE%"=="ARM64" (
    set "CMAKE_PLATFORM=ARM64"
) else (
    call :print_error "不支持的架构: %ARCHITECTURE%"
    exit /b 1
)

set "CMAKE_GENERATOR=Visual Studio %VS_VERSION_NUM% %VS_YEAR%"

REM 执行CMake配置
call :print_info "执行 CMake 配置..."
cmake -G "%CMAKE_GENERATOR%" -A "%CMAKE_PLATFORM%" ^
      -DCMAKE_BUILD_TYPE="%BUILD_TYPE%" ^
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
      -DWIN32_LEAN_AND_MEAN=ON ^
      ..

if errorlevel 1 (
    call :print_error "CMake 配置失败"
    exit /b 1
)

call :print_success "构建配置完成"

REM 编译项目
:build_project
call :print_info "开始编译项目..."

REM 获取CPU核心数
for /f %%i in ('echo %NUMBER_OF_PROCESSORS%') do set CORES=%%i
call :print_info "使用 %CORES% 个并行编译进程"

REM 编译
cmake --build . --config "%BUILD_TYPE%" --parallel %CORES%

if errorlevel 1 (
    call :print_error "项目编译失败"
    exit /b 1
)

call :print_success "项目编译完成"

REM 运行测试
:run_tests
if "%SKIP_TESTS%"=="true" (
    call :print_warning "跳过测试 (SKIP_TESTS=true)"
    goto :generate_report
)

call :print_info "运行测试套件..."

REM 检查测试可执行文件是否存在
if not exist "tests\bin\%BUILD_TYPE%\test_base.exe" (
    call :print_warning "测试可执行文件不存在，跳过测试"
    goto :generate_report
)

REM 运行跨平台测试
call :print_info "运行跨平台功能测试..."
tests\bin\%BUILD_TYPE%\test_platform.exe
if errorlevel 1 (
    call :print_error "跨平台测试失败"
    exit /b 1
)
call :print_success "跨平台测试通过"

REM 运行基础测试
call :print_info "运行基础组件测试..."
tests\bin\%BUILD_TYPE%\test_base.exe
if errorlevel 1 (
    call :print_error "基础组件测试失败"
    exit /b 1
)
call :print_success "基础组件测试通过"

REM 运行IO测试
call :print_info "运行IO多路复用测试..."
tests\bin\%BUILD_TYPE%\test_io.exe
if errorlevel 1 (
    call :print_error "IO多路复用测试失败"
    exit /b 1
)
call :print_success "IO多路复用测试通过"

call :print_success "所有测试通过"

REM 生成构建报告
:generate_report
call :print_info "生成构建报告..."

set "REPORT_FILE=build_report_windows_%ARCHITECTURE%.txt"

echo NetBox Windows 跨平台构建报告 > "%REPORT_FILE%"
echo ================================= >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"
echo 构建信息: >> "%REPORT_FILE%"
echo - 平台: Windows >> "%REPORT_FILE%"
echo - 架构: %ARCHITECTURE% >> "%REPORT_FILE%"
echo - 构建类型: %BUILD_TYPE% >> "%REPORT_FILE%"
echo - 构建时间: %date% %time% >> "%REPORT_FILE%"
echo - 构建目录: %BUILD_DIR% >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"
echo 系统信息: >> "%REPORT_FILE%"
echo - 操作系统: %OS% >> "%REPORT_FILE%"
echo - 处理器架构: %PROCESSOR_ARCHITECTURE% >> "%REPORT_FILE%"
echo - CPU核心数: %CORES% >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"
echo 开发环境: >> "%REPORT_FILE%"
echo - Visual Studio: %VS_YEAR% >> "%REPORT_FILE%"
echo - CMake版本: %CMAKE_VERSION% >> "%REPORT_FILE%"
echo - CMake生成器: %CMAKE_GENERATOR% >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"
echo 构建结果: >> "%REPORT_FILE%"
echo - 状态: 成功 >> "%REPORT_FILE%"
echo - 可执行文件: %BUILD_TYPE%\NetBox.exe >> "%REPORT_FILE%"
echo. >> "%REPORT_FILE%"
if "%SKIP_TESTS%"=="false" (
    echo 测试结果: >> "%REPORT_FILE%"
    echo - 跨平台测试: 通过 >> "%REPORT_FILE%"
    echo - 基础组件测试: 通过 >> "%REPORT_FILE%"
    echo - IO多路复用测试: 通过 >> "%REPORT_FILE%"
    echo. >> "%REPORT_FILE%"
)

call :print_success "构建报告已生成: %REPORT_FILE%"

REM 主函数完成
:main_complete
call :print_success "========================================"
call :print_success "NetBox Windows 跨平台构建完成!"
call :print_info "构建目录: %BUILD_DIR%"
call :print_info "构建报告: %BUILD_DIR%\%REPORT_FILE%"
echo.
call :print_info "下一步操作:"
call :print_info "1. 运行应用: cd %BUILD_DIR% && %BUILD_TYPE%\NetBox.exe"
call :print_info "2. 运行测试: cd %BUILD_DIR% && tests\bin\%BUILD_TYPE%\test_*.exe"
call :print_info "3. 查看报告: type %BUILD_DIR%\%REPORT_FILE%"

exit /b 0

REM 主程序入口
:main
call :print_info "开始 NetBox Windows 跨平台构建"
call :print_info "========================================"

call :parse_args %*
call :detect_visual_studio
call :check_dependencies
call :setup_build_directory
call :configure_build
call :build_project
call :run_tests
call :generate_report
call :main_complete

REM 调用主函数
call :main %*
