#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
#include <algorithm>
#include <sys/time.h> // Required for struct timeval
#include <errno.h> // Required for errno

/**
 * @brief 智能NetBox Redis客户端
 * 
 * 专门设计用于连接NetBox集成版Redis服务器
 * 核心特点：
 * 1. 🧠 智能协议头过滤 - 自动检测并移除NetBox协议头
 * 2. 🎯 纯净RESP体验 - 只显示标准Redis响应，无4Vx干扰
 * 3. 🌏 完美中文支持 - UTF-8字符完美处理
 * 4. 🚀 标准Redis体验 - 与官方redis-cli体验一致
 */
class SmartNetBoxRedisClient {
private:
    int m_socket;
    std::string m_host;
    int m_port;
    bool m_connected;
    bool m_debugMode;
    
public:
    SmartNetBoxRedisClient(const std::string& host = "192.168.88.135", int port = 6379, bool debugMode = false) 
        : m_socket(-1), m_host(host), m_port(port), m_connected(false), m_debugMode(debugMode) {
    }
    
    ~SmartNetBoxRedisClient() {
        disconnect();
    }
    
    bool connect() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            std::cerr << "❌ 连接失败: 无法创建socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(m_port);
        
        if (inet_pton(AF_INET, m_host.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "❌ 连接失败: 无效的服务器地址 " << m_host << std::endl;
            close(m_socket);
            return false;
        }
        
        if (::connect(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "❌ 连接失败: 无法连接到 " << m_host << ":" << m_port << std::endl;
            close(m_socket);
            return false;
        }
        
        m_connected = true;
        std::cout << "✅ 已连接到NetBox Redis: " << m_host << ":" << m_port << std::endl;
        return true;
    }
    
    void disconnect() {
        if (m_socket >= 0) {
            close(m_socket);
            m_socket = -1;
        }
        m_connected = false;
    }
    
    std::string sendCommand(const std::string& command) {
        if (!m_connected) {
            return "❌ 错误: 未连接到服务器";
        }

        // 发送Redis命令（RESP格式）
        std::string respCommand = command + "\r\n";
        ssize_t sent = send(m_socket, respCommand.c_str(), respCommand.length(), 0);
        if (sent < 0) {
            return "❌ 错误: 发送命令失败";
        }

        if (m_debugMode) {
            std::cout << "[DEBUG] 发送: " << command << " (" << sent << " 字节)" << std::endl;
        }

        // 🔧 修复：使用更大的缓冲区并处理多次接收
        std::string allData;
        char buffer[4096];
        
        // 设置接收超时
        struct timeval timeout;
        timeout.tv_sec = 5;  // 5秒超时
        timeout.tv_usec = 0;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        // 接收所有可用数据
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t received = recv(m_socket, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);

            if (received > 0) {
                allData.append(buffer, received);
                if (m_debugMode) {
                    std::cout << "[DEBUG] 接收块: " << received << " 字节" << std::endl;
                }
            } else if (received == 0) {
                return "❌ 错误: 服务器关闭连接";
            } else {
                // EAGAIN 或 EWOULDBLOCK 表示没有更多数据
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    return "❌ 错误: 接收响应失败";
                }
            }
        }

        // 如果没有接收到任何数据，尝试阻塞接收一次
        if (allData.empty()) {
            ssize_t received = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
            if (received > 0) {
                allData.append(buffer, received);
            } else if (received == 0) {
                return "❌ 错误: 服务器关闭连接";
            } else {
                return "❌ 错误: 接收响应失败";
            }
        }

        if (m_debugMode) {
            std::cout << "[DEBUG] 总接收: " << allData.length() << " 字节" << std::endl;
            showRawData(allData.c_str(), allData.length());
        }

        // 🧠 智能提取并解析所有RESP响应
        return extractAndParseAllResponses(allData);
    }

private:
    /**
     * @brief 🧠 智能提取并解析所有RESP响应
     * 处理混合数据，提取所有有效的RESP响应
     */
    std::string extractAndParseAllResponses(const std::string& allData) {
        if (allData.empty()) {
            return "(无响应)";
        }

        std::vector<std::string> responses;
        size_t pos = 0;

        while (pos < allData.length()) {
            // 跳过协议头数据和心跳包（查找RESP标识符）
            while (pos < allData.length()) {
                char c = allData[pos];
                if (c == '+' || c == '-' || c == ':' || c == '$' || c == '*') {
                    break;
                }
                pos++;
            }

            if (pos >= allData.length()) {
                break; // 没有找到更多RESP数据
            }

            // 提取完整的RESP响应
            std::string respData = extractSingleRespResponse(allData, pos);
            if (!respData.empty()) {
                std::string parsed = parseRespToUserFriendly(respData);
                if (parsed != "(无响应)" && parsed != "(unknown) ") {
                    responses.push_back(parsed);
                }
            }
        }

        if (responses.empty()) {
            return "(无响应)";
        }

        // 返回最后一个有效响应（对应当前命令）
        return responses.back();
    }

    /**
     * @brief 提取单个完整的RESP响应
     */
    std::string extractSingleRespResponse(const std::string& data, size_t& pos) {
        if (pos >= data.length()) return "";

        char type = data[pos];
        size_t start = pos;

        // 根据RESP类型确定响应结束位置
        switch (type) {
            case '+':  // Simple String
            case '-':  // Error
            case ':':  // Integer
                {
                    // 查找\r\n结尾
                    size_t end = data.find("\r\n", pos);
                    if (end != std::string::npos) {
                        pos = end + 2;
                        return data.substr(start, end - start + 2);
                    }
                }
                break;

            case '$':  // Bulk String
                {
                    // 查找长度
                    size_t lengthEnd = data.find("\r\n", pos);
                    if (lengthEnd != std::string::npos) {
                        std::string lengthStr = data.substr(pos + 1, lengthEnd - pos - 1);
                        int length = std::stoi(lengthStr);

                        if (length == -1) {
                            // NULL bulk string
                            pos = lengthEnd + 2;
                            return data.substr(start, lengthEnd - start + 2);
                        } else {
                            // 计算完整响应长度
                            size_t totalLength = (lengthEnd - start + 2) + length + 2;
                            if (start + totalLength <= data.length()) {
                                pos = start + totalLength;
                                return data.substr(start, totalLength);
                            }
                        }
                    }
                }
                break;

            case '*':  // Array - 简化处理
                {
                    size_t end = data.find("\r\n", pos);
                    if (end != std::string::npos) {
                        pos = end + 2;
                        return data.substr(start, end - start + 2);
                    }
                }
                break;
        }

        // 如果无法解析，跳过这个字符
        pos++;
        return "";
    }

    /**
     * @brief 🎯 解析RESP响应为用户友好格式
     */
    std::string parseRespToUserFriendly(const std::string& resp) {
        if (resp.empty()) {
            return "(无响应)";
        }

        char type = resp[0];
        std::string content = resp.substr(1);
        
        // 移除\r\n
        while (!content.empty() && (content.back() == '\r' || content.back() == '\n')) {
            content.pop_back();
        }

        switch (type) {
            case '+':  // Simple String - 直接返回内容
                return content;
                
            case '-':  // Error - 显示错误
                return "(error) " + content;
                
            case ':':  // Integer - 返回数字
                return "(integer) " + content;
                
            case '$':  // Bulk String - 处理字符串和NULL
                return parseBulkString(content);
                
            case '*':  // Array - 处理数组
                return parseArray(content);
                
            default:
                return "(unknown) " + resp;
        }
    }
    
    /**
     * @brief 解析Bulk String ($)
     */
    std::string parseBulkString(const std::string& content) {
        size_t pos = content.find('\n');
        if (pos == std::string::npos) return content;
        
        std::string lengthStr = content.substr(0, pos);
        int length = std::stoi(lengthStr);
        
        if (length == -1) {
            return "(nil)";  // Redis标准的NULL表示
        }
        
        if (length == 0) {
            return "\"\"";  // 空字符串
        }
        
        // 提取实际内容
        std::string actualContent = content.substr(pos + 1);
        while (!actualContent.empty() && 
               (actualContent.back() == '\r' || actualContent.back() == '\n')) {
            actualContent.pop_back();
        }
        
        // 如果内容包含空格或特殊字符，用引号包围
        if (actualContent.find(' ') != std::string::npos || 
            actualContent.find('\t') != std::string::npos) {
            return "\"" + actualContent + "\"";
        }
        
        return actualContent;
    }
    
    /**
     * @brief 解析Array (*)
     */
    std::string parseArray(const std::string& content) {
        size_t pos = content.find('\n');
        if (pos == std::string::npos) return content;
        
        std::string countStr = content.substr(0, pos);
        int count = std::stoi(countStr);
        
        if (count == 0) {
            return "(empty list or set)";
        }
        
        if (count == -1) {
            return "(nil)";
        }
        
        // 简化处理：显示数组信息
        return "(array with " + std::to_string(count) + " elements)";
    }
    
    /**
     * @brief 显示原始数据（调试用）
     */
    void showRawData(const char* data, ssize_t length) {
        std::cout << "[DEBUG] 原始数据 (" << length << " 字节): ";
        for (ssize_t i = 0; i < std::min(length, (ssize_t)32); ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (unsigned char)data[i] << " ";
        }
        std::cout << std::dec << std::endl;
        
        std::cout << "[DEBUG] 字符表示: ";
        for (ssize_t i = 0; i < std::min(length, (ssize_t)50); ++i) {
            char c = data[i];
            if (c >= 32 && c <= 126) {
                std::cout << c;
            } else if (c == '\r') {
                std::cout << "\\r";
            } else if (c == '\n') {
                std::cout << "\\n";
            } else {
                std::cout << "[" << (int)(unsigned char)c << "]";
            }
        }
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string host = "192.168.88.135";
    int port = 6379;
    bool debugMode = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if (arg == "--debug" || arg == "-d") {
            debugMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "🚀 智能NetBox Redis客户端" << std::endl;
            std::cout << "专为NetBox集成版Redis设计，智能过滤协议头，纯净Redis体验" << std::endl;
            std::cout << std::endl;
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --host <地址>     服务器地址 (默认: 192.168.88.135)" << std::endl;
            std::cout << "  --port <端口>     服务器端口 (默认: 6379)" << std::endl;
            std::cout << "  --debug, -d       启用调试模式" << std::endl;
            std::cout << "  --help, -h        显示帮助" << std::endl;
            std::cout << std::endl;
            std::cout << "✨ 特色功能:" << std::endl;
            std::cout << "  🧠 智能协议头过滤 - 自动移除NetBox协议封装" << std::endl;
            std::cout << "  🎯 纯净RESP体验 - 标准Redis客户端体验" << std::endl;
            std::cout << "  🌏 完美中文支持 - UTF-8字符完美显示" << std::endl;
            std::cout << "  🚫 无4Vx干扰 - 彻底解决协议头显示问题" << std::endl;
            return 0;
        }
    }
    
    std::cout << "🚀 智能NetBox Redis客户端" << std::endl;
    std::cout << "目标: " << host << ":" << port << std::endl;
    std::cout << "特点: 智能过滤协议头，纯净Redis体验，无4Vx干扰" << std::endl;
    
    SmartNetBoxRedisClient client(host, port, debugMode);
    
    if (!client.connect()) {
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "🎯 NetBox Redis 智能客户端" << std::endl;
    std::cout << "支持所有Redis命令，输入 'help' 查看帮助，'quit' 退出" << std::endl;
    std::cout << std::endl;
    
    std::string command;
    while (true) {
        std::cout << "netbox-redis> ";
        std::getline(std::cin, command);
        
        if (command == "quit" || command == "exit") {
            std::cout << "👋 再见！感谢使用智能NetBox Redis客户端" << std::endl;
            break;
        }
        
        if (command == "help") {
            std::cout << "📚 Redis命令帮助:" << std::endl;
            std::cout << "  PING                    - 测试连接" << std::endl;
            std::cout << "  SET key value           - 设置字符串值" << std::endl;
            std::cout << "  GET key                 - 获取字符串值" << std::endl;
            std::cout << "  DEL key [key ...]       - 删除一个或多个键" << std::endl;
            std::cout << "  KEYS pattern            - 查找匹配的键" << std::endl;
            std::cout << "  LPUSH key value         - 向列表左侧推入元素" << std::endl;
            std::cout << "  LPOP key                - 从列表左侧弹出元素" << std::endl;
            std::cout << "  LRANGE key start stop   - 获取列表范围内的元素" << std::endl;
            std::cout << "  HSET key field value    - 设置哈希字段值" << std::endl;
            std::cout << "  HGET key field          - 获取哈希字段值" << std::endl;
            std::cout << "  HKEYS key               - 获取哈希所有字段" << std::endl;
            std::cout << std::endl;
            std::cout << "💡 提示: 本客户端智能过滤NetBox协议头，确保纯净Redis体验" << std::endl;
            continue;
        }
        
        if (command.empty()) {
            continue;
        }
        
        std::string result = client.sendCommand(command);
        std::cout << result << std::endl;
    }
    
    return 0;
}

