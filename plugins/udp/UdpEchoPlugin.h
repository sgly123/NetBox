#ifndef UDP_ECHO_PLUGIN_H
#define UDP_ECHO_PLUGIN_H

#include "plugin/ServerPlugin.h"
#include "UdpEchoServer.h"
#include <memory>

/**
 * @brief UDP Echo服务器插件
 *        集成UDP Echo服务器到插件化框架中
 */
class UdpEchoPlugin : public ServerPlugin {
public:
    UdpEchoPlugin();
    virtual ~UdpEchoPlugin();

    // 插件基本信息
    std::string getName() const override;
    std::string getVersion() const override;
    std::string getDescription() const override;

    // 插件生命周期
    bool initialize(const std::unordered_map<std::string, std::string>& config) override;
    bool start() override;
    void stop() override;
    void cleanup() override;

    // 状态查询
    bool isRunning() const override;
    std::string getStatusInfo() const override;

    // 配置管理
    bool configure(const std::unordered_map<std::string, std::string>& config) override;
    std::unordered_map<std::string, std::string> getConfiguration() const override;

    // UDP特定方法
    void printStatistics() const;
    void cleanupInactiveClients(int timeout_seconds = 300);

private:
    std::unique_ptr<UdpEchoServer> m_udpServer;
    std::string m_ip;
    int m_port;
    IOMultiplexer::IOType m_ioType;
    bool m_initialized;
    bool m_running;

    // 统计信息定时器相关
    std::atomic<bool> m_statsEnabled;
    std::thread m_statsThread;
    int m_statsInterval; // 统计信息打印间隔（秒）

    void statsThreadFunc();
    bool parseConfig(const std::unordered_map<std::string, std::string>& config);
    IOMultiplexer::IOType parseIOType(const std::string& ioTypeStr);
};

#endif // UDP_ECHO_PLUGIN_H 