#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
高性能Echo测试客户端
用于测试通讯框架的QPS、延迟等性能指标
支持高并发连接和自定义测试参数
"""

import asyncio
import socket
import struct
import sys
import time
import statistics
from typing import List, Optional

# 协议常量（需与服务器保持一致）
PROTOCOL_ID = 1
LEN_FIELD_SIZE = 4
PROTOCOL_HEADER_SIZE = 4 + LEN_FIELD_SIZE  # 协议ID(4字节) + 长度字段(4字节)

class EchoClient:
    def __init__(self, host: str = '192.168.88.135', port: int = 8888):
        self.host = host
        self.port = port
        self.total_requests = 0
        self.total_responses = 0
        self.total_latency = 0.0
        self.latencies: List[float] = []
        self.error_count = 0
        self.test_running = False
        self.lock = asyncio.Lock()  # 异步锁保证统计数据线程安全

    def create_packet(self, message: str) -> bytes:
        """创建符合协议格式的数据包"""
        msg_bytes = message.encode('utf-8')
        msg_len = len(msg_bytes)
        
        # 协议头：协议ID(大端序4字节) + 消息长度(大端序4字节)
        header = struct.pack('>I', PROTOCOL_ID) + struct.pack('>I', msg_len)
        return header + msg_bytes

    async def handle_connection(self, message: str, duration: int):
        """处理单个连接的发送和接收逻辑"""
        reader: Optional[asyncio.StreamReader] = None
        writer: Optional[asyncio.StreamWriter] = None
        
        try:
            # 建立连接并设置缓冲区
            reader, writer = await asyncio.wait_for(
                asyncio.open_connection(self.host, self.port),
                timeout=10.0
            )
            
            sock = writer.get_extra_info('socket')
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024*1024)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024*1024)
            
            # 准备测试数据包
            packet = self.create_packet(message)
            end_time = time.time() + duration
            pending = 0  # 未完成的请求数
            
            while time.time() < end_time and self.test_running:
                # 控制未完成请求数量，避免内存溢出
                if pending > 1000:
                    await asyncio.sleep(0.001)
                    continue
                
                # 发送请求
                send_time = time.time()
                writer.write(packet)
                await writer.drain()
                
                async with self.lock:
                    self.total_requests += 1
                pending += 1
                
                # 接收响应
                try:
                    # 读取协议头
                    header = await asyncio.wait_for(
                        reader.read(PROTOCOL_HEADER_SIZE),
                        timeout=5.0
                    )
                    if not header or len(header) < PROTOCOL_HEADER_SIZE:
                        async with self.lock:
                            self.error_count += 1
                        pending -= 1
                        continue
                    
                    # 解析消息长度
                    _, body_len = struct.unpack('>II', header)
                    
                    # 读取消息体
                    body = await asyncio.wait_for(
                        reader.read(body_len),
                        timeout=5.0
                    )
                    
                    # 计算延迟并更新统计
                    latency = (time.time() - send_time) * 1000  # 转换为毫秒
                    async with self.lock:
                        self.total_responses += 1
                        self.total_latency += latency
                        self.latencies.append(latency)
                    pending -= 1
                    
                except (asyncio.TimeoutError, ConnectionResetError):
                    async with self.lock:
                        self.error_count += 1
                    pending -= 1
                    continue
        
        except Exception as e:
            async with self.lock:
                self.error_count += 1
        finally:
            if writer:
                try:
                    writer.close()
                    await writer.wait_closed()
                except:
                    pass

    async def run_test(self, concurrency: int, duration: int, message: str):
        """运行并发测试"""
        # 创建并发连接任务
        tasks = [
            self.handle_connection(message, duration)
            for _ in range(concurrency)
        ]
        
        # 等待所有任务完成
        await asyncio.gather(*tasks)

    def start_test(self, concurrency: int = 100, duration: int = 30, message: str = "test"):
        """启动性能测试"""
        print(f"📋 测试参数:")
        print(f"服务器: {self.host}:{self.port}")
        print(f"并发连接数: {concurrency}")
        print(f"测试持续时间: {duration}秒")
        print(f"消息内容: {message} (长度: {len(message)}字节)")
        print("----------------------------------------")
        print("开始测试...")
        
        self.test_running = True
        start_time = time.time()
        
        # 运行事件循环
        loop = asyncio.get_event_loop()
        try:
            loop.run_until_complete(self.run_test(concurrency, duration, message))
        finally:
            self.test_running = False
            loop.close()
        
        # 计算测试结果
        test_time = time.time() - start_time
        qps = self.total_responses / test_time if test_time > 0 else 0
        avg_latency = self.total_latency / self.total_responses if self.total_responses > 0 else 0
        
        # 计算P99延迟
        p99_latency = 0.0
        if self.latencies:
            self.latencies.sort()
            p99_index = int(len(self.latencies) * 0.99)
            p99_latency = self.latencies[p99_index] if p99_index < len(self.latencies) else self.latencies[-1]
        
        # 输出结果
        print("\n📊 性能测试结果")
        print("----------------------------------------")
        print(f"总请求数: {self.total_requests:,}")
        print(f"总响应数: {self.total_responses:,}")
        print(f"错误数: {self.error_count:,}")
        print(f"测试耗时: {test_time:.2f}秒")
        print(f"QPS: {qps:.2f}")
        print(f"平均响应延迟: {avg_latency:.2f}ms")
        print(f"P99响应延迟: {p99_latency:.2f}ms")
        print(f"成功率: {self.total_responses/self.total_requests*100:.2f}%" 
              if self.total_requests > 0 else "0%")

if __name__ == "__main__":
    # 解析命令行参数
    host = '192.168.88.135'
    port = 8888
    concurrency = 100
    duration = 30
    message = "performance_test"
    
    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    if len(sys.argv) > 3:
        concurrency = int(sys.argv[3])
    if len(sys.argv) > 4:
        duration = int(sys.argv[4])
    if len(sys.argv) > 5:
        message = sys.argv[5]
    
    client = EchoClient(host, port)
    client.start_test(concurrency, duration, message)
    