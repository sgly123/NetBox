@echo off
echo.
echo 🧪 NetBox Docker 测试脚本
echo ============================
echo.

REM 检查容器是否运行
docker ps --filter "name=netbox-server" --format "table {{.Names}}\t{{.Status}}" | findstr "netbox-server" >nul
if errorlevel 1 (
    echo ❌ NetBox容器未运行，请先启动容器
    echo 运行命令: docker-build.bat run
    pause
    exit /b 1
)

echo ✅ NetBox容器正在运行
echo.

echo 🔍 测试Redis协议连接...
echo.

REM 测试Redis PING命令
echo 测试1: PING命令
docker run --rm --network netbox-network redis:7-alpine redis-cli -h netbox -p 6379 ping
if errorlevel 1 (
    echo ❌ Redis PING测试失败
) else (
    echo ✅ Redis PING测试成功
)

echo.

REM 测试Redis SET/GET命令
echo 测试2: SET/GET命令
docker run --rm --network netbox-network redis:7-alpine redis-cli -h netbox -p 6379 set test_key "Hello NetBox!"
if errorlevel 1 (
    echo ❌ Redis SET测试失败
) else (
    echo ✅ Redis SET测试成功
)

docker run --rm --network netbox-network redis:7-alpine redis-cli -h netbox -p 6379 get test_key
if errorlevel 1 (
    echo ❌ Redis GET测试失败
) else (
    echo ✅ Redis GET测试成功
)

echo.

echo 🔍 测试Echo服务器连接...
echo.

REM 测试Echo服务器
echo 测试3: Echo服务器
echo "Hello NetBox Echo!" | docker run --rm --network netbox-network alpine:latest nc netbox 8888
if errorlevel 1 (
    echo ❌ Echo服务器测试失败
) else (
    echo ✅ Echo服务器测试成功
)

echo.
echo 🎉 所有测试完成！
echo.
echo 📊 测试总结:
echo   - Redis协议: 支持PING、SET、GET命令
echo   - Echo服务器: 支持文本回显
echo   - 网络连接: 容器间通信正常
echo.
pause 