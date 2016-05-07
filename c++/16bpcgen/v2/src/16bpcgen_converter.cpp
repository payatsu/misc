#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "/dev/stdin"  : store["input"];
	std::string output = store["output"] == "" ? "/dev/stdout" : store["output"];

	FrameBuffer image = FrameBuffer(input);
	FrameBuffer gray = image >> GrayScale() >> "./img/grayscale.png";

	gray >> 12 >> Median() >> Sobel()        << 12 >> "./img/median-sobel.png";
	gray >> 12             >> Sobel()        << 12 >> "./img/sobel.png";

	gray >> 12 >> Median() >> Prewitt()      << 12 >> "./img/median-prewitt.png";
	gray >> 12             >> Prewitt()      << 12 >> "./img/prewitt.png";

	gray >> 12 >> Median() >> Laplacian3x3() << 12 >> "./img/median-laplacian.png";
	gray >> 12 >>             Laplacian3x3() << 12 >> "./img/laplacian.png";

	FrameBuffer r = image >> Channel(Channel::R) >> "./img/r.png";
	FrameBuffer g = image >> Channel(Channel::G) >> "./img/g.png";
	FrameBuffer b = image >> Channel(Channel::B) >> "./img/b.png";

	FrameBuffer mask = (r | g | b) >> "./img/merge.png";

	image >> Reversal() >> "./img/reversal.png";

	std::fill(mask.head() + mask.data_size()/2, mask.head() + mask.data_size(), 0x0);

	(r & mask) >> "./img/masked.png";


	; //>> output;

	return 0;
}
