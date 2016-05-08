/**
 * @file
 * @brief Backend of 16bpcgen.
 * @details
 * This program can:
 * - generate PNG and TIFF images of predefined(builtin) patterns,
 * - generate PNG and TIFF images based on binary sequence from stdin.
 */

#include <fstream>
#include <iostream>
#include "Image.hpp"
#include "getopt.hpp"
#include "PatternGenerators.hpp"

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
			<< CrossHatch(192, 108)     >> "crosshatch";
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
	Image(width, height)
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

	Image image(width, height);
	if(store["input"] == "-"){
		image <<= std::cin;
	}else if(store["input"] != ""){
		std::ifstream ifs(store["input"].c_str());
		image <<= ifs;
	}

	if(store["output"] == ""){
		store["output"] = "out";
	}
	image >> store["output"];
	return 0;
}
