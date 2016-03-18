#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SIZE 12

int main(void)
{
	int fd = open("/dev/i2c-1", O_RDWR);
	if(fd == -1){
		perror("open()");
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, 0x48) < 0){
		perror("ioctl()");
		return -1;
	}
	uint8_t buf[SIZE] = {};
	buf[0] = 0x48;
	if(write(fd, buf, 1) != 1){
		perror("write()");
		return -1;
	}
	if(read(fd, buf, SIZE) != SIZE){
		perror("read()");
		return -1;
	}

	close(fd);
	int temp = (buf[0] << 8 | buf[1]) >> 3;
	if(4096 <= temp){
		temp -= 8192;
	}
	printf("%f℃\n", temp/16.0);
	return 0;

	/*************************
	i2c_msg msg = {
		.addr  = 0x48,
		.flags = I2C_M_RD,
		.len   =SIZE,
		.buf   = buf,
	};
	i2c_rdwr_ioctl_data data = {
		.msgs  = &msg,
		.nmsgs = 1,
	};

	if(ioctl(fd, I2C_RDWR, &data) == -1){
		perror("ioctl()");
		close(fd);
		return -1;
	}
	close(fd);
	*************************/
}
