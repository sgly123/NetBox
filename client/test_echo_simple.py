#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
简单的Echo客户端测试脚本
用于测试NetBox Echo服务器的协议格式
"""

import socket
import struct
import sys

def create_echo_packet(message):
    """
    创建符合Echo服务器要求的数据包
    格式: [协议路由头4字节] + [包体长度4字节] + [实际数据]
    """
    # 将消息编码为UTF-8字节
    message_bytes = message.encode('utf-8')
    message_len = len(message_bytes)
    
    # 协议路由头: 协议ID=1 (大端序)
    protocol_header = struct.pack('>I', 1)  # 0x00000001
    
    # SimpleHeader协议包头: 包体长度 (大端序)
    body_length = struct.pack('>I', message_len)
    
    # 组装完整数据包
    packet = protocol_header + body_length + message_bytes
    
    return packet

def parse_echo_response(data):
    """
    解析Echo服务器的响应
    修复后的服务器发送纯文本回显，不使用协议封装
    """
    if len(data) == 0:
        return "没有响应数据"

    # 直接解析纯文本响应
    try:
        message = data.decode('utf-8', errors='ignore')
        return message
    except Exception as e:
        return f"解析错误: {e}"

def test_echo_server(host='192.168.88.135', port=8888):
    """
    测试Echo服务器
    """
    print(f"🚀 连接Echo服务器: {host}:{port}")
    
    try:
        # 创建socket连接
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print("✅ 连接成功")
        
        # 测试消息列表
        test_messages = [
            "Hello NetBox!",
            "你好，世界！",
            "Test Echo Server",
            "中文测试消息",
            "123456789"
        ]
        
        for i, message in enumerate(test_messages, 1):
            print(f"\n📤 测试 {i}: 发送消息: '{message}'")
            
            # 创建数据包
            packet = create_echo_packet(message)
            print(f"   数据包长度: {len(packet)} 字节")
            print(f"   数据包十六进制: {packet.hex()}")
            
            # 发送数据包
            sock.send(packet)
            
            # 接收响应
            try:
                response_data = sock.recv(4096)
                if response_data:
                    print(f"📥 接收到: {len(response_data)} 字节")
                    print(f"   原始数据: {response_data.hex()}")
                    
                    # 解析响应
                    echo_message = parse_echo_response(response_data)
                    print(f"🔄 回显消息: '{echo_message}'")
                    
                    # 验证回显
                    expected = f"Echo: {message}"
                    if echo_message == expected:
                        print("✅ 回显正确")
                    else:
                        print(f"❌ 回显错误，期望: '{expected}'")
                else:
                    print("❌ 没有收到响应")
            except socket.timeout:
                print("⏰ 接收超时")
            except Exception as e:
                print(f"❌ 接收错误: {e}")
        
        sock.close()
        print("\n👋 测试完成")
        
    except ConnectionRefusedError:
        print("❌ 连接被拒绝，请确保Echo服务器正在运行")
    except Exception as e:
        print(f"❌ 连接错误: {e}")

def interactive_mode(host='192.168.88.135', port=8888):
    """
    交互模式
    """
    print(f"🎮 Echo客户端交互模式")
    print(f"连接到: {host}:{port}")
    print("输入消息发送到服务器，输入'quit'退出")
    print("=" * 50)
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print("✅ 连接成功\n")
        
        while True:
            try:
                message = input("echo> ").strip()
                
                if message.lower() in ['quit', 'exit']:
                    print("👋 再见！")
                    break
                
                if not message:
                    continue
                
                # 发送消息
                packet = create_echo_packet(message)
                sock.send(packet)
                print(f"📤 已发送: {len(packet)} 字节")
                
                # 接收响应
                response_data = sock.recv(4096)
                if response_data:
                    echo_message = parse_echo_response(response_data)
                    print(f"🔄 回显: {echo_message}")
                else:
                    print("❌ 没有收到响应")
                    
            except KeyboardInterrupt:
                print("\n👋 用户中断，退出")
                break
            except Exception as e:
                print(f"❌ 错误: {e}")
                break
        
        sock.close()
        
    except Exception as e:
        print(f"❌ 连接错误: {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "-i":
        # 交互模式
        interactive_mode()
    else:
        # 自动测试模式
        test_echo_server()
