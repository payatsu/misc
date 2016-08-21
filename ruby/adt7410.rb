#!/usr/bin/ruby
require 'fcntl'

fd = IO.sysopen '/dev/i2c-1', Fcntl::O_RDWR
IO.open fd, IO::RDWR{|io|
	io.ioctl(0x0703, 0x48)
	io.syswrite 0x48.chr
	buf = io.sysread 12
	temp = (buf[0].ord << 8 | buf[1].ord) >> 3
	temp -= 8192 if 4096 <= temp
	printf "%f\n", temp/16.0
}
