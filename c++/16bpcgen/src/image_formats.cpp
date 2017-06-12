#include <sys/stat.h>
#include <unistd.h>
#include "Image.hpp"
#include "PatternGenerators.hpp"
#ifdef _WIN32
#include <direct.h>
#define mkdir(name, perm) _mkdir(name)
#endif

int main(void)
{
	mkdir("./img", 0755);
	const column_t width  = 1920;
	const row_t    height = 1080;
	const std::string pngfile = "./img/PNG.png";
	const std::string tiffile = "./img/TIFF.tif";
	Image(width, height) << Luster(white) << Character("PNG",  black, 60) >> pngfile;
	Image(pngfile)       << Luster(white) << Character("TIFF", black, 60) >> tiffile;
	return 0;
}
