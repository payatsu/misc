#include <sys/stat.h>
#include <unistd.h>
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerators.hpp"
#include "PixelConverters.hpp"
#ifdef _WIN32
#include <direct.h>
#define mkdir(name, perm) _mkdir(name)
#endif

int main(void)
{
	mkdir("./img", 0755);
	Image orig(480, 270);
	orig >>= Ramp();
	Image r = orig >> Channel(Channel::R);
	Image g = orig >> Channel(Channel::G);
	Image b = orig >> Channel(Channel::B);

	Image gray      = orig >> GrayScale();
	Image threshold = orig >> Threshold(0x7fff, Channel::R);
	Image offset    = orig >> Offset(0xffff/5);
	Image reversal  = orig >> Reversal();

	Image bit6 = orig & Image::pixel_type(0xfc00, 0xfc00, 0xfc00);
	Image bit5 = orig & Image::pixel_type(0xf800, 0xf800, 0xf800);
	Image bit4 = orig & Image::pixel_type(0xf000, 0xf000, 0xf000);
	Image bit3 = orig & Image::pixel_type(0xe000, 0xe000, 0xe000);

	Image normalize = orig >> Normalize();
	Image median    = orig >> Median();
	Image smoothing = orig >> WeightedSmoothing();
	Image unsharp   = orig >> UnSharpMask();

	Image prewitt    = orig >> 12 >> Prewitt();
	Image sobel      = orig >> 12 >> Sobel();
	Image laplacian1 = orig >> 12 >> Laplacian3x3();
	Image laplacian2 = orig >> 12 >> Laplacian5x5();

	orig(r)(g)(b)
		(gray(threshold)(offset)(reversal), Image::ORI_VERT)
		(bit6(bit5)(bit4)(bit3), Image::ORI_VERT)
		(normalize(median)(smoothing)(unsharp), Image::ORI_VERT)
		(prewitt(sobel)(laplacian1)(laplacian2), Image::ORI_VERT) >> "./img/image_processes.png";
	return 0;
}
