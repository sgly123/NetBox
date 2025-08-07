#pragma once
#include <functional>

/**
 * @brief 线程池抽象接口，定义通用任务投递方法
 */
class IThreadPool {
public:
    virtual ~IThreadPool() = default;

    /**
     * @brief 投递任务到线程池
     * @param task 任意可调用对象
     * @return 是否成功投递任务
     */
    virtual bool enqueue(const std::function<void()>& task) = 0;
}; 