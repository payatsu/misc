#!/usr/bin/ruby

I2C_SLAVE = 0x0703

def i2c_read devaddr, regaddr, size
	File.open '/dev/i2c-1', IO::RDWR do |f|
		f.ioctl I2C_SLAVE, devaddr
		f.syswrite devaddr.chr
#		f.syswrite 2 # regaddr
		return f.sysread size
	end
end

# def i2c_write devaddr, regaddr, data
# 	File.open '/dev/i2c-1', IO::RDWR do |f|
# 		f.ioctl I2C_SLAVE, devaddr
# 		f.syswrite devaddr.chr | 0x01
# 		f.syswrite regaddr
# 		return f.sysread size
# 	end
# end
