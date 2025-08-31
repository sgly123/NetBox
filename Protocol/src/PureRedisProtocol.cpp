#include "PureRedisProtocol.h"
#include "base/Logger.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/socket.h>

PureRedisProtocol::PureRedisProtocol() {
    Logger::info("PureRedisProtocol 初始化完成");
}

uint32_t PureRedisProtocol::getProtocolId() const {
    return PURE_REDIS_PROTOCOL_ID;
}

std::string PureRedisProtocol::getType() const {
    return "PureRedis";
}

void PureRedisProtocol::reset() {
    m_clientBuffers.clear();
    Logger::debug("PureRedisProtocol状态已重置");
}

size_t PureRedisProtocol::onDataReceived(const char* data, size_t len) {
    // ProtocolBase接口，不使用clientFd，使用默认客户端ID 0
    return onClientDataReceived(0, data, len);
}

size_t PureRedisProtocol::onClientDataReceived(int clientFd, const char* data, size_t len) {
    Logger::info("PureRedisProtocol收到客户端" + std::to_string(clientFd) + "的数据，长度: " + std::to_string(len));
    
    if (!data || len == 0) {
        return 0;
    }
    
    // 调试：显示原始数据
    std::ostringstream hexStream, charStream;
    hexStream << "Pure Redis原始数据十六进制: ";
    charStream << "Pure Redis原始数据字符: ";
    
    for (size_t i = 0; i < len; ++i) {
        char c = data[i];
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)c << " ";
        
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
    Logger::debug(hexStream.str());
    Logger::debug(charStream.str());
    
    // 将数据添加到客户端缓冲区
    std::string& buffer = m_clientBuffers[clientFd];
    buffer.append(data, len);
    
    size_t totalProcessed = 0;
    
    // 处理完整的Redis命令
    while (true) {
        size_t commandLen = isCompleteRedisCommand(buffer);
        if (commandLen == 0) {
            break;  // 没有完整的命令
        }
        
        std::string commandLine = buffer.substr(0, commandLen);
        buffer.erase(0, commandLen);
        totalProcessed += commandLen;
        
        // 移除\r\n
        while (!commandLine.empty() && (commandLine.back() == '\r' || commandLine.back() == '\n')) {
            commandLine.pop_back();
        }
        
        Logger::info("Pure Redis处理命令: " + commandLine);
        processRedisCommand(clientFd, commandLine);
    }
    
    Logger::debug("PureRedisProtocol处理了 " + std::to_string(totalProcessed) + " 字节");
    return totalProcessed;
}

bool PureRedisProtocol::pack(const char* data, size_t len, std::vector<char>& packet) {
    // 纯RESP协议：不添加任何协议头，直接传输
    packet.clear();
    packet.reserve(len);
    packet.assign(data, data + len);
    
    Logger::debug("PureRedisProtocol直接传输，长度: " + std::to_string(len));
    return true;
}

void PureRedisProtocol::processRedisCommand(int clientFd, const std::string& commandLine) {
    if (commandLine.empty()) {
        Logger::warn("收到空的Redis命令");
        return;
    }
    
    // 解析Redis命令参数
    auto args = parseRedisCommand(commandLine);
    if (args.empty()) {
        Logger::warn("Redis命令解析失败: " + commandLine);
        return;
    }
    
    Logger::info("Pure Redis解析出 " + std::to_string(args.size()) + " 个参数");
    for (size_t i = 0; i < args.size(); ++i) {
        Logger::debug("Pure Redis参数[" + std::to_string(i) + "]: '" + args[i] + "'");
    }
    
    // 执行Redis命令并生成响应
    std::string response = executeRedisCommand(args);
    Logger::info("Pure Redis生成响应: " + response.substr(0, 20) + "...");
    
    // 直接发送RESP响应，不经过协议封装
    sendDirectResponse(clientFd, response);
}

std::vector<std::string> PureRedisProtocol::parseRedisCommand(const std::string& commandLine) {
    std::vector<std::string> args;
    std::istringstream iss(commandLine);
    std::string arg;
    
    // 简单的空格分割解析，保留引号内容
    while (iss >> arg) {
        // 移除引号（如果有），但保留引号内的内容
        if (arg.length() >= 2 && arg.front() == '"' && arg.back() == '"') {
            arg = arg.substr(1, arg.length() - 2);
        }
        args.push_back(arg);
    }
    
    // 调试：打印解析结果
    Logger::debug("PureRedisProtocol命令解析结果:");
    for (size_t i = 0; i < args.size(); ++i) {
        Logger::debug("  参数[" + std::to_string(i) + "]: '" + args[i] + "'");
    }
    
    return args;
}

size_t PureRedisProtocol::isCompleteRedisCommand(const std::string& buffer) {
    // 查找完整的命令行（以\n结尾）
    size_t pos = buffer.find('\n');
    if (pos != std::string::npos) {
        return pos + 1;  // 包含\n的长度
    }
    return 0;  // 没有完整的命令
}

std::string PureRedisProtocol::executeRedisCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return formatError("ERR empty command");
    }
    
    // 转换命令为大写
    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    Logger::info("PureRedisProtocol执行命令: " + cmd + " (参数数量: " + std::to_string(args.size()) + ")");
    
    // 调试：打印所有参数
    for (size_t i = 0; i < args.size(); ++i) {
        Logger::debug("  参数[" + std::to_string(i) + "]: '" + args[i] + "'");
    }
    
    // Redis命令处理
    if (cmd == "PING") {
        if (args.size() == 1) {
            return formatSimpleString("PONG");
        } else if (args.size() == 2) {
            return formatBulkString(args[1]);
        } else {
            return formatError("ERR wrong number of arguments for 'ping' command");
        }
    } else if (cmd == "SET" && args.size() == 3) {
        Logger::info("PureRedisProtocol执行SET命令: key='" + args[1] + "', value='" + args[2] + "'");
        m_stringData[args[1]] = args[2];
        std::string response = formatSimpleString("OK");
        Logger::info("PureRedisProtocol SET命令响应: " + response);
        return response;
    } else if (cmd == "GET" && args.size() == 2) {
        Logger::info("PureRedisProtocol执行GET命令: key='" + args[1] + "'");
        auto it = m_stringData.find(args[1]);
        if (it != m_stringData.end()) {
            std::string response = formatBulkString(it->second);
            Logger::info("PureRedisProtocol GET命令响应: " + response);
            return response;
        } else {
            std::string response = formatNull();
            Logger::info("PureRedisProtocol GET命令响应: " + response);
            return response;
        }
    } else if (cmd == "DEL" && args.size() >= 2) {
        int deleted = 0;
        for (size_t i = 1; i < args.size(); ++i) {
            if (m_stringData.erase(args[i]) > 0) {
                deleted++;
            }
        }
        return formatInteger(deleted);
    } else if (cmd == "KEYS" && args.size() == 2) {
        std::vector<std::string> keys;
        for (const auto& pair : m_stringData) {
            keys.push_back(pair.first);
        }
        return formatArray(keys);
    } else {
        Logger::warn("PureRedisProtocol未知命令: " + cmd + " (参数数量: " + std::to_string(args.size()) + ")");
        return formatError("ERR unknown command '" + cmd + "'");
    }
}

void PureRedisProtocol::sendDirectResponse(int clientFd, const std::string& response) {
    Logger::info("PureRedisProtocol通过回调发送响应到客户端" + std::to_string(clientFd));
    Logger::debug("响应内容: " + response);

    // 通过packetCallback_发送响应，让上层处理socket发送
    if (packetCallback_) {
        Logger::info("PureRedisProtocol准备调用packetCallback_");
        std::vector<char> responsePacket(response.begin(), response.end());

        // 调试：显示即将发送的数据
        std::ostringstream hexStream;
        hexStream << "即将通过回调发送的数据十六进制: ";
        for (char c : responsePacket) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)c << " ";
        }
        Logger::debug(hexStream.str());

        Logger::info("PureRedisProtocol正在调用packetCallback_...");
        try {
            packetCallback_(responsePacket);
            Logger::info("PureRedisProtocol成功调用packetCallback_");
        } catch (const std::exception& e) {
            Logger::error("PureRedisProtocol调用packetCallback_时发生异常: " + std::string(e.what()));
        } catch (...) {
            Logger::error("PureRedisProtocol调用packetCallback_时发生未知异常");
        }
        Logger::info("Pure Redis响应已通过回调发送，长度: " + std::to_string(response.length()));
    } else {
        Logger::error("Pure Redis回调函数未设置，无法发送响应");
    }
}

// RESP格式化方法
std::string PureRedisProtocol::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string PureRedisProtocol::formatError(const std::string& error) {
    return "-" + error + "\r\n";
}

std::string PureRedisProtocol::formatInteger(int64_t value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string PureRedisProtocol::formatBulkString(const std::string& str) {
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string PureRedisProtocol::formatArray(const std::vector<std::string>& array) {
    std::string result = "*" + std::to_string(array.size()) + "\r\n";
    for (const auto& item : array) {
        result += formatBulkString(item);
    }
    return result;
}

std::string PureRedisProtocol::formatNull() {
    return "$-1\r\n";
}
