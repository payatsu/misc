#include <algorithm>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI
#define M_PI 3.1415926535
#endif
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "Image.hpp"
#include "PatternGenerators.hpp"
#include "Painter.hpp"

Image& ColorBar::generate(Image& image)const
{
	const column_t width = image.width();
	const row_t   height = image.height();
	const column_t x = width*3/4;
	std::fill(&image[0][0],               &image[0][width/8],         white  /100*40);
	std::fill(&image[0][width/8],         &image[0][width/8 + x/7],   white  /100*75);
	std::fill(&image[0][width/8 + x/7],   &image[0][width/8 + x/7*2], yellow /100*75);
	std::fill(&image[0][width/8 + x/7*2], &image[0][width/8 + x/7*3], cyan   /100*75);
	std::fill(&image[0][width/8 + x/7*3], &image[0][width/8 + x/7*4], green  /100*75);
	std::fill(&image[0][width/8 + x/7*4], &image[0][width/8 + x/7*5], magenta/100*75);
	std::fill(&image[0][width/8 + x/7*5], &image[0][width/8 + x/7*6], red    /100*75);
	std::fill(&image[0][width/8 + x/7*6], &image[0][width/8 + x],     blue   /100*75);
	std::fill(&image[0][width/8 + x],     &image[0][width],           white  /100*40);
	const row_t h1 = height*7/12;
	Row::fill(image[1], image[h1], image[0]);

	std::fill(&image[h1][0],             &image[h1][width/8],       cyan);
	std::fill(&image[h1][width/8],       &image[h1][width/8 + x/7], white);
	std::fill(&image[h1][width/8 + x/7], &image[h1][width/8 + x],   white/100*75);
	std::fill(&image[h1][width/8 + x],   &image[h1][width],         blue);
	const row_t h2 = h1 + height/12;
	Row::fill(image[h1 + 1], image[h2], image[h1]);

	std::fill(&image[h2][0],           &image[h2][width/8], yellow);
	std::fill(&image[h2][width/8 + x], &image[h2][width],   red);
	std::generate(&image[h2][width/8], &image[h2][width/8 + x], Gradator(white/static_cast<Image::pixel_type::value_type>(x)));
	const row_t h3 = h2 + height/12;
	Row::fill(image[h2 + 1], image[h3], image[h2]);

	std::fill(&image[h3][0],                         &image[h3][width/8],                   white/100*15);
	std::fill(&image[h3][width/8],                   &image[h3][width/8 + x/7*3/2],         black);
	std::fill(&image[h3][width/8 + x/7*3/2],         &image[h3][width/8 + x/7*3/2 + 2*x/7], white);
	std::fill(&image[h3][width/8 + x/7*3/2 + 2*x/7], &image[h3][width/8 + x],               black);
	std::fill(&image[h3][width/8 + x],               &image[h3][width],                     white/100*15);
	Row::fill(image[h3 + 1], image[height], image[h3]);
	return image;
}

Image& Luster::generate(Image& image)const{std::fill(&image[0][0], &image[image.height()][0], pixel_); return image;}

Image& Checker::generate(Image& image)const
{
	const Image::pixel_type pattern1 = invert_ ? black : white;
	const Image::pixel_type pattern2 = invert_ ? white : black;
	const column_t width = image.width();
	const row_t   height = image.height();
	std::fill(&image[0][0],         &image[0][width/4],   pattern1);
	std::fill(&image[0][width/4],   &image[0][width/4*2], pattern2);
	std::fill(&image[0][width/4*2], &image[0][width/4*3], pattern1);
	std::fill(&image[0][width/4*3], &image[0][width],     pattern2);
	Row::fill(image[1],          image[height/4],   image[0]);
	Row::fill(image[height/4*2], image[height/4*3], image[0]);

	std::fill(&image[height/4][0],         &image[height/4][width/4],   pattern2);
	std::fill(&image[height/4][width/4],   &image[height/4][width/4*2], pattern1);
	std::fill(&image[height/4][width/4*2], &image[height/4][width/4*3], pattern2);
	std::fill(&image[height/4][width/4*3], &image[height/4][width],     pattern1);
	Row::fill(image[height/4+1], image[height/4*2], image[height/4]);
	Row::fill(image[height/4*3], image[height],     image[height/4]);
	return image;
}

Image& StairStepH::generate(Image& image)const
{
	const column_t width = image.width();
	const row_t   height = image.height();
	const row_t  stair_height = height/stairs_;
	const column_t step_width = width/steps_ + (width%steps_ ? 1 : 0);
	bool invert = invert_;
	for(row_t row = 0; row < height; row += stair_height){
		Gradator gradator(white/steps_, invert ? white : black, invert);
		for(column_t column = 0; column < width; column += step_width){
			std::fill(&image[row][column], &image[row][std::min(width, column + step_width)],
					gradator());
		}
		Row::fill(image[row + 1], image[std::min(height, row + stair_height)], image[row]);
		invert = !invert;
	}
	return image;
}

Image& StairStepV::generate(Image& image)const
{
	const column_t width = image.width();
	const row_t   height = image.height();
	const column_t stair_width = width/stairs_;
	const row_t    step_height = height/steps_ + (height%steps_ ? 1 : 0);
	bool invert = invert_;
	std::vector<Gradator> gradators;
	for(byte_t i = 0; i < stairs_; ++i){
		gradators.push_back(Gradator(white/steps_, invert ? white : black, invert));
		invert = !invert;
	}
	for(row_t row = 0; row < height; row += step_height){
		for(column_t column = 0; column < width; column += stair_width){
			std::fill(&image[row][column], &image[row][std::min(width, column + stair_width)],
					gradators.at(column/stair_width)());
		}
		Row::fill(image[row + 1], image[std::min(height, row + step_height)], image[row]);
	}
	return image;
}

Image& Ramp::generate(Image& image)const
{
	const column_t width = image.width();
	const row_t   height = image.height();
	if(Image::pixel_type::max < width){
		throw std::runtime_error(": too large image width.");
	}
	const Image::pixel_type::value_type w = static_cast<Image::pixel_type::value_type>(width);
	std::generate(&image[0][0],            &image[0][width],            Gradator(red    /w));
	std::generate(&image[height/12][0],    &image[height/12][width],    Gradator(green  /w));
	std::generate(&image[height/12*2][0],  &image[height/12*2][width],  Gradator(blue   /w));
	std::generate(&image[height/12*3][0],  &image[height/12*3][width],  Gradator(cyan   /w));
	std::generate(&image[height/12*4][0],  &image[height/12*4][width],  Gradator(magenta/w));
	std::generate(&image[height/12*5][0],  &image[height/12*5][width],  Gradator(yellow /w));
	std::generate(&image[height/12*6][0],  &image[height/12*6][width],  Gradator(cyan   /w, red));
	std::generate(&image[height/12*7][0],  &image[height/12*7][width],  Gradator(magenta/w, green));
	std::generate(&image[height/12*8][0],  &image[height/12*8][width],  Gradator(yellow /w, blue));
	std::generate(&image[height/12*9][0],  &image[height/12*9][width],  Gradator(red    /w, cyan));
	std::generate(&image[height/12*10][0], &image[height/12*10][width], Gradator(green  /w, magenta));
	std::generate(&image[height/12*11][0], &image[height/12*11][width], Gradator(blue   /w, yellow));

	Row::fill(image[1],                image[height/12],    image[0]);
	Row::fill(image[height/12    + 1], image[height/12*2],  image[height/12]);
	Row::fill(image[height/12*2  + 1], image[height/12*3],  image[height/12*2]);
	Row::fill(image[height/12*3  + 1], image[height/12*4],  image[height/12*3]);
	Row::fill(image[height/12*4  + 1], image[height/12*5],  image[height/12*4]);
	Row::fill(image[height/12*5  + 1], image[height/12*6],  image[height/12*5]);
	Row::fill(image[height/12*6  + 1], image[height/12*7],  image[height/12*6]);
	Row::fill(image[height/12*7  + 1], image[height/12*8],  image[height/12*7]);
	Row::fill(image[height/12*8  + 1], image[height/12*9],  image[height/12*8]);
	Row::fill(image[height/12*9  + 1], image[height/12*10], image[height/12*9]);
	Row::fill(image[height/12*10 + 1], image[height/12*11], image[height/12*10]);
	Row::fill(image[height/12*11 + 1], image[height],       image[height/12*11]);
	return image;
}

Image& CrossHatch::generate(Image& image)const
{
	const column_t width = image.width();
	const row_t   height = image.height();
	for(row_t i = 0; i < height; i += lattice_height_){
		std::fill(&image[i][0], &image[i][width], pixel_);
	}
	std::fill(&image[height - 1][0], &image[height][0], pixel_);

	for(column_t i = 0; i < width; i += lattice_width_){
		for(row_t j = 0; j < height; ++j){
			image[j][i] = pixel_;
		}
	}
	for(row_t i = 0; i < height; ++i){
		image[i][width - 1] = pixel_;
	}

	const double slope = static_cast<double>(height)/width;
	for(column_t i = 0; i < width; ++i){
		image[std::min(height - 1, static_cast<row_t>(         slope*i))][i] = pixel_;
		image[std::min(height - 1, static_cast<row_t>(height - slope*i))][i] = pixel_;
	}

	const row_t radius     = height/2;
	const row_t shift_v    = height/2;
	const column_t shift_h = width/2;
	for(double theta = 0; theta < 2.0*M_PI; theta += 2.0*M_PI/5000.0){
		row_t    row    = std::min(height - 1, static_cast<row_t>   (shift_v + radius*std::sin(theta)));
		column_t column = std::min(width  - 1, static_cast<column_t>(shift_h + radius*std::cos(theta)));
		image[row][column] = pixel_;
	}
	return image;
}

#if 201103L <= __cplusplus
Image& WhiteNoise::generate(Image& image)const
{
	RandomColor random_color;
	for(row_t row = 0; row < image.height(); ++row){
		for(column_t column = 0; column < image.width(); ++column){
			image[row][column] = random_color();
		}
	}
	return image;
}
#endif

const byte_t char_width  = 8; // dots
const byte_t char_height = 8; // dots
const byte_t char_tab_width = 4; // chars
const byte_t char_bitmask[8] = {
	0x80,
	0x40,
	0x20,
	0x10,
	0x08,
	0x04,
	0x02,
	0x01
};

const byte_t characters[][8] = {
	{ // NUL
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // SOH
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // STX
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ETX
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // EOT
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ENQ
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ACK
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // BEL
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // BS
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // HT
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // LF
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // VT
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // FF
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // CR
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // SO
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // SI
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DLE
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DC1
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DC2
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DC3
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DC4
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // NAK
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // SYN
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ETB
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // CAN
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // EM
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // SUB
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ESC
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // FS
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // GS
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // RS
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // US
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // ' '
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // !
		0x00,
		0x38,
		0x38,
		0x38,
		0x38,
		0x10,
		0x00,
		0x10,
	},{ // "
		0x00,
		0x6c,
		0x6c,
		0x24,
		0x48,
		0x00,
		0x00,
		0x00,
	},{ // #
		0x00,
		0x48,
		0xfc,
		0x48,
		0x48,
		0x48,
		0xfc,
		0x48,
	},{ // $
		0x10,
		0x7c,
		0x92,
		0x70,
		0x1c,
		0x92,
		0x7c,
		0x10,
	},{ // %
		0x00,
		0xe4,
		0xa4,
		0xe8,
		0x10,
		0x2e,
		0x4a,
		0x4e,
	},{ // &
		0x00,
		0x38,
		0x44,
		0x44,
		0x38,
		0x45,
		0x42,
		0x3d,
	},{ // '
		0x00,
		0x30,
		0x30,
		0x10,
		0x20,
		0x00,
		0x00,
		0x00,
	},{ // (
		0x06,
		0x18,
		0x10,
		0x10,
		0x10,
		0x10,
		0x18,
		0x06,
	},{ // )
		0x60,
		0x18,
		0x08,
		0x08,
		0x08,
		0x08,
		0x18,
		0x60,
	},{ // *
		0x00,
		0x10,
		0x54,
		0x38,
		0x10,
		0x38,
		0x54,
		0x10,
	},{ // +
		0x00,
		0x00,
		0x10,
		0x10,
		0x7c,
		0x10,
		0x10,
		0x00,
	},{ // ,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x60,
		0x20,
		0x40,
	},{ // -
		0x00,
		0x00,
		0x00,
		0x00,
		0x7c,
		0x00,
		0x00,
		0x00,
	},{ // .
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x60,
		0x60,
		0x00,
	},{ // /
		0x08,
		0x08,
		0x10,
		0x10,
		0x20,
		0x20,
		0x40,
		0x40,
	},{ // 0
		0x00,
		0x38,
		0x44,
		0x4c,
		0x54,
		0x64,
		0x44,
		0x38,
	},{ // 1
		0x00,
		0x10,
		0x70,
		0x10,
		0x10,
		0x10,
		0x10,
		0x7c,
	},{ // 2
		0x00,
		0x38,
		0x44,
		0x04,
		0x18,
		0x20,
		0x40,
		0x7c,
	},{ // 3
		0x00,
		0x38,
		0x44,
		0x04,
		0x38,
		0x04,
		0x44,
		0x38,
	},{ // 4
		0x00,
		0x08,
		0x18,
		0x28,
		0x48,
		0x7c,
		0x08,
		0x08,
	},{ // 5
		0x00,
		0x78,
		0x40,
		0x40,
		0x78,
		0x04,
		0x44,
		0x38,
	},{ // 6
		0x00,
		0x38,
		0x44,
		0x40,
		0x78,
		0x44,
		0x44,
		0x38,
	},{ // 7
		0x00,
		0x7c,
		0x44,
		0x04,
		0x08,
		0x10,
		0x10,
		0x10,
	},{ // 8
		0x00,
		0x38,
		0x44,
		0x44,
		0x38,
		0x44,
		0x44,
		0x38,
	},{ // 9
		0x00,
		0x38,
		0x44,
		0x44,
		0x3c,
		0x04,
		0x44,
		0x38,
	},{ // :
		0x00,
		0x30,
		0x30,
		0x00,
		0x30,
		0x30,
		0x00,
		0x00,
	},{ /// ;
		0x00,
		0x30,
		0x30,
		0x00,
		0x30,
		0x30,
		0x10,
		0x20,
	},{ // <
		0x00,
		0x0c,
		0x10,
		0x20,
		0x40,
		0x20,
		0x10,
		0x0c,
	},{ // =
		0x00,
		0x00,
		0x7c,
		0x00,
		0x00,
		0x7c,
		0x00,
		0x00,
	},{ // >
		0x00,
		0x60,
		0x10,
		0x08,
		0x04,
		0x08,
		0x10,
		0x60,
	},{ // ?
		0x00,
		0x38,
		0x44,
		0x04,
		0x18,
		0x10,
		0x00,
		0x10,
	},{ // @
		0x00,
		0x7c,
		0x82,
		0xb2,
		0x8a,
		0x3a,
		0x4a,
		0x34,
	},{ // A
		0x00,
		0x10,
		0x28,
		0x28,
		0x44,
		0x7c,
		0x44,
		0x44,
	},{ // B
		0x00,
		0x78,
		0x44,
		0x44,
		0x78,
		0x44,
		0x44,
		0x78,
	},{ // C
		0x00,
		0x38,
		0x44,
		0x40,
		0x40,
		0x40,
		0x44,
		0x38,
	},{ // D
		0x00,
		0x78,
		0x44,
		0x42,
		0x42,
		0x42,
		0x44,
		0x78,
	},{ // E
		0x00,
		0x7c,
		0x40,
		0x40,
		0x78,
		0x40,
		0x40,
		0x7c,
	},{ // F
		0x00,
		0x7c,
		0x40,
		0x40,
		0x78,
		0x40,
		0x40,
		0x40,
	},{ // G
		0x00,
		0x38,
		0x44,
		0x40,
		0x5c,
		0x44,
		0x44,
		0x38,
	},{ // H
		0x00,
		0x44,
		0x44,
		0x44,
		0x7c,
		0x44,
		0x44,
		0x44,
	},{ // I
		0x00,
		0x38,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x38,
	},{ // J
		0x00,
		0x1e,
		0x04,
		0x04,
		0x04,
		0x44,
		0x44,
		0x38,
	},{ // K
		0x00,
		0x44,
		0x48,
		0x48,
		0x50,
		0x68,
		0x44,
		0x42,
	},{ // L
		0x00,
		0x40,
		0x40,
		0x40,
		0x40,
		0x40,
		0x40,
		0x78,
	},{ // M
		0x00,
		0x82,
		0xc6,
		0xaa,
		0x92,
		0x92,
		0x82,
		0x82,
	},{ // N
		0x00,
		0x62,
		0x52,
		0x52,
		0x4a,
		0x4a,
		0x46,
		0x42,
	},{ // O
		0x00,
		0x38,
		0x44,
		0x44,
		0x54,
		0x44,
		0x44,
		0x38,
	},{ // P
		0x00,
		0x78,
		0x44,
		0x44,
		0x78,
		0x40,
		0x40,
		0x40,
	},{ // Q
		0x00,
		0x38,
		0x44,
		0x44,
		0x54,
		0x54,
		0x4c,
		0x3a,
	},{ // R
		0x00,
		0x78,
		0x44,
		0x44,
		0x78,
		0x48,
		0x44,
		0x44,
	},{ // S
		0x00,
		0x78,
		0x84,
		0x80,
		0x78,
		0x04,
		0x84,
		0x78,
	},{ // T
		0x00,
		0x7c,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
	},{ // U
		0x00,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x38,
	},{ // V
		0x00,
		0x44,
		0x44,
		0x44,
		0x28,
		0x28,
		0x28,
		0x10,
	},{ // W
		0x00,
		0x44,
		0x44,
		0x54,
		0x54,
		0x54,
		0x28,
		0x28,
	},{ // X
		0x00,
		0x44,
		0x28,
		0x10,
		0x10,
		0x28,
		0x28,
		0x44,
	},{ // Y
		0x00,
		0x44,
		0x44,
		0x44,
		0x28,
		0x10,
		0x10,
		0x10,
	},{ // Z
		0x00,
		0xfc,
		0x08,
		0x10,
		0x20,
		0x40,
		0x80,
		0xfc,
	},{ // [
		0x00,
		0x1e,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x1e,
	},{ // '\'
		0x40,
		0x40,
		0x20,
		0x20,
		0x10,
		0x10,
		0x08,
		0x08,
	},{ // ]
		0x00,
		0x78,
		0x08,
		0x08,
		0x08,
		0x08,
		0x08,
		0x78,
	},{ // ^
		0x00,
		0x10,
		0x28,
		0x44,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // _
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x7c,
	},{ // `
		0x00,
		0x18,
		0x04,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // a
		0x00,
		0x00,
		0x18,
		0x04,
		0x1c,
		0x24,
		0x24,
		0x3a,
	},{ // b
		0x00,
		0x40,
		0x40,
		0x40,
		0x78,
		0x44,
		0x44,
		0x78,
	},{ // c
		0x00,
		0x00,
		0x00,
		0x38,
		0x44,
		0x40,
		0x44,
		0x38,
	},{ // d
		0x00,
		0x04,
		0x04,
		0x04,
		0x3c,
		0x44,
		0x44,
		0x3c,
	},{ // e
		0x00,
		0x00,
		0x00,
		0x38,
		0x44,
		0x7c,
		0x40,
		0x38,
	},{ // f
		0x00,
		0x08,
		0x14,
		0x10,
		0x3c,
		0x10,
		0x10,
		0x10,
	},{ // g
		0x00,
		0x3a,
		0x44,
		0x78,
		0x40,
		0x78,
		0x44,
		0x38,
	},{ // h
		0x00,
		0x40,
		0x40,
		0x40,
		0x58,
		0x64,
		0x44,
		0x44,
	},{ // i
		0x00,
		0x10,
		0x10,
		0x00,
		0x30,
		0x10,
		0x10,
		0x18,
	},{ // j
		0x00,
		0x18,
		0x00,
		0x38,
		0x08,
		0x08,
		0x48,
		0x30,
	},{ // k
		0x00,
		0x40,
		0x40,
		0x48,
		0x50,
		0x60,
		0x50,
		0x48,
	},{ // l
		0x00,
		0x60,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x38,
	},{ // m
		0x00,
		0x00,
		0x00,
		0xa8,
		0x54,
		0x54,
		0x54,
		0x54,
	},{ // n
		0x00,
		0x00,
		0x00,
		0xd8,
		0x64,
		0x44,
		0x44,
		0x44,
	},{ // o
		0x00,
		0x00,
		0x00,
		0x38,
		0x44,
		0x44,
		0x44,
		0x38,
	},{ // p
		0x00,
		0x00,
		0x58,
		0x64,
		0x64,
		0x58,
		0x40,
		0x40,
	},{ // q
		0x00,
		0x00,
		0x34,
		0x4c,
		0x4c,
		0x34,
		0x04,
		0x04,
	},{ // r
		0x00,
		0x00,
		0x00,
		0x58,
		0x60,
		0x40,
		0x40,
		0x40,
	},{ // s
		0x00,
		0x00,
		0x00,
		0x78,
		0x80,
		0x78,
		0x04,
		0x78,
	},{ // t
		0x00,
		0x00,
		0x00,
		0x20,
		0x78,
		0x20,
		0x24,
		0x18,
	},{ // u
		0x00,
		0x00,
		0x00,
		0x44,
		0x44,
		0x44,
		0x44,
		0x3a,
	},{ // v
		0x00,
		0x00,
		0x00,
		0x44,
		0x44,
		0x28,
		0x28,
		0x10,
	},{ // w
		0x00,
		0x00,
		0x00,
		0x44,
		0x44,
		0x54,
		0x54,
		0x28,
	},{ // x
		0x00,
		0x00,
		0x00,
		0x44,
		0x28,
		0x10,
		0x28,
		0x44,
	},{ // y
		0x00,
		0x00,
		0x00,
		0x44,
		0x44,
		0x3c,
		0x04,
		0x78,
	},{ // z
		0x00,
		0x00,
		0x00,
		0x78,
		0x10,
		0x20,
		0x40,
		0x78,
	},{ // {
		0x00,
		0x06,
		0x08,
		0x08,
		0x10,
		0x08,
		0x08,
		0x06,
	},{ // |
		0x00,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
	},{ // }
		0x00,
		0x60,
		0x10,
		0x10,
		0x08,
		0x10,
		0x10,
		0x60,
	},{ // ~
		0x00,
		0x28,
		0x50,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},{ // DEL
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	},
};

Image& Character::generate(Image& image)const{write(image, row_, column_, text_, pixel_, scale_); return image;}

void Character::write(Image& image, row_t row, column_t column,
		unsigned char c, const Image::pixel_type& pixel, byte_t scale)const
{
	if('~' < c || image.height() <= row || image.width() <= column){
		std::ostringstream oss;
		oss << __func__ << ": out of range. can no write a character. ignored.: row = " << row << ", col = " << column << ", ascii = " << c << '(' << int(c) << ')';
		throw std::runtime_error(oss.str());
	}
	for(byte_t i = 0; i < char_height; ++i){
		for(byte_t j = 0; j < char_width; ++j){
			if(characters[c][i] & char_bitmask[j]){
				for(byte_t k = 0; k < scale; ++k){
					for(byte_t l = 0; l < scale; ++l){
						if(image.height() <= row + i*scale + k || image.width() <= column + j*scale + l){
							continue;
						}
						image[row + i*scale + k][column + j*scale + l] = pixel;
					}
				}
			}
		}
	}
}

void Character::write(Image& image, row_t row, column_t column,
		const std::string& str, const Image::pixel_type& pixel, byte_t scale)const
{
	for(std::string::size_type i = 0, j = 0; i < str.size(); ++i){
		if(str[i] == '\n'){
			row += scale*char_height;
			j = 0;
			continue;
		}else if(str[i] == '\t'){
			j += char_tab_width;
			continue;
		}
		try{
			write(image, row, static_cast<column_t>(column + j*scale*char_width), str[i], pixel, scale);
			++j;
		}catch(const std::runtime_error& err){
			std::cerr << err.what() << std::endl;
		}
	}
}

TypeWriter::TypeWriter(const std::string& textfilename, const Image::pixel_type& pixel):
	width_(), height_(), text_(), pixel_(pixel)
{
	std::ifstream ifs(textfilename.c_str());
	std::string line;
	while(std::getline(ifs, line)){
		width_ = std::max(width_,
				static_cast<column_t>(line.size() +
					(char_tab_width - 1)*std::count_if(line.begin(), line.end(), is_tab)));
		text_ += line + '\n';
		++height_;
	}
	width_ *= char_width;
	height_ *= char_height;
}

Image& TypeWriter::generate(Image& image)const{return image <<= Character(text_, pixel_);}

Image& Line::generate(Image& image)const
{
	if(image.width() <= from_col_ || image.width() <= to_col_ || image.height() <= from_row_ || image.height() <= to_row_){
		std::ostringstream oss;
		oss << __func__ << ": can not draw a line. out of range.: from(x, y) = (" << from_col_ << ", " << from_row_ << "), to(x, y) = (" << to_col_ << ", " << to_row_ << ')';
		throw std::runtime_error(oss.str());
	}
	column_t start_col = from_col_;
	row_t    start_row = from_row_;
	column_t end_col   = to_col_;
	row_t    end_row   = to_row_;
	if(end_row < start_row){
		std::swap(start_col, end_col);
		std::swap(start_row, end_row);
	}
	if(start_row == end_row){
		std::fill(&image[start_row][start_col], &image[start_row][end_col], pixel_);
	}else{
		const row_t diff = end_row - start_row;
		const double slope = static_cast<double>(end_col - start_col)/diff;
		for(row_t r = 0; r < diff; ++r){
			image[r + start_row][static_cast<column_t>(r*slope + start_col)] = pixel_;
		}
	}
	return image;
}

Image& Circle::generate(Image& image)const
{
	if(image.width() <= column_ || image.height() <= row_){
		std::ostringstream oss;
		oss << __func__ << ": can not draw a circle. out of range.: center(x, y) = (" << column_ << ", " << row_ << ')';
		throw std::runtime_error(oss.str());
	}
	image[row_][column_] = pixel_;
	for(double theta = 0.0; theta < 2.0*M_PI; theta += 2.0*M_PI/5000.0){
		row_t    row    = std::min(image.height() - 1, static_cast<row_t>   (row_    + radius_*std::sin(theta)));
		column_t column = std::min(image.width()  - 1, static_cast<column_t>(column_ + radius_*std::cos(theta)));
		image[row][column] = pixel_;
	}
	if(fill_enabled_){
		for(column_t c = column_ - radius_; c < image.width() && c <= column_ + radius_; ++c){
			for(radius_t r = row_ - radius_; r < image.height() && r <= row_ + radius_; ++r){
				if((c - column_)*(c - column_) + (r - row_)*(r - row_) <= radius_*radius_){
					image[r][c] = pixel_;
				}
			}
		}
	}
	return image;
}
