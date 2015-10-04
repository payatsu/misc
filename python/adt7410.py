#!/usr/bin/python3
# -*- coding: utf-8 -*-

import smbus

address = 0x48

block = smbus.SMBus(1).read_i2c_block_data(address, 0x00, 12)
temp = (block[0] << 8 | block[1]) >> 3
if temp >= 4096:
	temp -= 8192
print(temp/16.0, '℃')
