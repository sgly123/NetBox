#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import socket
import threading
import subprocess
import time
from datetime import datetime
import sys

class BenchmarkResult:
    """性能测试结果统计类"""
    def __init__(self):
        self.total = 0
        self.success = 0
        self.failure = 0
        self.timeouts = 0
        self.total_delay = 0.0  # 总延迟（连接+发送+接收）
        self.lock = threading.Lock()  # 线程安全锁

    def add_success(self, delay):
        with self.lock:
            self.success += 1
            self.total_delay += delay

    def add_failure(self):
        with self.lock:
            self.failure += 1

    def add_timeout(self):
        with self.lock:
            self.timeouts += 1

    def __str__(self):
        avg_delay = self.total_delay / self.success if self.success > 0 else 0
        return (f"测试结果：\n"
                f"总连接数：{self.total}\n"
                f"成功：{self.success}（{self.success/self.total*100:.2f}%）\n"
                f"失败：{self.failure}（{self.failure/self.total*100:.2f}%）\n"
                f"超时：{self.timeouts}（{self.timeouts/self.total*100:.2f}%）\n"
                f"平均延迟：{avg_delay:.4f}秒")

def check_server(port):
    """检查服务器端口是否监听（兼容Linux）"""
    try:
        # 使用ss命令（比netstat更高效）
        result = subprocess.run(
            f"ss -tlnp | grep ':{port} '",  # 加空格避免匹配端口前缀（如88888匹配8888）
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        if result.returncode == 0 and "LISTEN" in result.stdout:
            print(f"[系统检查] 服务器端口{port}正在监听：\n{result.stdout.strip()}")
            return True
        else:
            print(f"[系统检查] 服务器端口{port}未监听！")
            return False
    except Exception as e:
        print(f"[系统检查] 出错：{e}")
        return False

def client_task(server_ip, server_port, thread_id, result, data_size=1000):
    """客户端任务：连接服务器并测试回显（增加性能统计）"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(15)  # 连接和接收超时
        
        start_time = time.time()
        # 连接服务器
        sock.connect((server_ip, server_port))
        # 发送数据（固定大小，模拟真实场景）
        data = f"thread_{thread_id}_data_{'x'*(data_size-30)}".encode()  # 总长度约1000字节
        sock.sendall(data)
        # 接收回显
        recv_data = b""
        expected_len = len(data)
        while len(recv_data) < expected_len:
            chunk = sock.recv(min(4096, expected_len - len(recv_data)))
            if not chunk:
                raise Exception(f"连接被关闭，已接收{len(recv_data)}/{expected_len}字节")
            recv_data += chunk
        # 验证回显
        if recv_data == data:
            total_delay = time.time() - start_time
            result.add_success(total_delay)
            # 每1000个成功连接打印一次进度
            if thread_id % 1000 == 0:
                print(f"[线程{thread_id}] 成功（延迟：{total_delay:.4f}秒）")
        else:
            raise Exception(f"数据不匹配（发送{len(data)}字节，接收{len(recv_data)}字节）")
        
    except socket.timeout:
        result.add_timeout()
        if thread_id % 1000 == 0:
            print(f"[线程{thread_id}] 超时")
    except Exception as e:
        result.add_failure()
        if thread_id % 1000 == 0:
            print(f"[线程{thread_id}] 失败：{e}")
    finally:
        sock.close()

def main():
    SERVER_IP = "192.168.88.135"
    SERVER_PORT = 8888
    CONCURRENT = 20000  # 并发连接数
    DATA_SIZE = 1000    # 每个消息大小（字节）
    BATCH_SIZE = 500    # 批量启动线程（避免瞬间资源耗尽）
    BATCH_DELAY = 1.5   # 每批启动间隔（秒）

    # 检查服务器状态
    if not check_server(SERVER_PORT):
        print("服务器未就绪，退出测试")
        return

    # 初始化结果统计
    result = BenchmarkResult()
    result.total = CONCURRENT
    threads = []
    start_time = datetime.now()
    print(f"[{start_time}] 开始测试：{CONCURRENT}个并发连接，每个消息{DATA_SIZE}字节")

    # 批量启动线程（控制启动速度，避免系统过载）
    for i in range(CONCURRENT):
        t = threading.Thread(
            target=client_task,
            args=(SERVER_IP, SERVER_PORT, i, result, DATA_SIZE)
        )
        threads.append(t)
        t.start()
        # 每启动BATCH_SIZE个线程，等待一下
        if (i + 1) % BATCH_SIZE == 0:
            print(f"已启动{i+1}/{CONCURRENT}个线程，等待{BATCH_DELAY}秒...")
            time.sleep(BATCH_DELAY)

    # 等待所有线程完成
    for t in threads:
        t.join()

    # 输出统计结果
    end_time = datetime.now()
    print(f"\n[{end_time}] 测试结束，总耗时：{(end_time - start_time).total_seconds():.2f}秒")
    print(result)

if __name__ == "__main__":
    # 提示用户调整系统限制（避免"too many open files"错误）
    print("建议先执行：ulimit -n 20000（临时调整最大文件描述符）")
    main()
