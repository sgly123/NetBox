#include "SimpleHeaderProtocol.h"
#include "base/Logger.h"
#include <arpa/inet.h>
#include <cstring>
#include <algorithm>
#include "ProtocolRegister.h"
REGISTER_PROTOCOL(SimpleHeaderProtocol)

/**
 * @brief 构造函数：初始化协议状态
 * 
 * 初始化内容：
 * - 创建PooledBuffer接收缓冲区
 * - 设置默认的最大包大小
 * - 初始化解析状态
 */
SimpleHeaderProtocol::SimpleHeaderProtocol() 
    : buffer_(4096), parsingHeader_(false), expectedBodyLen_(0) {
    // 使用4KB初始容量，适合大多数网络数据包
    buffer_.reserve(4096);
}

/**
 * @brief 处理接收到的原始数据流（拆包）
 * @param data 原始数据流指针
 * @param len 数据长度
 * @return 处理的数据字节数
 */
size_t SimpleHeaderProtocol::onDataReceived(const char* data, size_t len) {
    if (len == 0) return 0;
    
    // 流量控制检查
    if (!checkFlowControl(len)) {
        Logger::warn("流量控制：接收速率超限");
        return 0;
    }
    
    // 将新数据追加到缓冲区
    buffer_.insert(buffer_.end(), data, data + len);
    
    size_t processed = 0;
    size_t buffer_size = buffer_.size();
    
    // 循环处理完整的数据包
    while (buffer_size >= 4) {  // 至少需要4字节包头
        uint32_t bodyLen = 0;
        
        // 解析包头（4字节包体长度，大端序）
        std::memcpy(&bodyLen, buffer_.data(), 4);
        bodyLen = ntohl(bodyLen);  // 网络字节序转主机字节序
        
        // 检查包体大小是否合理
        if (bodyLen > maxPacketSize_) {
            Logger::error("包体过大: " + std::to_string(bodyLen) + " bytes");
            if (errorCallback_) {
                errorCallback_("Packet too large: " + std::to_string(bodyLen) + " bytes");
            }
            reset();
            return processed;
        }
        
        // 检查是否有完整的包（包头4字节 + 包体）
        if (buffer_size < 4 + bodyLen) {
            // 数据不完整，等待更多数据
            break;
        }
        
        // 提取完整的包体数据
        std::vector<char> packet(bodyLen);
        std::memcpy(packet.data(), buffer_.data() + 4, bodyLen);
        
        // 调用回调函数处理完整的数据包
        if (packetCallback_) {
            packetCallback_(packet);
        }
        
        // 移除已处理的数据（包头 + 包体）
        buffer_.erase(buffer_.begin(), buffer_.begin() + 4 + bodyLen);
        processed += 4 + bodyLen;
        buffer_size = buffer_.size();
    }
    
    return processed;
}

/**
 * @brief 封包要发送的数据（std::vector版本，保持兼容性）
 * @param data 要发送的数据指针
 * @param len 数据长度
 * @param out 输出缓冲区
 * @return 封包是否成功
 */
bool SimpleHeaderProtocol::pack(const char* data, size_t len, std::vector<char>& out) {
    if (len > maxPacketSize_) {
        Logger::error("数据过大，无法封包: " + std::to_string(len) + " bytes");
        return false;
    }
    
    // 预分配内存：包头4字节 + 包体
    out.resize(4 + len);
    
    // 写入包头（包体长度，大端序）
    uint32_t bodyLen = htonl(static_cast<uint32_t>(len));
    std::memcpy(out.data(), &bodyLen, 4);
    
    // 写入包体数据
    if (len > 0) {
        std::memcpy(out.data() + 4, data, len);
    }
    
    return true;
}



/**
 * @brief 重置协议状态
 */
void SimpleHeaderProtocol::reset() {
    buffer_.clear();
    parsingHeader_ = false;
    expectedBodyLen_ = 0;
    Logger::info("协议状态已重置");
}

/**
 * @brief 检查流量控制
 * @param bytes 要处理的字节数
 * @return 是否允许处理
 */
bool SimpleHeaderProtocol::checkFlowControl(size_t bytes) {
    // 简化的流量控制：限制每秒处理的数据量
    static size_t lastCheckTime = 0;
    static size_t bytesThisSecond = 0;
    static const size_t MAX_BYTES_PER_SECOND = 1024 * 1024; // 1MB/s
    
    size_t currentTime = getCurrentTime();
    
    if (currentTime - lastCheckTime >= 1000) {  // 1秒时间窗口
        bytesThisSecond = 0;
        lastCheckTime = currentTime;
        }
    
    if (bytesThisSecond + bytes > MAX_BYTES_PER_SECOND) {
            return false;
    }
    
    bytesThisSecond += bytes;
    return true;
}

/**
 * @brief 获取当前时间戳（毫秒）
 * @return 当前时间戳
 */
size_t SimpleHeaderProtocol::getCurrentTime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
} 