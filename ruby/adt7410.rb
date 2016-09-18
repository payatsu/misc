#!/usr/bin/ruby
# -*- coding: utf-8 -*-

if $0 == __FILE__
	require_relative 'i2c'
	ADT7410 = 0x48
	DATA_SIZE = 12
	buf = i2c_read ADT7410, 0.chr, DATA_SIZE
	temp = (buf[0].ord << 8 | buf[1].ord) >> 3
	temp -= 8192 if temp & 0xf000 != 0
	printf "%f℃\n", temp/16.0
	buf.each_byte do |i|
		printf '%02x ', i.ord
	end
end
