#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	std::vector<std::vector<double> > kernel;
	kernel.push_back(std::vector<double>());
	kernel.at(0).push_back(0);
	kernel.at(0).push_back(1);
	kernel.at(0).push_back(0);
	kernel.push_back(std::vector<double>());
	kernel.at(1).push_back(1);
	kernel.at(1).push_back(0);
	kernel.at(1).push_back(-1);
	kernel.push_back(std::vector<double>());
	kernel.at(2).push_back(0);
	kernel.at(2).push_back(-1);
	kernel.at(2).push_back(0);

	FrameBuffer(input)
		>> Channel(Channel::B)
//		>> Filter(kernel)
		>> Normalize()
		>> output;

	return 0;
}
