#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	FrameBuffer gray = FrameBuffer(input) >> GrayScale() >> "grayscale.png";

	FrameBuffer med_sobel   = gray;
	FrameBuffer sobel       = gray;
	FrameBuffer med_prewitt = gray;
	FrameBuffer prewitt     = gray;

	med_sobel   >> 12 >> Median() >> Sobel()   >> "median-sobel.png";
	sobel       >> 12             >> Sobel()   >> "sobel.png";

	med_prewitt >> 12 >> Median() >> Prewitt() >> "median-prewitt.png";
	prewitt     >> 12             >> Prewitt() >> "prewitt.png";


	; //>> output;

	return 0;
}
