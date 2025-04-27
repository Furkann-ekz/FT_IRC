import socket
import select
import sys
import time

def send_ping(sock):
    ping_msg = "PING :ping\r\n"
    sock.sendall(ping_msg.encode())
    print("Sent PING to server.")

def receive_message(sock):
    data = sock.recv(4096)
    if data:
        print(data.decode().strip())

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 6667))

sock.sendall("PASS mypassword\r\n".encode())
sock.sendall("NICK testuser\r\n".encode())
sock.sendall("USER testuser localhost localhost :Test User\r\n".encode())
sock.sendall("JOIN #genel\r\n".encode())

last_ping_time = time.time()

try:
    while True:
        readable, _, _ = select.select([sock, sys.stdin], [], [])
        for s in readable:
            if s == sock:
                receive_message(s)
                if "PONG" in s.recv(1024).decode():
                    print("Received PONG from server.")
                    last_ping_time = time.time()
            else:
                user_input = sys.stdin.readline()
                if user_input:
                    sock.sendall(user_input.encode() + b"\r\n")

        # PING gönderme zamanını kontrol et
        if time.time() - last_ping_time >= 60:
            send_ping(sock)
            last_ping_time = time.time()

except KeyboardInterrupt:
    print("Closing connection...")
    sock.close()