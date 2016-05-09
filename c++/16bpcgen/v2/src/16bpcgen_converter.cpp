#include "Image.hpp"
#include "getopt.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverters.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];

	Image image = Image(input);
	Image r     = image >> Channel(Channel::R);
	Image g     = image >> Channel(Channel::G);
	Image b     = image >> Channel(Channel::B);

	Image gray      = image >> GrayScale();
	Image threshold = image >> Threshold(0x7fff, Channel::R);
	Image offset    = image >> Offset(0xffff/5);
	Image reversal  = image >> Reversal();

	Image bit10 = image & 0xfc00;
    Image bit11 = image & 0xf800;
    Image bit12 = image & 0xf000;
    Image bit13 = image & 0xe000;

	Image normalize  = image >> Normalize();
	Image median     = image >> Median();
	Image smoothing  = image >> WeightedSmoothing();
	Image unsharp    = image >> UnSharpMask();

	Image prewitt    = image >> 12 >> Prewitt();
	Image sobel      = image >> 12 >> Sobel();
	Image laplacian1 = image >> 12 >> Laplacian3x3();
	Image laplacian2 = image >> 12 >> Laplacian5x5();

	((image, r, g, b) +
	(gray, threshold, offset, reversal) +
	(bit10, bit11, bit12, bit13) +
	(normalize, median, smoothing, unsharp) +
	(prewitt, sobel, laplacian1, laplacian2)) >> output;

	return 0;
}
