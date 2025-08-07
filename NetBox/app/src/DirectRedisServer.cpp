#include "DirectRedisServer.h"
#include "base/Logger.h"
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <iomanip>

DirectRedisServer::DirectRedisServer(const std::string& ip, int port,
                                   IOMultiplexer::IOType io_type, IThreadPool* pool)
    : ApplicationServer(ip, port, io_type, pool) {
    Logger::info("DirectRedisServer 初始化完成");
    Logger::info("监听地址: " + ip + ":" + std::to_string(port));
    Logger::info("支持命令: PING, SET, GET, DEL, KEYS, LPUSH, LPOP, LRANGE, HSET, HGET, HKEYS");
}

void DirectRedisServer::initializeProtocolRouter() {
    // DirectRedis不使用协议栈，直接处理原始TCP数据
    Logger::info("DirectRedis跳过协议路由器初始化（直接处理TCP数据）");
}

std::string DirectRedisServer::handleHttpRequest(const std::string& request, int clientFd) {
    // DirectRedis不支持HTTP
    (void)request;
    (void)clientFd;
    return "HTTP/1.1 400 Bad Request\r\n\r\nDirectRedis server does not support HTTP";
}

std::string DirectRedisServer::handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) {
    // DirectRedis直接处理，不通过这个接口
    (void)command;
    return executeRedisCommand(args);
}

bool DirectRedisServer::parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) {
    // DirectRedis不使用这个接口
    (void)path;
    (void)command;
    (void)args;
    return false;
}

void DirectRedisServer::onDataReceived(int clientFd, const char* data, size_t len) {
    Logger::info("DirectRedis收到客户端" + std::to_string(clientFd) + "的数据，长度: " + std::to_string(len));

    // 调试：打印原始数据的十六进制
    std::ostringstream hexStream;
    hexStream << "原始数据十六进制: ";
    for (size_t i = 0; i < len && i < 50; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)data[i] << " ";
    }
    Logger::debug(hexStream.str());

    // 调试：打印原始数据的可打印字符
    std::ostringstream charStream;
    charStream << "原始数据字符: ";
    for (size_t i = 0; i < len; ++i) {
        char c = data[i];
        if (c >= 32 && c <= 126) {
            charStream << c;
        } else if (c == '\r') {
            charStream << "\\r";
        } else if (c == '\n') {
            charStream << "\\n";
        } else {
            charStream << "[" << (int)(unsigned char)c << "]";
        }
    }
    Logger::debug(charStream.str());

    // 不调用基类方法，直接处理数据
    // ApplicationServer::onDataReceived(clientFd, data, len);

    // 将数据添加到客户端缓冲区
    std::string& buffer = m_clientBuffers[clientFd];
    buffer.append(data, len);

    // 处理完整的命令行（以\n结尾）
    size_t pos = 0;
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string commandLine = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        // 移除\r（如果存在）
        if (!commandLine.empty() && commandLine.back() == '\r') {
            commandLine.pop_back();
        }

        Logger::info("处理命令行: " + commandLine);
        processCommandLine(clientFd, commandLine);
    }
}

void DirectRedisServer::onClientConnected(int clientFd) {
    Logger::info("DirectRedis客户端" + std::to_string(clientFd) + "连接成功");
    m_clientBuffers[clientFd] = "";  // 初始化缓冲区
}

void DirectRedisServer::onClientDisconnected(int clientFd) {
    Logger::info("DirectRedis客户端" + std::to_string(clientFd) + "断开连接");
    m_clientBuffers.erase(clientFd);  // 清理缓冲区
}

void DirectRedisServer::processCommandLine(int clientFd, const std::string& commandLine) {
    if (commandLine.empty()) {
        Logger::warn("收到空命令行");
        sendRedisResponse(clientFd, formatError("ERR empty command"));
        return;
    }
    
    // 解析Redis命令
    auto args = parseRedisCommand(commandLine);
    if (args.empty()) {
        Logger::warn("命令解析失败: " + commandLine);
        sendRedisResponse(clientFd, formatError("ERR invalid command format"));
        return;
    }
    
    Logger::info("解析出 " + std::to_string(args.size()) + " 个参数");
    for (size_t i = 0; i < args.size(); ++i) {
        Logger::debug("参数[" + std::to_string(i) + "]: '" + args[i] + "'");
    }
    
    // 执行Redis命令
    std::string result = executeRedisCommand(args);
    Logger::info("命令执行结果: " + result.substr(0, 50) + (result.length() > 50 ? "..." : ""));
    
    // 发送响应
    sendRedisResponse(clientFd, result);
}

std::vector<std::string> DirectRedisServer::parseRedisCommand(const std::string& command) {
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;
    
    // 简单的空格分割解析
    while (iss >> arg) {
        // 移除引号（如果有）
        if (arg.length() >= 2 && arg.front() == '"' && arg.back() == '"') {
            arg = arg.substr(1, arg.length() - 2);
        }
        args.push_back(arg);
    }
    
    return args;
}

std::string DirectRedisServer::executeRedisCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return formatError("ERR empty command");
    }
    
    // 转换命令为大写
    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    Logger::info("执行命令: " + cmd);
    
    // 路由到具体的命令处理函数
    if (cmd == "PING") {
        return cmdPing(args);
    } else if (cmd == "SET") {
        return cmdSet(args);
    } else if (cmd == "GET") {
        return cmdGet(args);
    } else if (cmd == "DEL") {
        return cmdDel(args);
    } else if (cmd == "KEYS") {
        return cmdKeys(args);
    } else if (cmd == "LPUSH") {
        return cmdLPush(args);
    } else if (cmd == "LPOP") {
        return cmdLPop(args);
    } else if (cmd == "LRANGE") {
        return cmdLRange(args);
    } else if (cmd == "HSET") {
        return cmdHSet(args);
    } else if (cmd == "HGET") {
        return cmdHGet(args);
    } else if (cmd == "HKEYS") {
        return cmdHKeys(args);
    } else {
        return formatError("ERR unknown command '" + cmd + "'");
    }
}

void DirectRedisServer::sendRedisResponse(int clientFd, const std::string& response) {
    Logger::info("发送Redis响应到客户端" + std::to_string(clientFd) + ": " + 
                response.substr(0, 50) + (response.length() > 50 ? "..." : ""));
    
    // 调试：显示响应的十六进制
    std::ostringstream hexStream;
    hexStream << "响应十六进制: ";
    for (size_t i = 0; i < response.length() && i < 50; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)response[i] << " ";
    }
    Logger::debug(hexStream.str());
    
    // 直接发送RESP格式的响应
    ssize_t sent = send(clientFd, response.c_str(), response.length(), 0);
    if (sent > 0) {
        Logger::info("Redis响应已发送，发送长度: " + std::to_string(sent));
    } else {
        Logger::error("发送Redis响应失败，错误码: " + std::to_string(errno));
    }
}

// ==================== Redis命令实现 ====================

std::string DirectRedisServer::cmdPing(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return formatSimpleString("PONG");
    } else if (args.size() == 2) {
        return formatBulkString(args[1]);
    } else {
        return formatError("ERR wrong number of arguments for 'ping' command");
    }
}

std::string DirectRedisServer::cmdSet(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'set' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    const std::string& value = args[2];

    // 删除其他类型的数据（如果存在）
    m_listData.erase(key);
    m_hashData.erase(key);

    // 设置字符串值
    m_stringData[key] = value;

    Logger::info("SET " + key + " = " + value);
    return formatSimpleString("OK");
}

std::string DirectRedisServer::cmdGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'get' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    auto it = m_stringData.find(key);
    if (it != m_stringData.end()) {
        return formatBulkString(it->second);
    }

    return formatNull();
}

std::string DirectRedisServer::cmdDel(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return formatError("ERR wrong number of arguments for 'del' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    int deletedCount = 0;
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& key = args[i];
        if (m_stringData.erase(key) > 0) {
            deletedCount++;
        }
        if (m_listData.erase(key) > 0) {
            deletedCount++;
        }
        if (m_hashData.erase(key) > 0) {
            deletedCount++;
        }
    }

    return formatInteger(deletedCount);
}

std::string DirectRedisServer::cmdKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'keys' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    std::vector<std::string> keys;

    // 收集所有键
    for (const auto& pair : m_stringData) {
        keys.push_back(pair.first);
    }
    for (const auto& pair : m_listData) {
        keys.push_back(pair.first);
    }
    for (const auto& pair : m_hashData) {
        keys.push_back(pair.first);
    }

    // 简单的通配符匹配（只支持*）
    const std::string& pattern = args[1];
    if (pattern != "*") {
        std::vector<std::string> filteredKeys;
        for (const auto& key : keys) {
            if (key.find(pattern) != std::string::npos) {
                filteredKeys.push_back(key);
            }
        }
        keys = filteredKeys;
    }

    std::sort(keys.begin(), keys.end());
    return formatArray(keys);
}

std::string DirectRedisServer::cmdLPush(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'lpush' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];

    // 删除字符串和哈希类型数据（如果存在）
    m_stringData.erase(key);
    m_hashData.erase(key);

    // 从左边插入所有值
    auto& list = m_listData[key];
    for (size_t i = 2; i < args.size(); ++i) {
        list.push_front(args[i]);
    }

    Logger::info("LPUSH " + key + " (size: " + std::to_string(list.size()) + ")");
    return formatInteger(static_cast<int>(list.size()));
}

std::string DirectRedisServer::cmdLPop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'lpop' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    auto it = m_listData.find(key);
    if (it == m_listData.end() || it->second.empty()) {
        return formatNull();
    }

    std::string value = it->second.front();
    it->second.pop_front();

    if (it->second.empty()) {
        m_listData.erase(it);
    }

    return formatBulkString(value);
}

std::string DirectRedisServer::cmdLRange(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return formatError("ERR wrong number of arguments for 'lrange' command");
    }

    int start, stop;
    try {
        start = std::stoi(args[2]);
        stop = std::stoi(args[3]);
    } catch (const std::exception&) {
        return formatError("ERR value is not an integer or out of range");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    auto it = m_listData.find(key);
    if (it == m_listData.end()) {
        return formatArray({});
    }

    const auto& list = it->second;
    std::vector<std::string> result;

    int size = static_cast<int>(list.size());
    if (start < 0) start += size;
    if (stop < 0) stop += size;

    if (start < 0) start = 0;
    if (stop >= size) stop = size - 1;

    if (start <= stop) {
        auto listIt = list.begin();
        std::advance(listIt, start);

        for (int i = start; i <= stop && listIt != list.end(); ++i, ++listIt) {
            result.push_back(*listIt);
        }
    }

    return formatArray(result);
}

std::string DirectRedisServer::cmdHSet(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return formatError("ERR wrong number of arguments for 'hset' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    const std::string& field = args[2];
    const std::string& value = args[3];

    // 删除字符串和列表类型数据（如果存在）
    m_stringData.erase(key);
    m_listData.erase(key);

    auto& hash = m_hashData[key];
    bool isNew = hash.find(field) == hash.end();
    hash[field] = value;

    return formatInteger(isNew ? 1 : 0);
}

std::string DirectRedisServer::cmdHGet(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'hget' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    const std::string& field = args[2];

    auto it = m_hashData.find(key);
    if (it == m_hashData.end()) {
        return formatNull();
    }

    auto fieldIt = it->second.find(field);
    if (fieldIt == it->second.end()) {
        return formatNull();
    }

    return formatBulkString(fieldIt->second);
}

std::string DirectRedisServer::cmdHKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'hkeys' command");
    }

    std::lock_guard<std::mutex> lock(m_dataLock);

    const std::string& key = args[1];
    auto it = m_hashData.find(key);
    if (it == m_hashData.end()) {
        return formatArray({});
    }

    std::vector<std::string> keys;
    for (const auto& pair : it->second) {
        keys.push_back(pair.first);
    }

    return formatArray(keys);
}

// ==================== RESP协议格式化 ====================

std::string DirectRedisServer::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string DirectRedisServer::formatBulkString(const std::string& str) {
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string DirectRedisServer::formatArray(const std::vector<std::string>& arr) {
    std::string result = "*" + std::to_string(arr.size()) + "\r\n";
    for (const auto& item : arr) {
        result += formatBulkString(item);
    }
    return result;
}

std::string DirectRedisServer::formatInteger(int num) {
    return ":" + std::to_string(num) + "\r\n";
}

std::string DirectRedisServer::formatError(const std::string& error) {
    return "-" + error + "\r\n";
}

std::string DirectRedisServer::formatNull() {
    return "$-1\r\n";
}
