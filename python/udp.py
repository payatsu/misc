#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket

host = '192.168.10.255'
port = 5000 # shall confirm the media players accept the port.
with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
	sock.sendto('abc', (host, port))
