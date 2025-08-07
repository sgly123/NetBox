#!/usr/bin/env python3
"""
NetBox AI Assistant
为NetBox脚手架提供AI驱动的智能助手功能
"""

import json
import re
from typing import Dict, List, Optional, Tuple
from pathlib import Path

class NetBoxAIAssistant:
    """NetBox AI助手"""
    
    def __init__(self):
        self.knowledge_base = self._load_knowledge_base()
        self.templates = self._load_templates()
    
    def _load_knowledge_base(self) -> Dict:
        """加载知识库"""
        return {
            "protocols": {
                "http": {
                    "description": "HTTP协议支持",
                    "use_cases": ["Web服务器", "RESTful API", "微服务"],
                    "performance": "中等",
                    "complexity": "简单"
                },
                "websocket": {
                    "description": "WebSocket实时通信",
                    "use_cases": ["聊天应用", "实时游戏", "推送服务"],
                    "performance": "高",
                    "complexity": "中等"
                },
                "tcp": {
                    "description": "原生TCP协议",
                    "use_cases": ["高性能服务器", "自定义协议", "游戏服务器"],
                    "performance": "最高",
                    "complexity": "复杂"
                }
            },
            "applications": {
                "web_server": {
                    "description": "Web服务器应用",
                    "recommended_protocols": ["http", "websocket"],
                    "typical_features": ["路由", "中间件", "静态文件服务"]
                },
                "game_server": {
                    "description": "游戏服务器应用",
                    "recommended_protocols": ["tcp", "websocket"],
                    "typical_features": ["玩家管理", "房间系统", "实时同步"]
                },
                "chat_server": {
                    "description": "聊天服务器应用",
                    "recommended_protocols": ["websocket", "tcp"],
                    "typical_features": ["消息路由", "用户认证", "群组管理"]
                }
            },
            "optimizations": {
                "high_concurrency": [
                    "使用EPOLL/KQUEUE IO多路复用",
                    "启用SO_REUSEPORT",
                    "调整TCP缓冲区大小",
                    "使用线程池"
                ],
                "low_latency": [
                    "启用TCP_NODELAY",
                    "减少系统调用",
                    "使用零拷贝技术",
                    "优化内存分配"
                ],
                "memory_efficiency": [
                    "使用对象池",
                    "避免频繁内存分配",
                    "使用move语义",
                    "及时释放资源"
                ]
            }
        }
    
    def _load_templates(self) -> Dict:
        """加载代码模板"""
        return {
            "high_performance_server": """
class HighPerformanceServer : public NetBox::Application::Application {
private:
    std::unique_ptr<NetBox::Core::ThreadPool> m_threadPool;
    std::shared_ptr<NetBox::Network::Optimizer> m_optimizer;
    
public:
    HighPerformanceServer() : Application("HighPerformanceServer") {
        // 创建线程池
        m_threadPool = std::make_unique<NetBox::Core::ThreadPool>(
            std::thread::hardware_concurrency()
        );
        
        // 创建网络优化器
        m_optimizer = std::make_shared<NetBox::Network::HighPerformanceOptimizer>();
    }
    
    bool initialize() override {
        // 高性能配置
        setConfig("tcp.nodelay", "true");
        setConfig("so.reuseport", "true");
        setConfig("buffer.size", "65536");
        
        return true;
    }
};""",
            "websocket_chat": """
class WebSocketChatServer : public NetBox::Application::Application {
private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<NetBox::Application::Context>>> m_rooms;
    std::mutex m_roomsMutex;
    
public:
    void onWebSocketMessage(std::shared_ptr<NetBox::Application::Context> ctx, 
                           const std::string& message) {
        // 解析消息
        auto jsonMsg = parseJSON(message);
        std::string roomId = jsonMsg["room"];
        std::string content = jsonMsg["content"];
        
        // 广播到房间
        broadcastToRoom(roomId, content, ctx);
    }
    
private:
    void broadcastToRoom(const std::string& roomId, const std::string& message,
                        std::shared_ptr<NetBox::Application::Context> sender) {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        
        if (m_rooms.find(roomId) != m_rooms.end()) {
            for (auto& client : m_rooms[roomId]) {
                if (client != sender) {
                    client->send(message);
                }
            }
        }
    }
};"""
        }
    
    def analyze_requirements(self, user_input: str) -> Dict:
        """分析用户需求"""
        analysis = {
            "project_type": "unknown",
            "protocols": [],
            "features": [],
            "performance_requirements": [],
            "recommendations": []
        }
        
        user_input_lower = user_input.lower()
        
        # 分析项目类型
        if any(word in user_input_lower for word in ["web", "http", "api", "rest"]):
            analysis["project_type"] = "web_server"
            analysis["protocols"].append("http")
        elif any(word in user_input_lower for word in ["game", "游戏", "实时"]):
            analysis["project_type"] = "game_server"
            analysis["protocols"].extend(["tcp", "websocket"])
        elif any(word in user_input_lower for word in ["chat", "聊天", "消息"]):
            analysis["project_type"] = "chat_server"
            analysis["protocols"].append("websocket")
        
        # 分析性能需求
        if any(word in user_input_lower for word in ["高并发", "high concurrency", "大量用户"]):
            analysis["performance_requirements"].append("high_concurrency")
        if any(word in user_input_lower for word in ["低延迟", "low latency", "实时"]):
            analysis["performance_requirements"].append("low_latency")
        if any(word in user_input_lower for word in ["内存", "memory", "资源"]):
            analysis["performance_requirements"].append("memory_efficiency")
        
        # 生成建议
        analysis["recommendations"] = self._generate_recommendations(analysis)
        
        return analysis
    
    def _generate_recommendations(self, analysis: Dict) -> List[str]:
        """生成建议"""
        recommendations = []
        
        # 基于项目类型的建议
        if analysis["project_type"] in self.knowledge_base["applications"]:
            app_info = self.knowledge_base["applications"][analysis["project_type"]]
            recommendations.append(f"建议创建{app_info['description']}")
            
            for protocol in app_info["recommended_protocols"]:
                if protocol not in analysis["protocols"]:
                    recommendations.append(f"考虑添加{protocol}协议支持")
        
        # 基于性能需求的建议
        for req in analysis["performance_requirements"]:
            if req in self.knowledge_base["optimizations"]:
                optimizations = self.knowledge_base["optimizations"][req]
                recommendations.extend([f"性能优化: {opt}" for opt in optimizations[:2]])
        
        return recommendations
    
    def generate_project_config(self, analysis: Dict, project_name: str) -> Dict:
        """生成项目配置"""
        config = {
            "project_name": project_name,
            "project_type": analysis["project_type"],
            "protocols": analysis["protocols"],
            "features": [],
            "optimizations": [],
            "template_vars": {}
        }
        
        # 基于分析结果添加特性
        if "high_concurrency" in analysis["performance_requirements"]:
            config["features"].extend(["thread_pool", "connection_pool"])
            config["optimizations"].extend(["epoll", "so_reuseport"])
        
        if "low_latency" in analysis["performance_requirements"]:
            config["optimizations"].extend(["tcp_nodelay", "zero_copy"])
        
        # 设置模板变量
        config["template_vars"] = {
            "enable_high_performance": "high_concurrency" in analysis["performance_requirements"],
            "enable_websocket": "websocket" in analysis["protocols"],
            "enable_threading": len(analysis["performance_requirements"]) > 0
        }
        
        return config
    
    def suggest_code_template(self, project_type: str) -> Optional[str]:
        """建议代码模板"""
        template_map = {
            "game_server": "high_performance_server",
            "chat_server": "websocket_chat",
            "web_server": "high_performance_server"
        }
        
        template_key = template_map.get(project_type)
        return self.templates.get(template_key) if template_key else None
    
    def diagnose_error(self, error_message: str) -> Dict:
        """诊断错误"""
        diagnosis = {
            "error_type": "unknown",
            "possible_causes": [],
            "solutions": [],
            "confidence": 0.0
        }
        
        error_lower = error_message.lower()
        
        # 编译错误诊断
        if "undefined reference" in error_lower:
            diagnosis["error_type"] = "linking_error"
            diagnosis["possible_causes"] = ["缺少库文件", "链接顺序错误"]
            diagnosis["solutions"] = [
                "检查CMakeLists.txt中的target_link_libraries",
                "确保所有依赖库都已安装"
            ]
            diagnosis["confidence"] = 0.8
        
        elif "no such file" in error_lower:
            diagnosis["error_type"] = "missing_file"
            diagnosis["possible_causes"] = ["头文件路径错误", "文件不存在"]
            diagnosis["solutions"] = [
                "检查include路径",
                "确保所有头文件都存在"
            ]
            diagnosis["confidence"] = 0.9
        
        elif "permission denied" in error_lower:
            diagnosis["error_type"] = "permission_error"
            diagnosis["possible_causes"] = ["文件权限不足", "目录权限问题"]
            diagnosis["solutions"] = [
                "使用chmod +x设置执行权限",
                "检查目录写权限"
            ]
            diagnosis["confidence"] = 0.95
        
        return diagnosis

def main():
    """测试AI助手"""
    ai = NetBoxAIAssistant()
    
    # 测试需求分析
    test_inputs = [
        "我想创建一个高并发的游戏服务器",
        "需要一个支持WebSocket的聊天应用",
        "构建一个低延迟的Web API服务"
    ]
    
    for user_input in test_inputs:
        print(f"\n用户输入: {user_input}")
        analysis = ai.analyze_requirements(user_input)
        print(f"分析结果: {json.dumps(analysis, indent=2, ensure_ascii=False)}")
        
        config = ai.generate_project_config(analysis, "TestProject")
        print(f"项目配置: {json.dumps(config, indent=2, ensure_ascii=False)}")

if __name__ == "__main__":
    main()
