#include "FrameBuffer.hpp"
#include "getopt.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);

	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	FrameBuffer(input) >> output;

	return 0;
}
