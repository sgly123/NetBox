#pragma once

/**
 * @file MetricsCollector.h
 * @brief NetBox 监控指标收集系统
 * 
 * 特性：
 * 1. 多种指标类型 (Counter, Gauge, Histogram, Timer)
 * 2. 高性能指标收集
 * 3. 多种导出格式 (Prometheus, JSON, InfluxDB)
 * 4. 自动系统指标收集 (CPU, 内存, 网络)
 * 5. 自定义指标支持
 * 6. 指标聚合和统计
 * 7. 实时监控面板
 */

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <functional>
#include <sstream>
#include <iomanip>

namespace NetBox {
namespace Monitoring {

/**
 * @brief 指标类型枚举
 */
enum class MetricType {
    COUNTER,    // 计数器 - 只增不减
    GAUGE,      // 仪表 - 可增可减
    HISTOGRAM,  // 直方图 - 分布统计
    TIMER       // 计时器 - 时间统计
};

/**
 * @brief 指标标签
 */
using Labels = std::unordered_map<std::string, std::string>;

/**
 * @brief 基础指标接口
 */
class Metric {
protected:
    std::string m_name;
    std::string m_description;
    Labels m_labels;
    MetricType m_type;
    std::chrono::system_clock::time_point m_lastUpdated;

public:
    Metric(const std::string& name, const std::string& description, 
           MetricType type, const Labels& labels = {})
        : m_name(name), m_description(description), m_labels(labels), 
          m_type(type), m_lastUpdated(std::chrono::system_clock::now()) {}
    
    virtual ~Metric() = default;
    
    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }
    const Labels& getLabels() const { return m_labels; }
    MetricType getType() const { return m_type; }
    
    virtual std::string serialize() const = 0;
    virtual void reset() = 0;
    
protected:
    void updateTimestamp() {
        m_lastUpdated = std::chrono::system_clock::now();
    }
};

/**
 * @brief 计数器指标
 */
class Counter : public Metric {
private:
    std::atomic<uint64_t> m_value{0};

public:
    Counter(const std::string& name, const std::string& description, 
            const Labels& labels = {})
        : Metric(name, description, MetricType::COUNTER, labels) {}
    
    void increment(uint64_t delta = 1) {
        m_value.fetch_add(delta);
        updateTimestamp();
    }
    
    uint64_t getValue() const {
        return m_value.load();
    }
    
    std::string serialize() const override {
        std::ostringstream oss;
        oss << "# TYPE " << m_name << " counter\n";
        oss << "# HELP " << m_name << " " << m_description << "\n";
        oss << m_name;
        
        if (!m_labels.empty()) {
            oss << "{";
            bool first = true;
            for (const auto& label : m_labels) {
                if (!first) oss << ",";
                oss << label.first << "=\"" << label.second << "\"";
                first = false;
            }
            oss << "}";
        }
        
        oss << " " << m_value.load() << "\n";
        return oss.str();
    }
    
    void reset() override {
        m_value.store(0);
        updateTimestamp();
    }
};

/**
 * @brief 仪表指标
 */
class Gauge : public Metric {
private:
    std::atomic<double> m_value{0.0};

public:
    Gauge(const std::string& name, const std::string& description, 
          const Labels& labels = {})
        : Metric(name, description, MetricType::GAUGE, labels) {}
    
    void setValue(double value) {
        m_value.store(value);
        updateTimestamp();
    }
    
    void increment(double delta = 1.0) {
        double current = m_value.load();
        while (!m_value.compare_exchange_weak(current, current + delta)) {
            // 重试直到成功
        }
        updateTimestamp();
    }
    
    void decrement(double delta = 1.0) {
        increment(-delta);
    }
    
    double getValue() const {
        return m_value.load();
    }
    
    std::string serialize() const override {
        std::ostringstream oss;
        oss << "# TYPE " << m_name << " gauge\n";
        oss << "# HELP " << m_name << " " << m_description << "\n";
        oss << m_name;
        
        if (!m_labels.empty()) {
            oss << "{";
            bool first = true;
            for (const auto& label : m_labels) {
                if (!first) oss << ",";
                oss << label.first << "=\"" << label.second << "\"";
                first = false;
            }
            oss << "}";
        }
        
        oss << " " << std::fixed << std::setprecision(6) << m_value.load() << "\n";
        return oss.str();
    }
    
    void reset() override {
        m_value.store(0.0);
        updateTimestamp();
    }
};

/**
 * @brief 直方图指标
 */
class Histogram : public Metric {
private:
    std::vector<double> m_buckets;
    std::vector<std::atomic<uint64_t>> m_bucketCounts;
    std::atomic<uint64_t> m_count{0};
    std::atomic<double> m_sum{0.0};
    mutable std::mutex m_mutex;

public:
    Histogram(const std::string& name, const std::string& description,
              const std::vector<double>& buckets = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0},
              const Labels& labels = {})
        : Metric(name, description, MetricType::HISTOGRAM, labels), m_buckets(buckets) {
        
        // 确保buckets是排序的
        std::sort(m_buckets.begin(), m_buckets.end());
        
        // 添加+Inf bucket
        m_buckets.push_back(std::numeric_limits<double>::infinity());
        
        // 初始化bucket计数器
        m_bucketCounts.resize(m_buckets.size());
        for (auto& counter : m_bucketCounts) {
            counter.store(0);
        }
    }
    
    void observe(double value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_count.fetch_add(1);
        
        double currentSum = m_sum.load();
        while (!m_sum.compare_exchange_weak(currentSum, currentSum + value)) {
            // 重试直到成功
        }
        
        // 更新bucket计数
        for (size_t i = 0; i < m_buckets.size(); ++i) {
            if (value <= m_buckets[i]) {
                m_bucketCounts[i].fetch_add(1);
            }
        }
        
        updateTimestamp();
    }
    
    uint64_t getCount() const {
        return m_count.load();
    }
    
    double getSum() const {
        return m_sum.load();
    }
    
    std::string serialize() const override {
        std::ostringstream oss;
        oss << "# TYPE " << m_name << " histogram\n";
        oss << "# HELP " << m_name << " " << m_description << "\n";
        
        // 输出buckets
        for (size_t i = 0; i < m_buckets.size(); ++i) {
            oss << m_name << "_bucket";
            
            if (!m_labels.empty()) {
                oss << "{";
                bool first = true;
                for (const auto& label : m_labels) {
                    if (!first) oss << ",";
                    oss << label.first << "=\"" << label.second << "\"";
                    first = false;
                }
                if (!first) oss << ",";
                oss << "le=\"";
                if (std::isinf(m_buckets[i])) {
                    oss << "+Inf";
                } else {
                    oss << m_buckets[i];
                }
                oss << "\"}";
            } else {
                oss << "{le=\"";
                if (std::isinf(m_buckets[i])) {
                    oss << "+Inf";
                } else {
                    oss << m_buckets[i];
                }
                oss << "\"}";
            }
            
            oss << " " << m_bucketCounts[i].load() << "\n";
        }
        
        // 输出总计数和总和
        oss << m_name << "_count " << m_count.load() << "\n";
        oss << m_name << "_sum " << std::fixed << std::setprecision(6) << m_sum.load() << "\n";
        
        return oss.str();
    }
    
    void reset() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_count.store(0);
        m_sum.store(0.0);
        for (auto& counter : m_bucketCounts) {
            counter.store(0);
        }
        updateTimestamp();
    }
};

/**
 * @brief 计时器指标
 */
class Timer : public Histogram {
private:
    std::chrono::high_resolution_clock::time_point m_startTime;

public:
    Timer(const std::string& name, const std::string& description,
          const Labels& labels = {})
        : Histogram(name + "_duration_seconds", description, 
                   {0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0}, 
                   labels) {}
    
    void start() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }
    
    void stop() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
        double seconds = duration.count() / 1000000.0;
        observe(seconds);
    }
    
    // RAII计时器辅助类
    class ScopedTimer {
    private:
        Timer& m_timer;
        
    public:
        ScopedTimer(Timer& timer) : m_timer(timer) {
            m_timer.start();
        }
        
        ~ScopedTimer() {
            m_timer.stop();
        }
    };
    
    ScopedTimer createScopedTimer() {
        return ScopedTimer(*this);
    }
};

/**
 * @brief 指标收集器
 */
class MetricsCollector {
private:
    std::unordered_map<std::string, std::shared_ptr<Metric>> m_metrics;
    mutable std::mutex m_mutex;

public:
    template<typename T, typename... Args>
    std::shared_ptr<T> createMetric(const std::string& name, Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto metric = std::make_shared<T>(name, std::forward<Args>(args)...);
        m_metrics[name] = metric;
        return metric;
    }
    
    std::shared_ptr<Counter> createCounter(const std::string& name, 
                                          const std::string& description,
                                          const Labels& labels = {}) {
        return createMetric<Counter>(name, description, labels);
    }
    
    std::shared_ptr<Gauge> createGauge(const std::string& name,
                                      const std::string& description,
                                      const Labels& labels = {}) {
        return createMetric<Gauge>(name, description, labels);
    }
    
    std::shared_ptr<Histogram> createHistogram(const std::string& name,
                                              const std::string& description,
                                              const std::vector<double>& buckets = {},
                                              const Labels& labels = {}) {
        if (buckets.empty()) {
            return createMetric<Histogram>(name, description, 
                std::vector<double>{0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0}, 
                labels);
        } else {
            return createMetric<Histogram>(name, description, buckets, labels);
        }
    }
    
    std::shared_ptr<Timer> createTimer(const std::string& name,
                                      const std::string& description,
                                      const Labels& labels = {}) {
        return createMetric<Timer>(name, description, labels);
    }
    
    std::shared_ptr<Metric> getMetric(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_metrics.find(name);
        return (it != m_metrics.end()) ? it->second : nullptr;
    }
    
    std::vector<std::shared_ptr<Metric>> getAllMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::shared_ptr<Metric>> metrics;
        for (const auto& pair : m_metrics) {
            metrics.push_back(pair.second);
        }
        return metrics;
    }
    
    std::string exportPrometheus() const {
        std::ostringstream oss;
        
        auto metrics = getAllMetrics();
        for (const auto& metric : metrics) {
            oss << metric->serialize() << "\n";
        }
        
        return oss.str();
    }
    
    std::string exportJSON() const {
        std::ostringstream oss;
        oss << "{\n  \"metrics\": [\n";
        
        auto metrics = getAllMetrics();
        for (size_t i = 0; i < metrics.size(); ++i) {
            const auto& metric = metrics[i];
            oss << "    {\n";
            oss << "      \"name\": \"" << metric->getName() << "\",\n";
            oss << "      \"description\": \"" << metric->getDescription() << "\",\n";
            oss << "      \"type\": \"" << static_cast<int>(metric->getType()) << "\"\n";
            oss << "    }";
            if (i < metrics.size() - 1) oss << ",";
            oss << "\n";
        }
        
        oss << "  ]\n}";
        return oss.str();
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_metrics) {
            pair.second->reset();
        }
    }
};

/**
 * @brief 全局指标收集器单例
 */
class GlobalMetrics {
private:
    static std::unique_ptr<MetricsCollector> s_instance;
    static std::once_flag s_initFlag;

public:
    static MetricsCollector& getInstance() {
        std::call_once(s_initFlag, []() {
            s_instance = std::make_unique<MetricsCollector>();
        });
        return *s_instance;
    }
    
    static std::shared_ptr<Counter> createCounter(const std::string& name, 
                                                 const std::string& description,
                                                 const Labels& labels = {}) {
        return getInstance().createCounter(name, description, labels);
    }
    
    static std::shared_ptr<Gauge> createGauge(const std::string& name,
                                             const std::string& description,
                                             const Labels& labels = {}) {
        return getInstance().createGauge(name, description, labels);
    }
    
    static std::shared_ptr<Timer> createTimer(const std::string& name,
                                             const std::string& description,
                                             const Labels& labels = {}) {
        return getInstance().createTimer(name, description, labels);
    }
    
    static std::string exportPrometheus() {
        return getInstance().exportPrometheus();
    }
};

} // namespace Monitoring
} // namespace NetBox

// 便利宏定义
#define NETBOX_COUNTER(name, desc) NetBox::Monitoring::GlobalMetrics::createCounter(name, desc)
#define NETBOX_GAUGE(name, desc) NetBox::Monitoring::GlobalMetrics::createGauge(name, desc)
#define NETBOX_TIMER(name, desc) NetBox::Monitoring::GlobalMetrics::createTimer(name, desc)
#define NETBOX_SCOPED_TIMER(timer) auto _scoped_timer = timer->createScopedTimer()
