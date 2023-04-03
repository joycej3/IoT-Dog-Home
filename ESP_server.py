import socket
import network

SSID = 'Hope'
PASSWORD = '123456789'

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

if not wlan.isconnected():
    wlan.connect(SSID, PASSWORD)
    while not wlan.isconnected():
        pass

print("Connected to WiFi")
print("Server IP:", wlan.ifconfig()[0])

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('', 80))
server.listen(5)

print("Server listening on port 80")

while True:
    conn, addr = server.accept()
    print("Connection from", addr)
    data = conn.recv(1024).decode()
    print("Received:", data)
    conn.send("Hello, Vipul : Team Bhidu!".encode())
    conn.close()
