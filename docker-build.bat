@echo off
setlocal enabledelayedexpansion

REM NetBox Docker构建脚本 (Windows版本)
REM 用于构建和部署NetBox Docker镜像

echo.
echo 🐳 NetBox Docker 构建脚本 (Windows)
echo ========================================
echo.

REM 检查Docker是否安装
docker --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Docker未安装，请先安装Docker Desktop
    pause
    exit /b 1
)

REM 检查Docker服务是否运行
docker info >nul 2>&1
if errorlevel 1 (
    echo ❌ Docker服务未运行，请启动Docker Desktop
    pause
    exit /b 1
)

echo ✅ Docker环境检查通过

REM 检查Docker Compose是否安装
docker-compose --version >nul 2>&1
if errorlevel 1 (
    echo ⚠️  Docker Compose未安装，将使用docker build命令
    set USE_COMPOSE=false
) else (
    echo ✅ Docker Compose环境检查通过
    set USE_COMPOSE=true
)

REM 解析命令行参数
set COMMAND=%1
if "%COMMAND%"=="" set COMMAND=help

REM 执行相应命令
if "%COMMAND%"=="build" goto :build
if "%COMMAND%"=="run" goto :run
if "%COMMAND%"=="dev" goto :dev
if "%COMMAND%"=="shell" goto :shell
if "%COMMAND%"=="stop" goto :stop
if "%COMMAND%"=="status" goto :status
if "%COMMAND%"=="test" goto :test
if "%COMMAND%"=="cleanup" goto :cleanup
if "%COMMAND%"=="help" goto :help
goto :help

:build
echo.
echo ℹ️  开始构建Docker镜像...
if "%USE_COMPOSE%"=="true" (
    echo ℹ️  使用Docker Compose构建...
    docker-compose build
) else (
    echo ℹ️  使用Docker build构建...
    docker build -t netbox:latest .
    if errorlevel 1 (
        echo ⚠️  标准构建失败，尝试使用简化版Dockerfile...
        docker build -f Dockerfile.simple -t netbox:latest .
        if errorlevel 1 (
            echo ❌ 构建失败
            pause
            exit /b 1
        )
    )
)
echo ✅ Docker镜像构建完成
goto :end

:run
echo.
echo ℹ️  构建并运行NetBox容器...
call :build
if errorlevel 1 goto :end

echo ℹ️  启动NetBox容器...
if "%USE_COMPOSE%"=="true" (
    echo ℹ️  使用Docker Compose启动...
    docker-compose up -d
) else (
    echo ℹ️  使用Docker run启动...
    docker run -d --name netbox-server --restart unless-stopped -p 6379:6379 -p 8888:8888 -v %cd%/config:/app/config:ro -v %cd%/logs:/app/logs -v %cd%/data:/app/data netbox:latest
)
if errorlevel 1 (
    echo ❌ 启动失败
    pause
    exit /b 1
)
echo ✅ NetBox容器启动完成
goto :end

:dev
echo.
echo 🔧 启动NetBox开发模式...
echo ℹ️  开发模式特点: 源码实时挂载，支持实时修改
echo.
if "%USE_COMPOSE%"=="true" (
    docker-compose --profile dev up -d
) else (
    echo ❌ 开发模式需要Docker Compose支持
    echo ℹ️  请安装Docker Compose或使用docker-compose.yml
    pause
    exit /b 1
)
if errorlevel 1 (
    echo ❌ 开发模式启动失败
    pause
    exit /b 1
)
echo.
echo ✅ NetBox开发模式启动完成
echo.
echo 📋 开发模式信息:
echo   容器名称: netbox-dev-server
echo   Echo端口: localhost:8888
echo   Redis端口: localhost:6379
echo   调试端口: localhost:9999
echo.
echo 🛠️  进入开发容器:
echo   %0 shell
echo.
echo 📝 开发流程:
echo   1. %0 shell           # 进入容器
echo   2. cd /workspace      # 进入源码目录
echo   3. mkdir -p build ^&^& cd build  # 创建构建目录
echo   4. cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=OFF
echo   5. make -j$(nproc)    # 编译
echo   6. ./NetBox ../config/config-echo.yaml  # 运行
echo.
goto :end

:shell
echo.
echo 🐚 进入NetBox开发容器...
docker exec -it netbox-dev-server bash 2>nul
if errorlevel 1 (
    echo ❌ 无法进入容器，请检查开发模式是否运行:
    echo   %0 dev
    pause
    exit /b 1
)
goto :end

:test
echo.
echo 🧪 运行双服务测试...
if exist "test-dual-services.bat" (
    call test-dual-services.bat
) else (
    echo ❌ 测试脚本 test-dual-services.bat 不存在
    echo ℹ️  请确保测试脚本在当前目录
    pause
    exit /b 1
)
goto :end

:stop
echo.
echo ℹ️  停止NetBox容器...
if "%USE_COMPOSE%"=="true" (
    docker-compose down
) else (
    docker stop netbox-server 2>nul
    docker rm netbox-server 2>nul
)
echo ✅ NetBox容器已停止
goto :end

:status
echo.
echo ℹ️  容器状态:
if "%USE_COMPOSE%"=="true" (
    docker-compose ps
) else (
    docker ps -a --filter "name=netbox"
)

echo.
echo ℹ️  容器日志:
if "%USE_COMPOSE%"=="true" (
    docker-compose logs --tail=20
) else (
    docker logs --tail=20 netbox-server 2>nul || echo 容器未运行
)
goto :end

:cleanup
echo.
echo ℹ️  清理Docker资源...
if "%USE_COMPOSE%"=="true" (
    docker-compose down --rmi all --volumes --remove-orphans
) else (
    docker stop netbox-server 2>nul
    docker rm netbox-server 2>nul
    docker rmi netbox:latest 2>nul
)
echo ✅ 清理完成
goto :end

:help
echo.
echo 用法: %0 [命令]
echo.
echo 命令:
echo   build    构建Docker镜像
echo   run      构建并运行双服务 (Echo + Redis)
echo   dev      启动开发模式 (源码实时挂载)
echo   stop     停止所有容器
echo   status   查看容器状态
echo   test     运行双服务测试
echo   shell    进入开发容器 (需要开发模式运行)
echo   cleanup  清理所有资源
echo   help     显示此帮助信息
echo.
echo 🎯 模式说明:
echo   生产模式: 源码编译到镜像，稳定高性能
echo   开发模式: 源码实时挂载，支持实时修改
echo.
echo 示例:
echo   %0 build    # 构建镜像
echo   %0 run      # 构建并运行双服务 (生产模式)
echo   %0 dev      # 启动开发模式
echo   %0 shell    # 进入开发容器
echo   %0 test     # 测试双服务
echo   %0 status   # 查看状态
echo.
echo 注意: 在Windows环境下，请确保Docker Desktop已启动
echo.

:end
if "%COMMAND%"=="help" pause 