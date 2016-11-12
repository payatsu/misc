#include <sys/stat.h>
#include <unistd.h>
#include "getopt.hpp"
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerators.hpp"
#include "PixelConverters.hpp"

void demo(const Image& orig);
void demo(column_t width, row_t height);

void demo(const Image& orig)
{
	Image r = orig >> Channel(Channel::R);
	Image g = orig >> Channel(Channel::G);
	Image b = orig >> Channel(Channel::B);

	Image gray      = orig >> GrayScale();
	Image threshold = orig >> Threshold(0x7fff, Channel::R);
	Image offset    = orig >> Offset(0xffff/5);
	Image reversal  = orig >> Reversal();

	Image bit6 = orig & 0xfc00;
	Image bit5 = orig & 0xf800;
	Image bit4 = orig & 0xf000;
	Image bit3 = orig & 0xe000;

	Image normalize = orig >> Normalize();
	Image median    = orig >> Median();
	Image smoothing = orig >> WeightedSmoothing();
	Image unsharp   = orig >> UnSharpMask();

	Image prewitt    = orig >> 12 >> Prewitt();
	Image sobel      = orig >> 12 >> Sobel();
	Image laplacian1 = orig >> 12 >> Laplacian3x3();
	Image laplacian2 = orig >> 12 >> Laplacian5x5();

	orig(r)(g)(b)(gray(threshold)(offset)(reversal), Image::ORI_VERT)
		(bit6(bit5)(bit4)(bit3), Image::ORI_VERT)
		(normalize(median)(smoothing)(unsharp), Image::ORI_VERT)
		(prewitt(sobel)(laplacian1)(laplacian2), Image::ORI_VERT) >> "demo.png";
}

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	std::string input  = store["input"]  == "" ? "-" : store["input"];
	std::string output = store["output"] == "" ? "-" : store["output"];
	
	mkdir("./img", 0755);
	chdir("./img");
	if(input != "-"){
		demo(Image(input));
	}

	return 0;
}
