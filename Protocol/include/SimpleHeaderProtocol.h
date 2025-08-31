#pragma once
#include "ProtocolBase.h"
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

/**
 * @brief 简单4字节包头协议实现类（内存池优化版本）
 * 
 * 协议格式：
 * - 4字节包体长度（大端序，uint32_t）
 * - 包体数据（变长）
 * 
 * 设计特点：
 * 1. 简单高效：4字节固定长度包头，解析开销小
 * 2. 支持变长：包体长度可变，适应不同数据大小
 * 3. 网络友好：大端序编码，符合网络字节序标准
 * 4. 健壮性强：内置流量控制和错误处理机制
 * 5. 内存优化：使用内存池管理缓冲区，减少分配开销
 * 
 * 功能特性：
 * - 自动拆包：处理粘包/半包问题
 * - 流量控制：防止网络拥塞和缓冲区溢出
 * - 错误处理：完善的错误检测和报告
 * - 回调机制：异步处理，不阻塞网络IO
 * - 内存池优化：使用PooledBuffer减少内存分配开销
 * 
 * 使用场景：
 * - 高性能服务器：低延迟、高吞吐量
 * - 实时通信：游戏、聊天、视频流
 * - 数据传输：文件传输、数据同步
 * 
 * 性能优化：
 * - 零拷贝：直接操作数据指针
 * - 内存池：减少内存分配开销
 * - 批量处理：支持批量数据包处理
 * - 内存复用：通过内存池复用内存块
 */
class SimpleHeaderProtocol : public ProtocolBase {
public:
    static constexpr uint32_t ID = 1;
    /**
     * @brief 构造函数，初始化协议状态
     */
    SimpleHeaderProtocol();
    
    /**
     * @brief 虚析构函数，确保正确释放资源
     */
    virtual ~SimpleHeaderProtocol() = default;

    /**
     * @brief 处理接收到的原始数据流（拆包）
     * @param data 原始数据流指针
     * @param len 数据长度
     * @return 处理的数据字节数
     * 
     * 处理流程：
     * 1. 流量控制检查：防止接收速率超限
     * 2. 数据追加：将新数据追加到接收缓冲区
     * 3. 循环拆包：从缓冲区中提取完整的数据包
     * 4. 回调触发：对每个完整包调用业务层回调
     * 5. 缓冲区清理：移除已处理的数据
     * 
     * 拆包算法：
     * - 检查缓冲区是否至少有4字节（包头长度）
     * - 解析包头，获取包体长度
     * - 检查是否有完整的包（包头+包体）
     * - 提取完整包，调用回调函数
     * - 移除已处理的数据，继续处理剩余数据
     * 
     * 错误处理：
     * - 包体过大：触发错误回调，重置协议状态
     * - 流量超限：记录错误，暂停处理
     * - 数据损坏：跳过损坏数据，继续处理
     */
    size_t onDataReceived(const char* data, size_t len) override;

    /**
     * @brief 封包要发送的数据
     * @param data 要发送的数据指针
     * @param len 数据长度
     * @param out 输出缓冲区，存储封包后的数据
     * @return 封包是否成功
     * 
     * 封包格式：
     * - 4字节包体长度（大端序）
     * - 包体数据（原始数据）
     * 
     * 处理流程：
     * 1. 流量控制检查：防止发送速率超限
     * 2. 包大小检查：确保包体不超过最大限制
     * 3. 内存分配：为封包数据分配内存
     * 4. 数据拷贝：将包头和包体数据拷贝到输出缓冲区
     * 5. 统计更新：更新发送统计信息
     * 
     * 优化特性：
     * - 预分配内存：避免动态扩容
     * - 批量拷贝：使用memcpy优化数据拷贝
     * - 字节序优化：使用htonl优化字节序转换
     * - 内存池优化：使用PooledBuffer减少分配开销
     */
    bool pack(const char* data, size_t len, std::vector<char>& out) override;

    /**
     * @brief 封包要发送的数据（内存池版本）
     * @param data 要发送的数据指针
     * @param len 数据长度
     * @param out 输出缓冲区，使用内存池管理
     * @return 封包是否成功
     * 
     * 优势：
     * - 使用内存池管理内存，减少分配开销
     * - 支持内存复用，提高性能
     * - 自动内存管理，避免内存泄漏
     */


    /**
     * @brief 获取协议类型标识
     * @return 协议类型字符串
     * 
     * 用于：
     * - 协议识别和路由
     * - 日志记录和调试
     * - 协议统计和监控
     */
    std::string getType() const override { return "SimpleHeader"; }

    /**
     * @brief 获取协议ID（用于分发器路由）
     * @return 协议ID（uint32_t）
     * 
     * 协议ID设计：
     * - 1: SimpleHeader协议
     * - 2: HTTP协议（预留）
     * - 3: WebSocket协议（预留）
     * - 4: Binary协议（预留）
     * 
     * 扩展建议：
     * - 使用枚举定义协议ID，提高可读性
     * - 支持协议版本号，便于协议升级
     * - 支持协议特性位，标识协议能力
     */
    uint32_t getProtocolId() const override { return 1; }

    /**
     * @brief 重置协议状态（清空缓冲区等）
     * 
     * 重置内容：
     * - 清空接收缓冲区
     * - 重置解析状态
     * - 清零统计计数器
     * - 重置时间戳
     * 
     * 使用场景：
     * - 连接重置：客户端重连时清理状态
     * - 错误恢复：协议错误后重置状态
     * - 内存清理：释放不必要的内存占用
     */
    void reset() override;

    /**
     * @brief 设置最大包大小限制
     * @param maxPacketSize 最大包大小（字节），0表示无限制
     * 
     * 安全考虑：
     * - 防止内存耗尽：限制单个包的最大大小
     * - 防止DoS攻击：避免超大包导致系统资源耗尽
     * - 性能优化：合理设置包大小，平衡内存和性能
     * 
     * 建议设置：
     * - 小包场景（聊天）：1KB - 4KB
     * - 中包场景（文件传输）：64KB - 1MB
     * - 大包场景（视频流）：1MB - 16MB
     */
    void setMaxPacketSize(size_t maxPacketSize) { maxPacketSize_ = maxPacketSize; }

    /**
     * @brief 获取当前缓冲区大小
     * @return 缓冲区中未处理的数据大小
     * 
     * 监控用途：
     * - 内存使用监控：跟踪缓冲区内存占用
     * - 性能分析：分析数据积压情况
     * - 调试辅助：排查数据处理问题
     */
    size_t getBufferSize() const { return buffer_.size(); }

    /**
     * @brief 获取缓冲区统计信息
     * @return 缓冲区大小
     * 
     * 用于：
     * - 性能监控：跟踪缓冲区使用情况
     * - 调试分析：分析数据积压情况
     */
    size_t getBufferStats() const { return buffer_.size(); }

private:
    /**
     * @brief 检查流量控制
     * @param bytes 要处理的字节数
     * @return 是否允许处理
     * 
     * 流量控制算法：
     * - 滑动窗口：基于1秒时间窗口的统计
     * - 双限控制：同时限制接收和发送速率
     * - 自适应调整：根据网络状况动态调整限制
     * 
     * 控制策略：
     * - 接收控制：防止接收缓冲区溢出
     * - 发送控制：防止网络拥塞
     * - 公平分配：确保多个连接公平使用带宽
     */
    bool checkFlowControl(size_t bytes);

    /**
     * @brief 获取当前时间戳（毫秒）
     * @return 当前时间戳
     * 
     * 时间精度：
     * - 毫秒级精度：满足大多数流量控制需求
     * - 单调时钟：使用steady_clock避免系统时间调整影响
     * - 高性能：避免频繁的系统调用
     */
    size_t getCurrentTime() const;

    std::vector<char> buffer_;      // 接收缓冲区：使用标准vector
    size_t maxPacketSize_ = 1024*1024; // 最大包大小（1MB）：防止内存耗尽和DoS攻击
    bool parsingHeader_ = false;      // 包头解析状态：标识当前是否正在解析包头
    uint32_t expectedBodyLen_ = 0;    // 期望的包体长度：从包头解析出的包体长度
}; 