#include "server.h"
#include "base/Logger.h"
#include "base/ThreadPool.h"
#include "HttpProtocol.h"
#include <cstring>
#include <unistd.h>
#include <memory>
#include <sstream>
#include <ctime>

/**
 * @brief EchoServer 构造函数
 * 1. 注册 SimpleHeader 协议到协议分发器（ProtocolRouter）
 * 2. 设置协议层和分发器的回调，打通网络层->协议层->业务层的数据流
 */
EchoServer::EchoServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool)
    : ApplicationServer(ip, port, io_type, pool) {
    
    // 创建并注册 SimpleHeader 协议对象
    auto simpleProto = std::make_shared<SimpleHeaderProtocol>();
    // 设置协议层的包回调，收到完整包时调用业务处理
    simpleProto->setPacketCallback([this](const std::vector<char>& packet) {
        this->onPacketReceived(packet);
    });
    // 协议层错误回调
    simpleProto->setErrorCallback([](const std::string& error) {
        Logger::error("SimpleHeader协议错误: " + error);
    });
    // 设置流量控制和最大包大小
    simpleProto->setFlowControl(1024, 1024);
    simpleProto->setMaxPacketSize(1024);
    // 注册协议到分发器，协议ID由协议对象决定
    m_router.registerProtocol(simpleProto->getProtocolId(), simpleProto);
    Logger::info("注册SimpleHeader协议，ID: " + std::to_string(simpleProto->getProtocolId()));
    

    
    // 创建并注册 HTTP 协议对象
    auto httpProto = std::make_shared<HttpProtocol>();
    // 设置协议层的包回调，收到完整包时调用业务处理
    httpProto->setPacketCallback([this](const std::vector<char>& packet) {
        this->onHttpPacketReceived(packet);
    });
    // 协议层错误回调
    httpProto->setErrorCallback([](const std::string& error) {
        Logger::error("HTTP协议错误: " + error);
    });
    // 设置流量控制和最大包大小
    httpProto->setFlowControl(1024*1024, 1024*1024); // 1MB/s
    httpProto->setMaxRequestSize(1024*1024); // 1MB
    // 注册协议到分发器，协议ID由协议对象决定
    m_router.registerProtocol(httpProto->getProtocolId(), httpProto);
    Logger::info("注册HTTP协议，ID: " + std::to_string(httpProto->getProtocolId()));
    
    // 分发器错误回调
    m_router.setErrorCallback([](const std::string& error) {
        Logger::error("分发器错误: " + error);
    });
    // 分发器包回调，收到完整包时调用业务处理
    m_router.setPacketCallback([this](uint32_t protoId, const std::vector<char>& packet) {
        Logger::info("协议分发器收到数据包，协议ID: " + std::to_string(protoId) + ", 长度: " + std::to_string(packet.size()));
        if (protoId == 1) {
            this->onPacketReceived(packet);
        } else if (protoId == 2) {
            this->onHttpPacketReceived(packet);
        } else {
            Logger::warn("未知协议ID: " + std::to_string(protoId));
        }
    });
}

EchoServer::~EchoServer() {
    stop();
}

/**
 * @brief 处理客户端读事件（网络层入口）
 * 1. 从 client_fd 读取原始数据
 * 2. 调用协议分发器进行协议识别和分发
 * 3. 记录当前处理的 client_fd，便于业务层回显
 */
void EchoServer::handleRead(int client_fd) {
    char buffer[4096];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        Logger::info("收到客户端" + std::to_string(client_fd) + "的数据，长度: " + std::to_string(bytes_received));
        // 记录当前处理的客户端fd，供回显使用
        m_currentClientFd = client_fd;
        // 网络层->协议层：协议分发器处理数据，自动识别协议ID并分发
        size_t processed = m_router.onDataReceived(client_fd, buffer, bytes_received);
        Logger::info("协议分发器处理了 " + std::to_string(processed) + " 字节");
        if (processed == 0 && bytes_received > 0) {
            Logger::warn("协议分发器未处理任何数据，可能数据不完整");
        }
        // 清除当前客户端fd
        m_currentClientFd = -1;
    } else if (bytes_received == 0) {
        Logger::info("客户端" + std::to_string(client_fd) + "断开连接");
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Logger::error("接收数据错误");
        }
    }
}

/**
 * @brief 业务层回调：收到完整协议包时被调用
 * 1. 记录日志
 * 2. 构造回显数据包（协议ID+协议包）
 * 3. 通过 send() 发送回客户端，实现 echo 功能
 * @note 依赖 m_currentClientFd 记录当前处理的客户端
 */
void EchoServer::onPacketReceived(const std::vector<char>& packet) {
    // 业务层处理完整的数据包
    std::string message(packet.begin(), packet.end());
    Logger::info("收到业务数据: '" + message + "' (长度: " + std::to_string(packet.size()) + ")");

    // 防止空数据包造成循环
    if (packet.empty()) {
        Logger::warn("收到空数据包，跳过处理");
        return;
    }

    // 回显逻辑：将接收到的数据发送回客户端
    if (m_currentClientFd > 0) {
        // 添加"Echo: "前缀
        std::string echoMessage = "Echo: " + message;
        Logger::info("准备回显消息: '" + echoMessage + "'");

        // 直接发送纯文本回显，不使用协议封装（避免循环解析）
        ssize_t sent = send(m_currentClientFd, echoMessage.c_str(), echoMessage.length(), 0);
        if (sent > 0) {
            Logger::info("回显数据已发送: '" + echoMessage + "', 发送长度: " + std::to_string(sent));
        } else {
            Logger::error("发送回显数据失败");
        }
    } else {
        Logger::warn("无法确定客户端fd，跳过回显");
    }
}

/**
 * @brief HTTP协议回调：处理完整的HTTP数据包
 * @param packet 完整的HTTP数据包
 * 
 * 业务逻辑：
 * 1. 解析HTTP请求内容
 * 2. 构造HTTP响应
 * 3. 使用HTTP协议封包
 * 4. 发送回客户端
 */
void EchoServer::onHttpPacketReceived(const std::vector<char>& packet) {
    // 解析HTTP请求
    std::string httpRequest(packet.begin(), packet.end());
    Logger::info("收到HTTP请求: " + httpRequest.substr(0, std::min(httpRequest.length(), size_t(200))) + "...");
    
    // 简单的HTTP响应处理
    if (m_currentClientFd > 0) {
        // 构造HTTP响应
        auto httpProto = std::make_shared<HttpProtocol>();
        std::vector<char> httpResponse;
        
        // 解析请求行
        std::istringstream requestStream(httpRequest);
        std::string method, path, version;
        if (requestStream >> method >> path >> version) {
            Logger::info("HTTP请求解析: 方法=" + method + ", 路径=" + path + ", 版本=" + version);
            
            // 构造响应头部
            std::map<std::string, std::string> headers;
            headers["Content-Type"] = "text/html; charset=utf-8";
            headers["Server"] = "NetBox/1.0";
            headers["Connection"] = "close";
            
            // 构造响应体
            std::string body = "<html><head><title>NetBox HTTP Server</title></head>"
                              "<body><h1>Welcome to NetBox!</h1>"
                              "<p>Request Method: " + method + "</p>"
                              "<p>Request Path: " + path + "</p>"
                              "<p>Request Version: " + version + "</p>"
                              "<p>Time: " + std::to_string(std::time(nullptr)) + "</p>"
                              "</body></html>";
            
            // 构造纯HTTP响应（不包含协议头）
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html; charset=utf-8\r\n";
            response += "Server: NetBox/1.0\r\n";
            response += "Connection: close\r\n";
            response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
            response += "\r\n";
            response += body;
            
            Logger::info("HTTP响应构造成功，长度: " + std::to_string(response.size()));
            
            // 发送纯HTTP响应
            ssize_t sent = send(m_currentClientFd, response.data(), response.size(), 0);
            if (sent > 0) {
                Logger::info("HTTP响应已发送，长度: " + std::to_string(sent));
            } else {
                Logger::error("发送HTTP响应失败");
            }
        } else {
            Logger::error("HTTP请求解析失败");
            
            // 发送400错误响应
            std::map<std::string, std::string> errorHeaders;
            errorHeaders["Content-Type"] = "text/plain";
            errorHeaders["Server"] = "NetBox/1.0";
            
            std::string errorBody = "400 Bad Request - Invalid HTTP request format";
            std::vector<char> errorResponse;
            
            if (httpProto->packResponse(HttpProtocol::StatusCode::BAD_REQUEST, errorHeaders, errorBody, errorResponse)) {
                send(m_currentClientFd, errorResponse.data(), errorResponse.size(), 0);
            }
        }
    } else {
        Logger::warn("无法确定客户端fd，跳过HTTP响应");
    }
}

// ApplicationServer纯虚函数实现
void EchoServer::initializeProtocolRouter() {
    // 这里可以注册协议，实际逻辑已在构造函数实现
}

std::string EchoServer::handleHttpRequest(const std::string& request, int clientFd) {
    (void)request;  // 避免未使用参数警告
    (void)clientFd; // 避免未使用参数警告
    // EchoServer暂不处理HTTP业务
    return "";
}

std::string EchoServer::handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) {
    (void)command; // 避免未使用参数警告
    (void)args;    // 避免未使用参数警告
    // EchoServer暂不处理通用业务命令
    return "";
}

bool EchoServer::parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) {
    (void)path;     // 避免未使用参数警告
    (void)command;  // 避免未使用参数警告
    (void)args;     // 避免未使用参数警告
    // EchoServer暂不解析业务路径
    return false;
}

