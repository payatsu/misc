#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>
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
	const column_t width  = 1024u;
	const row_t    height = 1024u;
	Image image(width, height);
	typedef Image::pixel_type::value_type value_type;

	mkdir("./img", 0755);

	for(row_t r = 0; r < height; ++r){
		for(column_t c = 0; c < width; ++c){
			try{
				Image::pixel_type p(Image::pixel_type::max/2, static_cast<value_type>(c*Image::pixel_type::max/width), static_cast<value_type>(r*Image::pixel_type::max/height), Image::pixel_type::CS_YCBCR_BT601);
				image[height - 1 - r][c] = p;
			}catch(const std::invalid_argument& err){
				image[height - 1 - r][c] = black;
			}
		}
	}
	image >> "./img/YCbCr601.png";

	for(row_t r = 0; r < height; ++r){
		for(column_t c = 0; c < width; ++c){
			try{
				Image::pixel_type p(Image::pixel_type::max/2, static_cast<value_type>(c*Image::pixel_type::max/width), static_cast<value_type>(r*Image::pixel_type::max/height), Image::pixel_type::CS_YCBCR_BT709);
				image[height - 1 - r][c] = p;
			}catch(const std::invalid_argument& err){
				image[height - 1 - r][c] = black;
			}
		}
	}
	image >> "./img/YCbCr709.png";

	for(row_t r = 0; r < height; ++r){
		for(column_t c = 0; c < width; ++c){
			try{
				Image::pixel_type p(Image::pixel_type::max/2, static_cast<value_type>(c*Image::pixel_type::max/width), static_cast<value_type>(r*Image::pixel_type::max/height), Image::pixel_type::CS_YCBCR_BT2020);
				image[height - 1 - r][c] = p;
			}catch(const std::invalid_argument& err){
				image[height - 1 - r][c] = black;
			}
		}
	}
	image >> "./img/YCbCr2020.png";

	image <<= Luster(black);
	const column_t center_column = width/2;
	const row_t    center_row    = height/2;
	const column_t max_radius    = center_column;
	for(double degree = 0.0; degree < 360.0; degree += 1.0/64.0){
		for(column_t r = 0; r < max_radius; ++r){
			const double theta = degree/360.0*2.0*M_PI;
			Pixel<double> p(degree, r*Pixel<double>::max/(max_radius), Pixel<double>::max, Pixel<double>::CS_HSV);
			image[static_cast<row_t>(center_row + r*std::sin(theta))][static_cast<column_t>(center_column + r*std::cos(theta))] = p;
		}
	}
	image >> "./img/HSV1.png";

	const column_t width2  = 1920u;
	const row_t    height2 = 1080u;
	Image image2(width2, height2);
	for(row_t r = 0; r < height2; ++r){
		for(column_t c = 0; c < width2; ++c){
			Pixel<double> p(c*360.0/width2, Pixel<double>::max*(height2 - r)/height2, (height2 - r)*Pixel<double>::max/height2, Pixel<double>::CS_HSV);
			image2[r][c] = p;
		}
	}
	image2 >> "./img/HSV2.png";
	return 0;
}
