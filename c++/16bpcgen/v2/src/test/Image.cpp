#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include "Image.hpp"
#include "PatternGenerators.hpp"

int main(void)
{
	mkdir("./img", 0755);
	mkdir("./img/test", 0755);
	Image image(1920, 1080);
	image >>= Ramp();
	image >> "./img/test/test.tif";

	image << "./img/test/test.tif";
	try{
		image << "./img/test/not_found.tif";
	}catch(const std::invalid_argument& err){
		std::cerr << err.what() << std::endl;
	}
	return 0;
}
