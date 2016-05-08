#include "Image.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "/dev/stdin"  : store["input"];
	std::string output = store["output"] == "" ? "/dev/stdout" : store["output"];

	Image image = Image(input);

	image >> 10 << 10 >> Median() >> "./img/10bit.png";
	image >> 11 << 11 >> Median() >> "./img/11bit.png";
	image >> 12 << 12 >> Median() >> "./img/12bit.png";
	image >> 13 << 13 >> Median() >> "./img/13bit.png";

	return 0;
}
