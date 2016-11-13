#include <cmath>
#include <sys/stat.h>
#include <unistd.h>
#include "Image.hpp"
#include "PatternGenerators.hpp"

int main(void)
{
	const column_t width  = 1024u;
	const row_t    height = 1024u;
	Image image(width, height);

	mkdir("./img", 0755);

	for(row_t r = 0; r < height; ++r){
		for(column_t c = 0; c < width; ++c){
			try{
				Image::pixel_type p(Image::pixel_type::max/2, c*Image::pixel_type::max/width, r*Image::pixel_type::max/height, Image::pixel_type::CS_YCBCR601);
				image[height - 1 - r][c] = p;
			}catch(const std::range_error& err){
				image[height - 1 - r][c] = black;
			}
		}
	}
	image >> "./img/YCbCr601.png";

	for(row_t r = 0; r < height; ++r){
		for(column_t c = 0; c < width; ++c){
			try{
				Image::pixel_type p(Image::pixel_type::max/2, c*Image::pixel_type::max/width, r*Image::pixel_type::max/height, Image::pixel_type::CS_YCBCR709);
				image[height - 1 - r][c] = p;
			}catch(const std::range_error& err){
				image[height - 1 - r][c] = black;
			}
		}
	}
	image >> "./img/YCbCr709.png";

	image <<= Luster(black);
	const column_t center_column = width/2;
	const row_t    center_row    = height/2;
	const column_t max_radius    = center_column;
	for(double degree = 0.0; degree < 360.0; degree += 1.0/64.0){
		for(column_t r = 0; r < max_radius; ++r){
			const double theta = degree/360.0*2.0*M_PI;
			Pixel<double> p(degree, r*Pixel<double>::max/(max_radius), Pixel<double>::max, Pixel<double>::CS_HSV);
			image[center_row + r*std::sin(theta)][center_column + r*std::cos(theta)] = p;
		}
	}
	image >> "./img/HSV.png";
	return 0;
}
