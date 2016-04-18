/**
 * @file
 * @brief Backend of 16bpcgen.
 * @details
 * This program can:
 * - generate PNG and TIFF images of predefined(builtin) patterns,
 * - generate PNG and TIFF images based on binary sequence from stdin.
 */

// TODO ColorStep
// TODO Multi
// TODO Focus

#define _USE_MATH_DEFINES
#define PNG_NO_SETJMP

#include <fstream>
#include "getopt.hpp"
#include "FrameBuffer.hpp"
#include "Painter.hpp"
#include "PatternGenerator.hpp"
#include "write_img.hpp"

void generate_builtin_patterns(uint32_t width, uint32_t height)
{
	FrameBuffer buffer(width, height);

	generate_16bpc_img("colorbar",    buffer << ColorBar());
	generate_16bpc_img("white100",    buffer << Luster(white));
	generate_16bpc_img("red100",      buffer << Luster(red));
	generate_16bpc_img("green100",    buffer << Luster(green));
	generate_16bpc_img("blue100",     buffer << Luster(blue));
	generate_16bpc_img("white50",     buffer << Luster(white/2));
	generate_16bpc_img("red50",       buffer << Luster(red/2));
	generate_16bpc_img("green50",     buffer << Luster(green/2));
	generate_16bpc_img("blue50",      buffer << Luster(blue/2));
	generate_16bpc_img("checker1",    buffer << Checker());
	generate_16bpc_img("checker2",    buffer << Checker(true));
	generate_16bpc_img("stairstepH1", buffer << StairStepH());
	generate_16bpc_img("stairstepH2", buffer << StairStepH(1, 20, false));
	generate_16bpc_img("stairstepH3", buffer << StairStepH(1, 20, true));
	generate_16bpc_img("stairstepV1", buffer << StairStepV());
	generate_16bpc_img("stairstepV2", buffer << StairStepV(1, 20, false));
	generate_16bpc_img("stairstepV3", buffer << StairStepV(1, 20, true));
	generate_16bpc_img("ramp",        buffer << Ramp());
	generate_16bpc_img("crosshatch",  buffer << Luster(black) << CrossHatch(192, 108));
	generate_16bpc_img("character",   buffer << Luster(black) << Character(" !\"#$%&'()*+,-./\n"
			"0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\n"
			"abcdefghijklmno\npqrstuvwxyz{|}~", red, 10));
#if 201103L <= __cplusplus
	generate_16bpc_img("whitenoise", buffer << WhiteNoise());
#endif
}

void generate_self(uint32_t width, uint32_t height)
{
	generate_16bpc_img("sourcecode", FrameBuffer(width, height)
			<< Luster(black) << TypeWriter(__FILE__));
}

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	const uint32_t height = store["height"] == "" ? 1080 : atoi(store["height"].c_str());
	const uint32_t width  = store["width"]  == "" ? 1920 : atoi(store["width"].c_str());

	if(store["builtins"] != ""){
		generate_builtin_patterns(width, height);
		return 0;
	}

	FrameBuffer buffer(width, height);
	if(store["input"] == "-"){
		buffer << std::cin;
	}else if(store["input"] != ""){
		std::ifstream ifs(store["input"].c_str());
		buffer << ifs;
	}

	if(store["output"] == ""){
		store["output"] = "out";
	}
#ifdef ENABLE_TIFF
	generate_16bpc_tiff(append_extension(store["output"], ".tif"), buffer);
#endif
#ifdef ENABLE_PNG
	generate_16bpc_png(append_extension(store["output"], ".png"), buffer);
#endif
	return 0;
}
