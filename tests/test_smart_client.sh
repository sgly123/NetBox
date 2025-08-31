#!/bin/bash

echo "🧪 智能NetBox Redis客户端测试"
echo "================================"

CLIENT_PATH="build/bin/smart_netbox_redis_client"

# 测试基本命令
echo "1. 测试基本Redis命令..."
echo -e "ping\nset name \"hello world\"\nget name\nset age 25\nget age\ndel name\nget name\nquit" | $CLIENT_PATH

echo -e "\n2. 测试中文支持..."
echo -e "set 中文 \"你好世界\"\nget 中文\nquit" | $CLIENT_PATH

echo -e "\n3. 测试列表操作..."
echo -e "lpush mylist \"item1\"\nlpush mylist \"item2\"\nlrange mylist 0 -1\nquit" | $CLIENT_PATH

echo -e "\n4. 测试哈希操作..."
echo -e "hset user name \"张三\"\nhset user age 30\nhget user name\nhkeys user\nquit" | $CLIENT_PATH

echo -e "\n5. 测试错误处理..."
echo -e "get nonexistent\nset name\nquit" | $CLIENT_PATH

echo -e "\n✅ 测试完成！" 