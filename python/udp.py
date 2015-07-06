import socket

host = '192.168.10.255'
port = 4000 # shall confirm the media players accept the port.
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto('abc'.encode('utf-8'), (host, port))
sock.close()
