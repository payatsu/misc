#!/usr/bin/python
import socket

host = '192.168.10.255'
port = 5000 # shall confirm the media players accept the port.
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto('abc', (host, port))
sock.close()
