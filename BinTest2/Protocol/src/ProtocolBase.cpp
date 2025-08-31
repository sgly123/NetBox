#include "ProtocolBase.h"
#include <chrono>

/**
 * @brief 检查流量控制
 * @param bytes 要处理的字节数
 * @return 是否允许处理
 */
bool ProtocolBase::checkFlowControl(size_t bytes) {
    // 如果没有设置流量控制，直接允许
    if (maxReceiveRate_ == 0) {
        return true;
    }
    
    size_t currentTime = getCurrentTime();
    
    // 如果超过1秒，重置计数器
    if (currentTime - lastReceiveTime_ >= 1000) {
        receiveBytes_ = 0;
        lastReceiveTime_ = currentTime;
    }
    
    // 检查是否超过限制
    if (receiveBytes_ + bytes > maxReceiveRate_) {
        return false;
    }
    
    receiveBytes_ += bytes;
    return true;
}

/**
 * @brief 获取当前时间戳（毫秒）
 * @return 当前时间戳
 */
size_t ProtocolBase::getCurrentTime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
} 