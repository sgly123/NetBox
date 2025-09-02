@echo off
echo.
echo 🔧 Docker网络问题修复脚本
echo ============================
echo.

echo ℹ️  检查Docker网络配置...

REM 检查Docker是否运行
docker info >nul 2>&1
if errorlevel 1 (
    echo ❌ Docker服务未运行，请启动Docker Desktop
    pause
    exit /b 1
)

echo ✅ Docker服务正在运行

REM 清理Docker缓存
echo ℹ️  清理Docker缓存...
docker system prune -f

REM 清理构建缓存
echo ℹ️  清理构建缓存...
docker builder prune -f

REM 检查网络连接
echo ℹ️  测试网络连接...
docker run --rm alpine:latest ping -c 3 mirrors.aliyun.com

if errorlevel 1 (
    echo ⚠️  网络连接可能有问题，尝试使用代理...
    echo 请确保您的网络可以访问Docker Hub
) else (
    echo ✅ 网络连接正常
)

echo.
echo 🎯 建议的解决方案:
echo 1. 如果网络连接失败，请检查防火墙设置
echo 2. 如果在公司网络，可能需要配置代理
echo 3. 尝试使用VPN或更换网络环境
echo 4. 使用简化版Dockerfile: docker build -f Dockerfile.simple -t netbox:latest .
echo.

pause 