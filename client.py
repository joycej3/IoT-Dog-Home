import socket
import network
import time

SSID = 'Hope'
PASSWORD = '123456789'

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

if not wlan.isconnected():
    wlan.connect(SSID, PASSWORD)
    while not wlan.isconnected():
        pass

print("Connected to WiFi")

server_ip = '192.168.137.37'
server_port = 80

time.sleep(5)  # Wait for the server to start listening

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((server_ip, server_port))

client.send("Hello, server!".encode())

response = client.recv(1024).decode()
print("Received from server:", response)

client.close()
