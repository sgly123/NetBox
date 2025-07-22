#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import socket
import threading
import subprocess
import time

def check_server(port):
    """检查服务器端口是否在监听（Linux端特有的系统命令调用）"""
    try:
        # 调用netstat检查端口状态
        result = subprocess.run(
            f"netstat -tlnp | grep {port}",
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

def client_task(server_ip, server_port, thread_id):
    """客户端任务：连接服务器并测试回显"""
    try:
        # 创建socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 设置超时（5秒）
        sock.settimeout(5)
        
        # 连接服务器
        start_time = time.time()
        sock.connect((server_ip, server_port))
        connect_time = time.time() - start_time
        local_ip, local_port = sock.getsockname()
        print(f"[线程{thread_id}] 连接成功（本地：{local_ip}:{local_port} → 服务器：{server_ip}:{server_port}，耗时{connect_time:.2f}秒）")
        data = f"test_from_thread_{thread_id}_at_{time.time()}".encode() + b"\n"
        sock.sendall(data)
        print(f"[线程{thread_id}] 发送数据：{data.decode().strip()}（长度{len(data)}）")
        
        recv_data = b""
        expected_len = len(data)
        while len(recv_data) < expected_len:
            chunk = sock.recv(expected_len - len(recv_data))
            if not chunk:
                raise Exception(f"服务器提前关闭连接，已接收{len(recv_data)}/{expected_len}")
            recv_data += chunk
        
        if recv_data == data:
            print(f"[线程{thread_id}] 回显正确（耗时{time.time() - start_time:.2f}秒）")
        else:
            print(f"[线程{thread_id}] 回显错误：发送{data!r}，收到{recv_data!r}")
        
    except socket.timeout:
        print(f"[线程{thread_id}] 超时（连接/接收超过5秒）")
    except Exception as e:
        print(f"[线程{thread_id}] 出错：{e}")
    finally:
        sock.close()

if __name__ == "__main__":
    SERVER_IP = "192.168.88.135"  
    SERVER_PORT = 8888
    if not check_server(SERVER_PORT):
        print("服务器未启动或端口未监听，退出测试")
        exit(1)

    thread_count = 10
    threads = []
    for i in range(thread_count):
        t = threading.Thread(target=client_task, args=(SERVER_IP, SERVER_PORT, i))
        threads.append(t)
        t.start()
        time.sleep(0.1)  
    

    for t in threads:
        t.join()
    print("所有客户端测试完成")