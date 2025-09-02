@echo off
echo.
echo 🧪 NetBox 简化测试脚本
echo ======================
echo.

REM 启动主服务
echo ℹ️  启动NetBox服务器...
docker-compose up -d netbox

REM 等待服务启动
echo ℹ️  等待服务启动...
timeout /t 10 /nobreak >nul

REM 测试Redis PING
echo.
echo 🔍 测试Redis PING命令...
docker run --rm --network netbox-network redis:7-alpine redis-cli -h netbox -p 6379 ping

REM 测试Echo服务器
echo.
echo 🔍 测试Echo服务器...
echo "Hello NetBox!" | docker run --rm --network netbox-network alpine:latest nc netbox 8888

echo.
echo 🎉 测试完成！
echo.
pause 