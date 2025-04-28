import socket
import select
import sys
import time

def send(sock, message):
    sock.sendall((message + "\r\n").encode())
    time.sleep(0.3)

# Bağlantı kur
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 6667))

# Giriş işlemleri
send(sock, "PASS mypassword")
send(sock, "NICK eren")
send(sock, "USER eren localhost localhost :Eren Yildiz")
send(sock, "JOIN #genel")

last_ping_time = time.time()

def send_ping():
    global last_ping_time
    current_time = time.time()
    if current_time - last_ping_time > 60:
        send(sock, "PING :ping")
        last_ping_time = current_time

def handle_ping_response(data):
    global last_ping_time
    if b"PONG" in data:
        print("Received PONG from server.")
        last_ping_time = time.time()

try:
    while True:
        readable, _, _ = select.select([sock, sys.stdin], [], [], 1)

        for s in readable:
            if s == sock:
                data = sock.recv(4096)
                if not data:
                    print("Server disconnected.")
                    sys.exit()
                print(data.decode().strip())
                handle_ping_response(data)

            elif s == sys.stdin:
                user_input = sys.stdin.readline().strip()
                if user_input:
                    send(sock, user_input)

        send_ping()

except KeyboardInterrupt:
    print("Closing connection...")
    sock.close()
