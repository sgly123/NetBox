#include "PureRedisProtocol.h"
#include "base/Logger.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/socket.h>

// 构造
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
    return onClientDataReceived(0, data, len);
}

size_t PureRedisProtocol::onClientDataReceived(int clientFd, const char* data, size_t len) {
    if (!data || len == 0) return 0;
    std::string& buf = m_clientBuffers[clientFd];
    buf.append(data, len);

    size_t totalProcessed = 0;
    while (true) {
        size_t consumed = 0;
        auto [ok, args] = resp_decode(buf, consumed);
        if (!ok) break;               // 包不完整
        buf.erase(0, consumed);
        if (!args.empty()) processRedisCommand(clientFd, args);
        totalProcessed += consumed;
    }
    return totalProcessed;
}

bool PureRedisProtocol::pack(const char* data, size_t len, std::vector<char>& packet) {
    packet.assign(data, data + len);
    return true;
}

// -------------- RESP 解码 --------------
std::pair<bool, std::vector<std::string>>
PureRedisProtocol::resp_decode(const std::string& buf, size_t& consumed) {
    consumed = 0;
    if (buf.empty()) return {false, {}};
    if (buf[0] != '*') return {false, {}};          // 只支持数组
    size_t le = buf.find("\r\n", 1);
    if (le == std::string::npos) return {false, {}};
    int64_t count = std::stoll(buf.substr(1, le - 1));
    size_t pos = le + 2;
    std::vector<std::string> out;
    for (int64_t i = 0; i < count; ++i) {
        if (pos + 1 >= buf.size()) return {false, {}};
        if (buf[pos] != '$') return {false, {}};
        le = buf.find("\r\n", pos + 1);
        if (le == std::string::npos) return {false, {}};
        int64_t len = std::stoll(buf.substr(pos + 1, le - pos - 1));
        pos = le + 2;
        if (pos + len + 2 > buf.size()) return {false, {}};
        out.emplace_back(buf.substr(pos, len));
        pos += len + 2;   // 跳过 \r\n
    }
    consumed = pos;
    return {true, out};
}

// -------------- 命令处理 --------------
void PureRedisProtocol::processRedisCommand(int clientFd, const std::vector<std::string>& args) {
    if (args.empty()) return;
    std::string response = executeRedisCommand(args);
    sendDirectResponse(clientFd, response);
}

std::string PureRedisProtocol::executeRedisCommand(const std::vector<std::string>& args) {
    std::string cmd = args[0];
    Logger::info("executeRedisCommand 首参数: '" + args[0] + "'");
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);


    if (cmd == "COMMAND") {
    return formatArray({}); 
    }
    if (cmd == "PING") {
        if (args.size() == 1) return formatSimpleString("PONG");
        if (args.size() == 2) return formatBulkString(args[1]);
        return formatError("ERR wrong number of arguments for 'ping' command");
    }
    if (cmd == "SET" && args.size() == 3) {
        m_stringData[args[1]] = args[2];
        return formatSimpleString("OK");
    }
    if (cmd == "GET" && args.size() == 2) {
        auto it = m_stringData.find(args[1]);
        if (it == m_stringData.end()) return formatNull();
        return formatBulkString(it->second);
    }
    if (cmd == "DEL" && args.size() >= 2) {
        int deleted = 0;
        for (size_t i = 1; i < args.size(); ++i) deleted += m_stringData.erase(args[i]);
        return formatInteger(deleted);
    }
    if (cmd == "KEYS" && args.size() == 2) {
        std::vector<std::string> keys;
        for (auto& p : m_stringData) keys.push_back(p.first);
        return formatArray(keys);
    }
    return formatError("ERR unknown command '" + cmd + "'");
}

// -------------- RESP 格式化 --------------
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
    std::string s = "*" + std::to_string(array.size()) + "\r\n";
    for (auto& item : array) s += formatBulkString(item);
    return s;
}
std::string PureRedisProtocol::formatNull() {
    return "$-1\r\n";
}

// -------------- 发送 --------------
void PureRedisProtocol::sendDirectResponse(int clientFd, const std::string& response) {
    /*
    if (packetCallback_) {
        std::vector<char> pkt(response.begin(), response.end());
        packetCallback_(pkt);
    }
    */

    // ✅ 新增：校验RESP响应首字符是否合法
    if (!response.empty()) {
        char first = response[0];
        if (first != '+' && first != '-' && first != ':' && first != '$' && first != '*') {
            Logger::error("非法RESP响应首字符: 0x" + 
                std::to_string(static_cast<unsigned char>(first)));
            return; // 阻止发送非法响应
        }
    }
    
    std::lock_guard<std::mutex> lock(m_sendMutex);  // 加锁
    std::ostringstream hexStream;
    for (unsigned char c : response) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    Logger::debug("响应内容十六进制: " + hexStream.str());

    Logger::debug("响应内容: " + response);
    if (clientFd > 0) {
        ssize_t sent = ::send(clientFd, response.c_str(), response.length(), 0);
        if (sent > 0) {
            Logger::info("PureRedisProtocol 直接发送成功，长度: " + std::to_string(sent));
        } else {
            Logger::error("PureRedisProtocol 发送失败，错误码: " + std::to_string(errno));
        }
    } else {
        Logger::error("无效的客户端FD，无法发送响应");
    }
}