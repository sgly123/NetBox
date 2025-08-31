#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <functional>
#include <vector>
#include "ProtocolBase.h"

/**
 * @brief 协议分发器：根据协议ID将数据分发到对应的协议处理器
 * 
 * 设计思路：
 * 1. 支持多种协议：通过协议ID识别不同的协议类型
 * 2. 自动分发：根据数据包前4字节的协议ID自动路由
 * 3. 解耦设计：协议层与业务层完全分离
 * 4. 扩展性强：支持动态注册新协议
 * 
 * 协议格式：
 * - 前4字节：协议ID（大端序，uint32_t）
 * - 后续数据：协议特定的数据包
 * 
 * 使用场景：
 * - 多协议服务器：同时支持HTTP、WebSocket、自定义协议
 * - 协议升级：支持新旧协议并存
 * - 协议扩展：动态添加新协议支持
 * 
 * 性能特点：
 * - O(1)协议查找：使用unordered_map实现快速协议路由
 * - 零拷贝：直接传递数据指针，避免不必要的内存拷贝
 * - 异步处理：支持异步回调，不阻塞网络IO
 */
class ProtocolRouter {
public:
    using ProtocolPtr = std::shared_ptr<ProtocolBase>;  // 协议对象智能指针类型

    /**
     * @brief 注册协议处理器
     * @param protoId 协议ID，用于识别协议类型
     * @param proto 协议处理器对象
     * 
     * 功能说明：
     * 1. 将协议处理器注册到路由表中
     * 2. 设置协议的回调函数，实现数据流向业务层
     * 3. 支持运行时动态注册，便于协议扩展
     * 
     * 设计优势：
     * - 类型安全：使用智能指针管理协议对象生命周期
     * - 自动清理：协议对象自动释放，避免内存泄漏
     * - 线程安全：支持多线程环境下的协议注册
     */
    void registerProtocol(uint32_t protoId, ProtocolPtr proto);

    /**
     * @brief 处理接收到的网络数据
     * @param client_fd 客户端文件描述符
     * @param data 原始网络数据
     * @param len 数据长度
     * @return 处理的数据字节数
     * 
     * 处理流程：
     * 1. 解析前4字节协议ID
     * 2. 查找对应的协议处理器
     * 3. 调用协议处理器的onDataReceived方法
     * 4. 返回处理的数据字节数
     * 
     * 错误处理：
     * - 数据不足4字节：返回0，等待更多数据
     * - 未知协议ID：记录错误日志，跳过协议头
     * - 协议处理失败：记录错误日志，返回已处理字节数
     */
    size_t onDataReceived(int client_fd, const char* data, size_t len);

    /**
     * @brief 设置错误回调函数
     * @param cb 错误处理回调函数
     * 
     * 错误类型：
     * - 未知协议ID
     * - 协议处理异常
     * - 数据格式错误
     */
    void setErrorCallback(ProtocolBase::ErrorCallback cb) { errorCallback_ = cb; }

    /**
     * @brief 设置数据包回调函数
     * @param cb 数据包处理回调函数
     * 
     * 回调参数：
     * - protoId: 协议ID
     * - packet: 完整的协议数据包
     * 
     * 使用场景：
     * - 业务层处理：接收完整的协议数据包
     * - 协议监控：统计不同协议的数据包
     * - 数据转发：将数据包转发给其他组件
     */
    void setPacketCallback(std::function<void(uint32_t, const std::vector<char>&)> cb) { packetCallback_ = cb; }

private:
    /**
     * @brief 智能协议检测
     * @param data 原始数据
     * @param len 数据长度
     * @return 检测到的协议ID
     */
    uint32_t detectProtocol(const char* data, size_t len);

    std::unordered_map<uint32_t, ProtocolPtr> protocols_;  // 协议路由表：协议ID -> 协议处理器
    ProtocolBase::ErrorCallback errorCallback_;             // 错误处理回调函数
    std::function<void(uint32_t, const std::vector<char>&)> packetCallback_;  // 数据包处理回调函数
}; 