#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	FrameBuffer (input)
		>> 8
		>> WeightedSmoothing()
		>> Laplacian3x3()
		<< 8
		>> output;

	return 0;
}
