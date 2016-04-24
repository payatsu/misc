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

#include <fstream>
#include <iostream>
#include "FrameBuffer.hpp"
#include "getopt.hpp"
#include "PatternGenerators.hpp"

void generate_builtin_patterns(uint32_t width, uint32_t height)
{
	FrameBuffer buffer(width, height);

	buffer << ColorBar()               >> "colorbar";
	buffer << Luster(white)            >> "white100";
	buffer << Luster(red)              >> "red100";
	buffer << Luster(green)            >> "green100";
	buffer << Luster(blue)             >> "blue100";
	buffer << Luster(white/2)          >> "white50";
	buffer << Luster(red/2)            >> "red50";
	buffer << Luster(green/2)          >> "green50";
	buffer << Luster(blue/2)           >> "blue50";
	buffer << Checker()                >> "checker1";
	buffer << Checker(true)            >> "checker2";
	buffer << StairStepH()             >> "stairstepH1";
	buffer << StairStepH(1, 20, false) >> "stairstepH2";
	buffer << StairStepH(1, 20, true)  >> "stairstepH3";
	buffer << StairStepV()             >> "stairstepV1";
	buffer << StairStepV(1, 20, false) >> "stairstepV2";
	buffer << StairStepV(1, 20, true)  >> "stairstepV3";
	buffer << Ramp()                   >> "ramp";
	buffer << Luster(black)
		   << CrossHatch(192, 108)     >> "crosshatch";
	buffer << Luster(black)
		   << Character(" !\"#$%&'()*+,-./\n"
						"0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\n"
						"abcdefghijklmno\npqrstuvwxyz{|}~", red, 10) >> "character";
#if 201103L <= __cplusplus
	buffer << WhiteNoise() >> "whitenoise";
#endif
}

void generate_self(uint32_t width, uint32_t height)
{
	FrameBuffer(width, height)
		<< Luster(black) << TypeWriter(__FILE__) >> "sourcecode";
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
	buffer >> store["output"];
	return 0;
}
