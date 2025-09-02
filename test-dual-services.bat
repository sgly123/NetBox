@echo off
echo.
echo 🧪 NetBox 双服务测试脚本
echo ========================
echo.

REM 启动双服务
echo ℹ️  启动NetBox双服务 (Echo + Redis)...
docker-compose up -d

REM 等待服务启动
echo ℹ️  等待服务启动...
timeout /t 15 /nobreak >nul

echo.
echo 📊 检查服务状态...
docker-compose ps

echo.
echo 🔍 测试Echo服务器 (端口8888)...
echo "Hello NetBox Echo!" | docker run --rm --network netbox-network alpine:latest nc netbox-echo-server 8888

echo.
echo 🔍 测试Redis服务器 (端口6379)...
docker run --rm --network netbox-network redis:7-alpine redis-cli -h netbox-redis-server -p 6379 ping

echo.
echo 📝 查看Echo服务器日志 (最后10行)...
docker logs --tail=10 netbox-echo-server

echo.
echo 📝 查看Redis服务器日志 (最后10行)...
docker logs --tail=10 netbox-redis-server

echo.
echo 🎉 双服务测试完成！
echo.
echo 📋 服务信息:
echo   - Echo服务器: localhost:8888
echo   - Redis服务器: localhost:6379
echo.
pause 