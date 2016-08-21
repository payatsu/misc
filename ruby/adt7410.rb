#!/usr/bin/ruby
# -*- condig: utf-8 -*-

dev = '/dev/i2c-1'

if __FILE__ == $0
	fd = IO.sysopen dev, IO::RDWR
	io = IO.new fd, IO::RDWR
	io.ioctl(0x0703, 0x48)
	buf = Array.new
	buf << 0x48
	io.write buf
	io.read buf
	io.close

	puts buf[0], buf[1]
end
