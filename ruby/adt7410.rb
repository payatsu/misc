#!/usr/bin/ruby
require 'fcntl'

File.open '/dev/i2c-1', IO::RDWR do |f|
	f.ioctl(0x0703, 0x48)
	f.syswrite 0x48.chr
	buf = f.sysread 12
	temp = (buf[0].ord << 8 | buf[1].ord) >> 3
	temp -= 8192 if 4096 <= temp
	printf "%f\n", temp/16.0
end
