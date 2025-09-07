# win_redis_cli.py
import socket, sys

def send_cmd(cmd: str):
    s = socket.socket()
    s.connect(('127.0.0.1', 6379))
    parts = cmd.split()
    resp = f"*{len(parts)}\r\n"
    for p in parts:
        resp += f"${len(p)}\r\n{p}\r\n"
    s.send(resp.encode())
    return s.recv(1024).decode(errors='ignore')

if __name__ == "__main__":
    print("?? 交互式 Redis ( bypass Windows redis-cli )")
    while True:
        try:
            cmd = input("127.0.0.1:6379> ").strip()
            if cmd.lower() in {"quit", "exit"}:
                break
            print(send_cmd(cmd), end='')
        except KeyboardInterrupt:
            break