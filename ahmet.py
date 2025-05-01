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
send(sock, "NICK ahmet")
send(sock, "USER ahmet localhost localhost :Ahmet")
send(sock, "JOIN #genel")

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

            elif s == sys.stdin:
                user_input = sys.stdin.readline().strip()
                if user_input:
                    send(sock, user_input)

except KeyboardInterrupt:
    print("Closing connection...")
    sock.close()