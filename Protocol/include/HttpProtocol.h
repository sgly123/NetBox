#pragma once
#include "ProtocolBase.h"
#include <string>
#include <map>
#include <vector>
#include <cstdint>

/**
 * @brief HTTP协议实现类
 * 
 * 协议特点：
 * 1. 文本协议：基于ASCII文本，易于调试
 * 2. 请求-响应模式：客户端请求，服务器响应
 * 3. 无状态：每个请求独立处理
 * 4. 可扩展：支持自定义头部和扩展
 * 
 * 支持的功能：
 * - HTTP/1.1协议解析
 * - 请求方法：GET, POST, PUT, DELETE等
 * - 响应状态码：200, 404, 500等
 * - 头部解析：Content-Length, Content-Type等
 * - 分块传输：支持Transfer-Encoding: chunked
 * - 连接管理：支持Keep-Alive
 * 
 * 使用场景：
 * - Web服务器：提供HTTP API服务
 * - 代理服务器：HTTP代理功能
 * - 网关服务：API网关和路由
 * - 监控接口：提供系统监控API
 */
class HttpProtocol : public ProtocolBase {
public:
    static constexpr uint32_t ID = 2;
    
    // HTTP方法枚举
    enum class Method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        PATCH,
        UNKNOWN
    };
    
    // HTTP状态码
    enum class StatusCode {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        INTERNAL_ERROR = 500,
        NOT_IMPLEMENTED = 501
    };
    
    // HTTP版本
    enum class Version {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0,
        UNKNOWN
    };

    /**
     * @brief HTTP请求结构
     */
    struct HttpRequest {
        Method method = Method::UNKNOWN;
        std::string path;
        Version version = Version::UNKNOWN;
        std::map<std::string, std::string> headers;
        std::string body;
        size_t contentLength = 0;
        bool chunked = false;
    };

    /**
     * @brief HTTP响应结构
     */
    struct HttpResponse {
        Version version = Version::UNKNOWN;
        StatusCode statusCode = StatusCode::OK;
        std::string statusText;
        std::map<std::string, std::string> headers;
        std::string body;
        size_t contentLength = 0;
        bool chunked = false;
    };

    /**
     * @brief 构造函数
     */
    HttpProtocol();
    
    /**
     * @brief 析构函数
     */
    virtual ~HttpProtocol() = default;

    /**
     * @brief 处理接收到的HTTP数据
     * @param data 原始数据
     * @param len 数据长度
     * @return 处理的数据长度
     */
    size_t onDataReceived(const char* data, size_t len) override;

    /**
     * @brief 封包要发送的数据
     * @param data 要发送的数据
     * @param len 数据长度
     * @param out 封包后的数据
     * @return 封包是否成功
     */
    bool pack(const char* data, size_t len, std::vector<char>& out) override;

    /**
     * @brief 封包HTTP响应数据
     * @param statusCode 状态码
     * @param headers 响应头部
     * @param body 响应体
     * @param out 输出缓冲区
     * @return 封包是否成功
     */
    bool packResponse(StatusCode statusCode, 
                     const std::map<std::string, std::string>& headers,
                     const std::string& body,
                     std::vector<char>& out);

    /**
     * @brief 封包HTTP请求数据
     * @param method 请求方法
     * @param path 请求路径
     * @param headers 请求头部
     * @param body 请求体
     * @param out 输出缓冲区
     * @return 封包是否成功
     */
    bool packRequest(Method method,
                    const std::string& path,
                    const std::map<std::string, std::string>& headers,
                    const std::string& body,
                    std::vector<char>& out);

    /**
     * @brief 获取协议类型
     */
    std::string getType() const override { return "HTTP"; }

    /**
     * @brief 获取协议ID
     */
    uint32_t getProtocolId() const override { return ID; }

    /**
     * @brief 重置协议状态
     */
    void reset() override;

    /**
     * @brief 设置最大请求大小
     */
    void setMaxRequestSize(size_t size) { maxRequestSize_ = size; }

    /**
     * @brief 获取当前解析的请求信息
     */
    const HttpRequest& getCurrentRequest() const { return currentRequest_; }

    /**
     * @brief 获取当前解析的响应信息
     */
    const HttpResponse& getCurrentResponse() const { return currentResponse_; }

    /**
     * @brief 检查是否为完整的HTTP消息
     */
    bool isComplete() const { return messageComplete_; }

    /**
     * @brief 获取当前缓冲区大小
     * @return 缓冲区中未处理的数据大小
     */
    size_t getBufferSize() const { return buffer_.size(); }

private:
    /**
     * @brief 解析HTTP请求行
     */
    bool parseRequestLine(const std::string& line, HttpRequest& request);

    /**
     * @brief 解析HTTP响应行
     */
    bool parseResponseLine(const std::string& line, HttpResponse& response);

    /**
     * @brief 解析HTTP头部
     */
    bool parseHeaders(const std::vector<std::string>& headerLines, 
                     std::map<std::string, std::string>& headers);

    /**
     * @brief 解析HTTP体
     */
    bool parseBody(const std::string& data, HttpRequest& request);
    bool parseBody(const std::string& data, HttpResponse& response);

    /**
     * @brief 解析分块传输编码
     */
    bool parseChunkedBody(const std::string& data, std::string& body);

    /**
     * @brief 将方法字符串转换为枚举
     */
    Method stringToMethod(const std::string& method);

    /**
     * @brief 将方法枚举转换为字符串
     */
    std::string methodToString(Method method);

    /**
     * @brief 将版本字符串转换为枚举
     */
    Version stringToVersion(const std::string& version);

    /**
     * @brief 将版本枚举转换为字符串
     */
    std::string versionToString(Version version);

    /**
     * @brief 将状态码转换为字符串
     */
    std::string statusCodeToString(StatusCode code);

    /**
     * @brief 构建HTTP头部字符串
     */
    std::string buildHeaders(const std::map<std::string, std::string>& headers);

    std::vector<char> buffer_;         // 接收缓冲区
    HttpRequest currentRequest_;       // 当前解析的请求
    HttpResponse currentResponse_;     // 当前解析的响应
    bool isRequest_ = true;           // 是否为请求（true为请求，false为响应）
    bool messageComplete_ = false;    // 消息是否完整
    size_t maxRequestSize_ = 1024*1024; // 最大请求大小（1MB）
    std::vector<std::string> headerLines_; // 临时存储头部行
    bool parsingHeaders_ = true;      // 是否正在解析头部
    bool parsingBody_ = false;        // 是否正在解析体
    size_t expectedBodyLength_ = 0;   // 期望的体长度
    std::string chunkedBuffer_;       // 分块传输缓冲区
}; 