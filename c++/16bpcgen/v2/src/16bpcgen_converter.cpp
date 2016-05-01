#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	std::vector<std::vector<Pixel::value_type> > array;
	array.push_back(std::vector<Pixel::value_type>());
	array.at(0).push_back(0);
	array.at(0).push_back(1);
	array.at(0).push_back(0);
	array.push_back(std::vector<Pixel::value_type>());
	array.at(1).push_back(1);
	array.at(1).push_back(0);
	array.at(1).push_back(-1);
	array.push_back(std::vector<Pixel::value_type>());
	array.at(2).push_back(0);
	array.at(2).push_back(-1);
	array.at(2).push_back(0);

	FrameBuffer(input)
		>> Channel(Channel::B)
		>> Filter(array)
//		>> Normalize()
		>> output;

	return 0;
}
