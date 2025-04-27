import socket
import select
import sys
import time

# Sunucuya mesaj göndermek için bir fonksiyon
def send(sock, message):
    sock.sendall((message + "\r\n").encode())
    time.sleep(0.3)

# IRC sunucusuna bağlanma
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 6667))

# Sunucuya bağlantı için gerekli başlangıç komutları
send(sock, "PASS mypassword")
send(sock, "NICK eren")
send(sock, "USER eren localhost localhost :Eren Yildiz")
send(sock, "JOIN #genel")

# PING gönderme zamanını kontrol et
def send_ping():
    global last_ping_time
    current_time = time.time()
    if current_time - last_ping_time > 60:  # 60 saniye geçtiğinde PING gönder
        send(sock, "PING :ping")
        last_ping_time = current_time

# Ping cevabı alınca son ping zamanını güncelleme
def handle_ping_response(data):
    global last_ping_time
    if "PONG" in data.decode():
        print("Received PONG from server.")
        last_ping_time = time.time()

last_ping_time = time.time()

try:
    while True:
        readable, _, _ = select.select([sock, sys.stdin], [], [])
        for s in readable:
            if s == sock:
                data = sock.recv(4096)
                if not data:
                    print("Server disconnected.")
                    sys.exit()
                print(data.decode().strip())

                # Eğer PONG mesajı alındıysa
                handle_ping_response(data)

            else:
                user_input = sys.stdin.readline()
                if user_input:
                    sock.sendall(user_input.strip().encode() + b"\r\n")

                # PING gönderme
                send_ping()

except KeyboardInterrupt:
    print("Closing connection...")
    sock.close()
