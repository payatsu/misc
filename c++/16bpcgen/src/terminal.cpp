#include <cstdio>
#include "Image.hpp"
#include "PatternGenerators.hpp"

int main(int argc, char* argv[])
{
	if(argc < 2){
		return -1;
	}
	Image img(argv[1]);
	for(row_t i = 0; i < img.height(); ++i){
		for(column_t j = 0; j < img.width(); ++j){
			const Pixel<>& p = img[i][j];
			std::printf("\033[48;2;%d;%d;%dm  \033[49m", p.R()/256, p.G()/256, p.B()/256);
		}
		std::putchar('\n');
	}
	return 0;
}
