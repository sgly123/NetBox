#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

// 使用紧凑对齐（虽然当前结构体不需要）
#pragma pack(push, 1)

struct ProtocolHeader {
    uint32_t body_length;  // 消息体长度（网络字节序）
    
    // 序列化消息头
    void serialize(char* buffer) const {
        uint32_t net_len = htonl(body_length);
        std::memcpy(buffer, &net_len, sizeof(net_len));
    }
    
    // 反序列化消息头
    static bool deserialize(const char* data, size_t len, ProtocolHeader& header) {
        if (len < sizeof(ProtocolHeader)) {
            return false;
        }
        
        uint32_t net_len;
        std::memcpy(&net_len, data, sizeof(net_len));
        header.body_length = ntohl(net_len);
        return true;
    }
};

class ProtocolMessage {
public:
    ProtocolHeader header;
    std::vector<char> body;
    
    // 获取完整消息大小
    size_t full_size() const {
        return sizeof(ProtocolHeader) + body.size();
    }
    
    // 序列化整个消息
    std::vector<char> serialize() const {
        std::vector<char> data;
        data.resize(full_size());
        
        // 序列化头部
        header.serialize(data.data());
        
        // 拷贝消息体
        if (!body.empty()) {
            std::memcpy(data.data() + sizeof(ProtocolHeader), 
                        body.data(), 
                        body.size());
        }
        
        return data;
    }

    // 反序列化整个消息（更安全的版本）
    static bool deserialize(const char* data, size_t len, ProtocolMessage& out_msg) {
        // 1. 解析头部
        ProtocolHeader temp_header;
        if (!ProtocolHeader::deserialize(data, len, temp_header)) {
            return false; // 头部解析失败
        }
        
        // 2. 验证消息体长度
        const size_t expected_size = sizeof(ProtocolHeader) + temp_header.body_length;
        if (len < expected_size) {
            return false; // 数据不完整
        }
        
        // 3. 验证长度合理性（防止恶意超大包）
        if (temp_header.body_length > MAX_BODY_LENGTH) {
            return false; // 消息体过大
        }
        
        // 4. 填充结果
        out_msg.header = temp_header;
        out_msg.body.assign(
            data + sizeof(ProtocolHeader),
            data + expected_size
        );
        
        return true;
    }
    
    // 添加字符串（带长度前缀）
    void add_string(const std::string& str) {
        // 添加长度前缀（网络字节序）
        uint32_t len = static_cast<uint32_t>(str.size());
        uint32_t net_len = htonl(len);
        
        const char* len_ptr = reinterpret_cast<const char*>(&net_len);
        body.insert(body.end(), len_ptr, len_ptr + sizeof(net_len));
        
        // 添加字符串内容
        body.insert(body.end(), str.begin(), str.end());
        
        // 更新总长度
        header.body_length = body.size();
    }
    
    // 获取字符串列表
    std::vector<std::string> get_strings() const {
        std::vector<std::string> result;
        const char* ptr = body.data();
        size_t remaining = body.size();
        
        while (remaining >= sizeof(uint32_t)) {
            // 读取长度前缀
            uint32_t net_len;
            std::memcpy(&net_len, ptr, sizeof(net_len));
            ptr += sizeof(net_len);
            remaining -= sizeof(net_len);
            
            uint32_t str_len = ntohl(net_len);
            
            // 验证长度有效性
            if (str_len > remaining) {
                throw std::runtime_error("Invalid string length in message body");
            }
            
            // 提取字符串
            result.emplace_back(ptr, str_len);
            ptr += str_len;
            remaining -= str_len;
        }
        
        if (remaining > 0) {
            throw std::runtime_error("Extra bytes in message body");
        }
        
        return result;
    }

private:
    // 最大消息体长度限制（根据实际需求调整）
    static constexpr size_t MAX_BODY_LENGTH = 10 * 1024 * 1024; // 10MB
};

#pragma pack(pop)

#endif // PROTOCOL_H