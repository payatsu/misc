/**
 * @file
 * @brief Frontend of 16bpcgen.
 */

#include <iostream>
#include "getopt.h"

typedef unsigned int uint32_t;

unsigned long long read_rgb(const char* str)
{
	unsigned long long value = 0;
	while(*str != '\0'){
		if('0' <= *str && *str <= '9'){
			value = value * 0x10 + *str - '0';
		}else if('a' <= *str && *str <= 'f'){
			value = value * 0x10 + 10 + *str - 'a';
		}else if('A' <= *str && *str <= 'F'){
			value = value * 0x10 + 10 + *str - 'A';
		}
		++str;
	}
	return value;
}

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	const uint32_t           height    = store["height"]    == "" ? 1080           : atoi(store["height"].c_str());
	const uint32_t           width     = store["width"]     == "" ? 1920           : atoi(store["width"].c_str());
	const unsigned long long initial   = store["initial"]   == "" ? 0x0            : read_rgb(store["initial"].c_str());
	const unsigned long long increment = store["increment"] == "" ? 0x002200220022 : read_rgb(store["increment"].c_str());
	const uint32_t           tread     = store["tread"]     == "" ? 192            : atoi(store["tread"].c_str());
	for(uint32_t i = 0; i < height; ++i){
		unsigned short rgb[] = {
			static_cast<unsigned short>(initial >> 32 & 0xffff),
			static_cast<unsigned short>(initial >> 16 & 0xffff),
			static_cast<unsigned short>(initial >>  0 & 0xffff)
		};
		for(uint32_t j = 1; j <= width; ++j){
			std::cout.write(reinterpret_cast<const char*>(rgb), 6);
			if(j % tread == 0){
				rgb[0] += increment >> 32 & 0xffff;
				rgb[1] += increment >> 16 & 0xffff;
				rgb[2] += increment >>  0 & 0xffff;
			}
		}
	}
	return 0;
}
