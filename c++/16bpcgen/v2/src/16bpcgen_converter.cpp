#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	FrameBuffer(input)
		>> Tone(GrayScale(), Area(1920/2, 1080/2))
		>> Crop(Area(1920/2, 1080/2, 1920/4, 1080/4))
		>> output;

	return 0;
}
