#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "read_img.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);

	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	int result = 0;
	FrameBuffer buffer = read_16bpc_tiff(input, result);

	buffer >> output;

	return 0;
}
