#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include "Image.hpp"
#include "PatternGenerators.hpp"
#ifdef _WIN32
#include <direct.h>
#define mkdir(name, perm) _mkdir(name)
#endif

int main(void)
{
	mkdir("./img", 0755);
	mkdir("./img/test", 0755);
	Image image(1920, 1080);
	image >>= Ramp();

	image >> "./img/test/test.tif";
	try{
		image >> "/nonwritable.tif";
	}catch(const std::invalid_argument& err){
		std::cerr << err.what() << std::endl;
	}

	image << "./img/test/test.tif";
	try{
		image << "./img/test/not_found.tif";
	}catch(const std::invalid_argument& err){
		std::cerr << err.what() << std::endl;
	}
	Image image2("./img/test/test.tif");

	image >> "./img/test/test.png";
	try{
		image >> "/nonwritable.png";
	}catch(const std::invalid_argument& err){
		std::cerr << err.what() << std::endl;
	}

	image << "./img/test/test.png";
	try{
		image << "./img/test/not_found.png";
	}catch(const std::invalid_argument& err){
		std::cerr <<err.what() << std::endl;
	}
	try{
		image << "./img/test/test.tif";
	}catch(const std::runtime_error& err){
		std::cerr << err.what() << std::endl;
	}
	Image image3("./img/test/test.png");

	return 0;
}
