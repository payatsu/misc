#include <sys/stat.h>
#include <unistd.h>
#include "getopt.hpp"
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerators.hpp"
#include "PixelConverters.hpp"

void generate_builtin_patterns(uint32_t width, uint32_t height);
void generate_self(uint32_t width, uint32_t height);
void demo(const Image& orig);
void demo(uint32_t width, uint32_t height);

void generate_builtin_patterns(uint32_t width, uint32_t height)
{
	Image image(width, height);

	image << ColorBar()               >> "colorbar";
	image << Luster(white)            >> "white100";
	image << Luster(red)              >> "red100";
	image << Luster(green)            >> "green100";
	image << Luster(blue)             >> "blue100";
	image << Luster(white/2)          >> "white50";
	image << Luster(red/2)            >> "red50";
	image << Luster(green/2)          >> "green50";
	image << Luster(blue/2)           >> "blue50";
	image << Checker()                >> "checker1";
	image << Checker(true)            >> "checker2";
	image << StairStepH()             >> "stairstepH1";
	image << StairStepH(1, 20, false) >> "stairstepH2";
	image << StairStepH(1, 20, true)  >> "stairstepH3";
	image << StairStepV()             >> "stairstepV1";
	image << StairStepV(1, 20, false) >> "stairstepV2";
	image << StairStepV(1, 20, true)  >> "stairstepV3";
	image << Ramp()                   >> "ramp";
	image << Luster(black)
			<< CrossHatch(width/10, height/10)   >> "crosshatch";
	image << Luster(black)
			<< Character(" !\"#$%&'()*+,-./\n"
						"0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\n"
						"abcdefghijklmno\npqrstuvwxyz{|}~", red, 10) >> "character";
#if 201103L <= __cplusplus
	image << WhiteNoise() >> "whitenoise";
#endif
}

void generate_self(uint32_t width, uint32_t height)
{
	Image(width, height) << Luster(white) << TypeWriter(__FILE__, black) >> "sourcecode";
}

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

	((orig, r, g, b) +
	(gray, threshold, offset, reversal) +
	(bit6, bit5, bit4, bit3) +
	(normalize, median, smoothing, unsharp) +
	(prewitt, sobel, laplacian1, laplacian2)) >> "demo.png";
}

void demo(uint32_t width, uint32_t height)
{
	generate_builtin_patterns(width, height);
	generate_self(width, height);

	Image orig = Image(width/4, height/5) << Ramp();
	demo(orig);
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
	}else{
		demo(1920, 1200);
	}

	return 0;
}
