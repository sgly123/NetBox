/**
 * @file WebServer.cpp
 * @brief NetBox Web服务器实现
 */

#include "WebServer.h"
#include "logging/Logger.h"
#include "config/ConfigManager.h"
#include "monitoring/MetricsCollector.h"
#include <sstream>
#include <fstream>
#include <regex>

WebServer::WebServer() {
    // 初始化监控指标
    m_requestCounter = NETBOX_COUNTER("http_requests_total", "HTTP请求总数");
    m_responseTimer = NETBOX_TIMER("http_response_duration", "HTTP响应时间");
    m_activeConnections = NETBOX_GAUGE("http_active_connections", "活跃连接数");
}

WebServer::~WebServer() {
    stop();
}

void WebServer::get(const std::string& path, RouteHandler handler) {
    m_routes["GET " + path] = handler;
}

void WebServer::post(const std::string& path, RouteHandler handler) {
    m_routes["POST " + path] = handler;
}

void WebServer::put(const std::string& path, RouteHandler handler) {
    m_routes["PUT " + path] = handler;
}

void WebServer::del(const std::string& path, RouteHandler handler) {
    m_routes["DELETE " + path] = handler;
}

void WebServer::use(Middleware middleware) {
    m_middlewares.push_back(middleware);
}

bool WebServer::start(const std::string& host, int port) {
    // 设置连接处理器
    m_server.setConnectionHandler([this](std::shared_ptr<NetBox::TcpConnection> conn) {
        handleConnection(conn);
    });
    
    return m_server.start(host, port);
}

void WebServer::run() {
    m_server.run();
}

void WebServer::stop() {
    m_server.stop();
}

void WebServer::setupRoutes() {
    // 根路由
    get("/", [](const HttpRequest& req, HttpResponse& res) {
        res.html(R"(
<!DOCTYPE html>
<html>
<head>
    <title>NetBox Web服务器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { color: #333; border-bottom: 2px solid #007acc; padding-bottom: 10px; }
        .info { background: #f0f8ff; padding: 20px; border-radius: 5px; margin: 20px 0; }
        .api-list { background: #f9f9f9; padding: 15px; border-radius: 5px; }
    </style>
</head>
<body>
    <h1 class="header">🌐 NetBox Web服务器</h1>
    <div class="info">
        <h2>欢迎使用NetBox Web服务器!</h2>
        <p>这是一个基于NetBox跨平台网络框架构建的高性能Web服务器。</p>
    </div>
    
    <div class="api-list">
        <h3>📋 可用的API端点:</h3>
        <ul>
            <li><strong>GET /</strong> - 首页</li>
            <li><strong>GET /api/hello</strong> - Hello API</li>
            <li><strong>GET /api/users</strong> - 用户列表</li>
            <li><strong>POST /api/users</strong> - 创建用户</li>
            <li><strong>GET /health</strong> - 健康检查</li>
            <li><strong>GET /metrics</strong> - 监控指标</li>
            <li><strong>GET /docs</strong> - API文档</li>
        </ul>
    </div>
    
    <div class="info">
        <h3>🚀 特性:</h3>
        <ul>
            <li>跨平台支持 (Windows, Linux, macOS)</li>
            <li>高性能IO多路复用</li>
            <li>RESTful API支持</li>
            <li>中间件系统</li>
            <li>监控和日志</li>
            <li>静态文件服务</li>
        </ul>
    </div>
</body>
</html>
        )");
    });
    
    // API路由
    get("/api/hello", [](const HttpRequest& req, HttpResponse& res) {
        res.json(R"({"message": "Hello from NetBox Web Server!", "timestamp": ")" + 
                 std::to_string(std::time(nullptr)) + R"("})");
    });
    
    get("/api/users", [](const HttpRequest& req, HttpResponse& res) {
        res.json(R"([
            {"id": 1, "name": "Alice", "email": "alice@example.com"},
            {"id": 2, "name": "Bob", "email": "bob@example.com"},
            {"id": 3, "name": "Charlie", "email": "charlie@example.com"}
        ])");
    });
    
    post("/api/users", [](const HttpRequest& req, HttpResponse& res) {
        // 简单的用户创建示例
        res.statusCode = 201;
        res.json(R"({"id": 4, "message": "User created successfully"})");
    });
    
    // 健康检查
    get("/health", [](const HttpRequest& req, HttpResponse& res) {
        res.json(R"({
            "status": "healthy",
            "timestamp": ")" + std::to_string(std::time(nullptr)) + R"(",
            "uptime": ")" + std::to_string(std::time(nullptr)) + R"(",
            "version": "1.0.0"
        })");
    });
    
    // 监控指标
    get("/metrics", [](const HttpRequest& req, HttpResponse& res) {
        res.setContentType("text/plain");
        res.body = NetBox::Monitoring::GlobalMetrics::exportPrometheus();
    });
    
    // API文档
    get("/docs", [](const HttpRequest& req, HttpResponse& res) {
        res.html(R"(
<!DOCTYPE html>
<html>
<head>
    <title>API文档 - NetBox Web服务器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .endpoint { background: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .method { color: white; padding: 5px 10px; border-radius: 3px; font-weight: bold; }
        .get { background: #28a745; }
        .post { background: #007bff; }
        .put { background: #ffc107; color: black; }
        .delete { background: #dc3545; }
    </style>
</head>
<body>
    <h1>📖 API文档</h1>
    
    <div class="endpoint">
        <span class="method get">GET</span> <strong>/api/hello</strong>
        <p>返回欢迎消息</p>
        <pre>{"message": "Hello from NetBox Web Server!", "timestamp": "1234567890"}</pre>
    </div>
    
    <div class="endpoint">
        <span class="method get">GET</span> <strong>/api/users</strong>
        <p>获取用户列表</p>
        <pre>[{"id": 1, "name": "Alice", "email": "alice@example.com"}]</pre>
    </div>
    
    <div class="endpoint">
        <span class="method post">POST</span> <strong>/api/users</strong>
        <p>创建新用户</p>
        <pre>{"id": 4, "message": "User created successfully"}</pre>
    </div>
    
    <div class="endpoint">
        <span class="method get">GET</span> <strong>/health</strong>
        <p>健康检查</p>
        <pre>{"status": "healthy", "timestamp": "1234567890"}</pre>
    </div>
    
    <div class="endpoint">
        <span class="method get">GET</span> <strong>/metrics</strong>
        <p>Prometheus格式的监控指标</p>
    </div>
</body>
</html>
        )");
    });
}

void WebServer::setupMiddleware() {
    // CORS中间件
    use([](const HttpRequest& req, HttpResponse& res) -> bool {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        if (req.method == "OPTIONS") {
            res.statusCode = 200;
            return false; // 直接返回，不继续处理
        }
        
        return true; // 继续处理
    });
    
    // 日志中间件
    use([](const HttpRequest& req, HttpResponse& res) -> bool {
        NETBOX_LOG_INFO("HTTP " + req.method + " " + req.path + " from " + req.getHeader("User-Agent"));
        return true;
    });
    
    // 请求计数中间件
    use([this](const HttpRequest& req, HttpResponse& res) -> bool {
        m_requestCounter->increment();
        return true;
    });
}

void WebServer::handleConnection(std::shared_ptr<NetBox::TcpConnection> conn) {
    m_activeConnections->increment();
    
    conn->setMessageHandler([this](std::shared_ptr<NetBox::TcpConnection> conn, const std::string& data) {
        auto timer = m_responseTimer->createScopedTimer();
        
        // 解析HTTP请求
        HttpRequest request = parseRequest(data);
        HttpResponse response;
        
        // 运行中间件
        if (runMiddlewares(request, response)) {
            // 处理路由
            handleRoute(request, response);
        }
        
        // 发送响应
        std::string responseData = buildResponse(response);
        conn->send(responseData);
        
        // HTTP/1.0 默认关闭连接
        if (request.version == "HTTP/1.0" || request.getHeader("Connection") == "close") {
            conn->close();
        }
    });
    
    conn->setCloseHandler([this](std::shared_ptr<NetBox::TcpConnection> conn) {
        m_activeConnections->decrement();
    });
}

HttpRequest WebServer::parseRequest(const std::string& data) {
    HttpRequest request;
    std::istringstream stream(data);
    std::string line;
    
    // 解析请求行
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> request.method >> request.path >> request.version;
    }
    
    // 解析头部
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // 去除空格
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            request.headers[name] = value;
        }
    }
    
    // 解析请求体
    std::string body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    request.body = body;
    
    return request;
}

std::string WebServer::buildResponse(const HttpResponse& response) {
    std::ostringstream stream;
    
    // 状态行
    stream << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
    
    // 头部
    for (const auto& header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    // Content-Length
    stream << "Content-Length: " << response.body.length() << "\r\n";
    
    // 空行
    stream << "\r\n";
    
    // 响应体
    stream << response.body;
    
    return stream.str();
}

void WebServer::handleRoute(const HttpRequest& request, HttpResponse& response) {
    std::string routeKey = request.method + " " + request.path;
    
    auto it = m_routes.find(routeKey);
    if (it != m_routes.end()) {
        it->second(request, response);
    } else {
        // 404 Not Found
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.html("<h1>404 Not Found</h1><p>The requested resource was not found.</p>");
    }
}

bool WebServer::runMiddlewares(const HttpRequest& request, HttpResponse& response) {
    for (const auto& middleware : m_middlewares) {
        if (!middleware(request, response)) {
            return false; // 中间件阻止继续处理
        }
    }
    return true;
}
