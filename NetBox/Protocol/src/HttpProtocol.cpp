#include "HttpProtocol.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <arpa/inet.h>

HttpProtocol::HttpProtocol() {
    // 设置默认流量控制
    setFlowControl(1024*1024, 1024*1024); // 1MB/s
}

size_t HttpProtocol::onDataReceived(const char* data, size_t len) {
    // 流量控制检查
    if (!checkFlowControl(len)) {
        if (errorCallback_) {
            errorCallback_("HTTP: Flow control exceeded");
        }
        return 0;
    }

    // 追加数据到缓冲区
    buffer_.insert(buffer_.end(), data, data + len);
    
    size_t processedBytes = 0;
    std::string bufferStr(reinterpret_cast<const char*>(buffer_.data()), buffer_.size());
    
    // 检查缓冲区大小限制
    if (buffer_.size() > maxRequestSize_) {
        if (errorCallback_) {
            errorCallback_("HTTP: Request too large");
        }
        reset();
        return 0;
    }

    // 解析HTTP消息
    while (!messageComplete_ && buffer_.size() > 0) {
        if (parsingHeaders_) {
            // 查找头部结束标记 "\r\n\r\n"
            size_t headerEnd = bufferStr.find("\r\n\r\n");
            if (headerEnd == std::string::npos) {
                // 头部不完整，等待更多数据
                break;
            }

            // 提取头部数据
            std::string headersData = bufferStr.substr(0, headerEnd);
            std::istringstream headerStream(headersData);
            std::string line;
            std::vector<std::string> headerLines;

            // 解析头部行
            while (std::getline(headerStream, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back(); // 移除\r
                }
                if (!line.empty()) {
                    headerLines.push_back(line);
                }
            }

            if (headerLines.empty()) {
                if (errorCallback_) {
                    errorCallback_("HTTP: Invalid headers");
                }
                reset();
                return 0;
            }

            // 解析第一行（请求行或响应行）
            if (isRequest_) {
                if (!parseRequestLine(headerLines[0], currentRequest_)) {
                    if (errorCallback_) {
                        errorCallback_("HTTP: Invalid request line");
                    }
                    reset();
                    return 0;
                }
            } else {
                if (!parseResponseLine(headerLines[0], currentResponse_)) {
                    if (errorCallback_) {
                        errorCallback_("HTTP: Invalid response line");
                    }
                    reset();
                    return 0;
                }
            }

            // 解析其他头部
            std::vector<std::string> otherHeaders(headerLines.begin() + 1, headerLines.end());
            if (!parseHeaders(otherHeaders, isRequest_ ? currentRequest_.headers : currentResponse_.headers)) {
                if (errorCallback_) {
                    errorCallback_("HTTP: Invalid headers");
                }
                reset();
                return 0;
            }

            // 检查是否需要解析体
            auto& headers = isRequest_ ? currentRequest_.headers : currentResponse_.headers;
            auto contentLengthIt = headers.find("content-length");
            auto transferEncodingIt = headers.find("transfer-encoding");

            if (contentLengthIt != headers.end()) {
                expectedBodyLength_ = std::stoul(contentLengthIt->second);
                parsingBody_ = true;
            } else if (transferEncodingIt != headers.end() && 
                      transferEncodingIt->second.find("chunked") != std::string::npos) {
                if (isRequest_) {
                    currentRequest_.chunked = true;
                } else {
                    currentResponse_.chunked = true;
                }
                parsingBody_ = true;
            } else {
                // 没有体，消息完成
                messageComplete_ = true;
            }

            parsingHeaders_ = false;
            processedBytes = headerEnd + 4; // 包含\r\n\r\n

        } else if (parsingBody_) {
            // 解析体
            std::string bodyData = bufferStr.substr(processedBytes);
            
            if (isRequest_) {
                if (currentRequest_.chunked) {
                    if (!parseChunkedBody(bodyData, currentRequest_.body)) {
                        break; // 等待更多数据
                    }
                } else {
                    if (bodyData.length() < expectedBodyLength_) {
                        break; // 等待更多数据
                    }
                    currentRequest_.body = bodyData.substr(0, expectedBodyLength_);
                }
            } else {
                if (currentResponse_.chunked) {
                    if (!parseChunkedBody(bodyData, currentResponse_.body)) {
                        break; // 等待更多数据
                    }
                } else {
                    if (bodyData.length() < expectedBodyLength_) {
                        break; // 等待更多数据
                    }
                    currentResponse_.body = bodyData.substr(0, expectedBodyLength_);
                }
            }

            messageComplete_ = true;
            processedBytes += isRequest_ ? currentRequest_.body.length() : currentResponse_.body.length();
        }
    }

    // 如果消息完成，调用回调
    if (messageComplete_) {
        // 将HTTP消息转换为字节数组传递给回调
        std::vector<char> messageData;
        if (isRequest_) {
            // 构建请求消息
            std::string requestStr = methodToString(currentRequest_.method) + " " + 
                                   currentRequest_.path + " " + 
                                   versionToString(currentRequest_.version) + "\r\n";
            requestStr += buildHeaders(currentRequest_.headers);
            requestStr += "\r\n";
            requestStr += currentRequest_.body;
            
            messageData.assign(requestStr.begin(), requestStr.end());
        } else {
            // 构建响应消息
            std::string responseStr = versionToString(currentResponse_.version) + " " + 
                                    std::to_string(static_cast<int>(currentResponse_.statusCode)) + " " + 
                                    currentResponse_.statusText + "\r\n";
            responseStr += buildHeaders(currentResponse_.headers);
            responseStr += "\r\n";
            responseStr += currentResponse_.body;
            
            messageData.assign(responseStr.begin(), responseStr.end());
        }

        if (packetCallback_) {
            packetCallback_(messageData);
        }

        // 清理已处理的数据
        buffer_.erase(buffer_.begin(), buffer_.begin() + processedBytes);
        
        // 重置状态，准备处理下一个消息
        reset();
    }

    return processedBytes;
}

bool HttpProtocol::packResponse(StatusCode statusCode, 
                               const std::map<std::string, std::string>& headers,
                               const std::string& body,
                               std::vector<char>& out) {
    // 构建响应行
    std::string response = "HTTP/1.1 " + std::to_string(static_cast<int>(statusCode)) + " ";
    
    switch (statusCode) {
        case StatusCode::OK: response += "OK"; break;
        case StatusCode::CREATED: response += "Created"; break;
        case StatusCode::NO_CONTENT: response += "No Content"; break;
        case StatusCode::BAD_REQUEST: response += "Bad Request"; break;
        case StatusCode::NOT_FOUND: response += "Not Found"; break;
        case StatusCode::INTERNAL_ERROR: response += "Internal Server Error"; break;
        case StatusCode::NOT_IMPLEMENTED: response += "Not Implemented"; break;
        default: response += "Unknown"; break;
    }
    response += "\r\n";

    // 添加头部
    response += buildHeaders(headers);
    response += "\r\n";

    // 添加体
    response += body;

    // 添加协议头
    out.resize(4 + response.size());
    
    // 添加协议ID头（大端序）
    uint32_t protocolId = htonl(ID);
    std::memcpy(out.data(), &protocolId, 4);
    
    // 添加HTTP响应数据
    std::memcpy(out.data() + 4, response.data(), response.size());
    
    return true;
}

bool HttpProtocol::pack(const char* data, size_t len, std::vector<char>& out) {
    // 流量控制检查
    if (!checkFlowControl(len)) {
        if (errorCallback_) {
            errorCallback_("HTTP: Flow control exceeded");
        }
        return false;
    }

    // HTTP协议封包：添加4字节协议头 + 原始数据
    out.resize(4 + len);
    
    // 添加协议ID头（大端序）
    uint32_t protocolId = htonl(ID);
    std::memcpy(out.data(), &protocolId, 4);
    
    // 添加原始数据
    std::memcpy(out.data() + 4, data, len);
    
    return true;
}

bool HttpProtocol::packRequest(Method method,
                              const std::string& path,
                              const std::map<std::string, std::string>& headers,
                              const std::string& body,
                              std::vector<char>& out) {
    // 构建请求行
    std::string request = methodToString(method) + " " + path + " HTTP/1.1\r\n";

    // 添加头部
    request += buildHeaders(headers);
    request += "\r\n";

    // 添加体
    request += body;

    // 添加协议头
    out.resize(4 + request.size());
    
    // 添加协议ID头（大端序）
    uint32_t protocolId = htonl(ID);
    std::memcpy(out.data(), &protocolId, 4);
    
    // 添加HTTP请求数据
    std::memcpy(out.data() + 4, request.data(), request.size());
    
    return true;
}

void HttpProtocol::reset() {
    buffer_.clear();
    currentRequest_ = HttpRequest{};
    currentResponse_ = HttpResponse{};
    messageComplete_ = false;
    parsingHeaders_ = true;
    parsingBody_ = false;
    expectedBodyLength_ = 0;
    headerLines_.clear();
    chunkedBuffer_.clear();
}

bool HttpProtocol::parseRequestLine(const std::string& line, HttpRequest& request) {
    std::istringstream iss(line);
    std::string method, path, version;
    
    if (!(iss >> method >> path >> version)) {
        return false;
    }

    request.method = stringToMethod(method);
    request.path = path;
    request.version = stringToVersion(version);
    
    return request.method != Method::UNKNOWN && request.version != Version::UNKNOWN;
}

bool HttpProtocol::parseResponseLine(const std::string& line, HttpResponse& response) {
    std::istringstream iss(line);
    std::string version, statusCodeStr, statusText;
    
    if (!(iss >> version >> statusCodeStr)) {
        return false;
    }

    response.version = stringToVersion(version);
    response.statusCode = static_cast<StatusCode>(std::stoi(statusCodeStr));
    
    // 读取状态文本（可能包含空格）
    std::getline(iss, response.statusText);
    if (!response.statusText.empty() && response.statusText[0] == ' ') {
        response.statusText = response.statusText.substr(1);
    }
    
    return response.version != Version::UNKNOWN;
}

bool HttpProtocol::parseHeaders(const std::vector<std::string>& headerLines, 
                               std::map<std::string, std::string>& headers) {
    for (const auto& line : headerLines) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            return false;
        }
        
        std::string name = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        
        // 去除首尾空格
        while (!value.empty() && std::isspace(value.front())) {
            value.erase(0, 1);
        }
        while (!value.empty() && std::isspace(value.back())) {
            value.pop_back();
        }
        
        // 转换为小写
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        
        headers[name] = value;
    }
    return true;
}

bool HttpProtocol::parseChunkedBody(const std::string& data, std::string& body) {
    // 简化的分块传输解析
    // 实际实现需要更复杂的解析逻辑
    size_t pos = 0;
    while (pos < data.length()) {
        size_t lineEnd = data.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            return false; // 等待更多数据
        }
        
        std::string chunkSizeStr = data.substr(pos, lineEnd - pos);
        size_t chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        
        if (chunkSize == 0) {
            // 最后一个块
            body += data.substr(pos + lineEnd - pos + 2);
            return true;
        }
        
        size_t chunkStart = lineEnd + 2;
        if (chunkStart + chunkSize + 2 > data.length()) {
            return false; // 等待更多数据
        }
        
        body += data.substr(chunkStart, chunkSize);
        pos = chunkStart + chunkSize + 2;
    }
    
    return false; // 等待更多数据
}

HttpProtocol::Method HttpProtocol::stringToMethod(const std::string& method) {
    if (method == "GET") return Method::GET;
    if (method == "POST") return Method::POST;
    if (method == "PUT") return Method::PUT;
    if (method == "DELETE") return Method::DELETE;
    if (method == "HEAD") return Method::HEAD;
    if (method == "OPTIONS") return Method::OPTIONS;
    if (method == "PATCH") return Method::PATCH;
    return Method::UNKNOWN;
}

std::string HttpProtocol::methodToString(Method method) {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE: return "DELETE";
        case Method::HEAD: return "HEAD";
        case Method::OPTIONS: return "OPTIONS";
        case Method::PATCH: return "PATCH";
        default: return "UNKNOWN";
    }
}

HttpProtocol::Version HttpProtocol::stringToVersion(const std::string& version) {
    if (version == "HTTP/1.0") return Version::HTTP_1_0;
    if (version == "HTTP/1.1") return Version::HTTP_1_1;
    if (version == "HTTP/2.0") return Version::HTTP_2_0;
    return Version::UNKNOWN;
}

std::string HttpProtocol::versionToString(Version version) {
    switch (version) {
        case Version::HTTP_1_0: return "HTTP/1.0";
        case Version::HTTP_1_1: return "HTTP/1.1";
        case Version::HTTP_2_0: return "HTTP/2.0";
        default: return "HTTP/1.1";
    }
}

std::string HttpProtocol::buildHeaders(const std::map<std::string, std::string>& headers) {
    std::string result;
    for (const auto& header : headers) {
        result += header.first + ": " + header.second + "\r\n";
    }
    return result;
} 