#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include "FrameBuffer.hpp"
#include "PatternGenerators.hpp"
#include "Painter.hpp"

FrameBuffer& ColorBar::generate(FrameBuffer& buffer)const
{
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	const uint32_t x = width*3/4;
	std::fill(&buffer[0][0],               &buffer[0][width/8],         white  /100*40);
	std::fill(&buffer[0][width/8],         &buffer[0][width/8 + x/7],   white  /100*75);
	std::fill(&buffer[0][width/8 + x/7],   &buffer[0][width/8 + x/7*2], yellow /100*75);
	std::fill(&buffer[0][width/8 + x/7*2], &buffer[0][width/8 + x/7*3], cyan   /100*75);
	std::fill(&buffer[0][width/8 + x/7*3], &buffer[0][width/8 + x/7*4], green  /100*75);
	std::fill(&buffer[0][width/8 + x/7*4], &buffer[0][width/8 + x/7*5], magenta/100*75);
	std::fill(&buffer[0][width/8 + x/7*5], &buffer[0][width/8 + x/7*6], red    /100*75);
	std::fill(&buffer[0][width/8 + x/7*6], &buffer[0][width/8 + x],     blue   /100*75);
	std::fill(&buffer[0][width/8 + x],     &buffer[0][width],           white  /100*40);
	const uint32_t h1 = height*7/12;
	Row::fill(buffer[1], buffer[h1], buffer[0]);

	std::fill(&buffer[h1][0],             &buffer[h1][width/8],       cyan);
	std::fill(&buffer[h1][width/8],       &buffer[h1][width/8 + x/7], white);
	std::fill(&buffer[h1][width/8 + x/7], &buffer[h1][width/8 + x],   white/100*75);
	std::fill(&buffer[h1][width/8 + x],   &buffer[h1][width],         blue);
	const uint32_t h2 = h1 + height/12;
	Row::fill(buffer[h1 + 1], buffer[h2], buffer[h1]);

	std::fill(&buffer[h2][0],           &buffer[h2][width/8], yellow);
	std::fill(&buffer[h2][width/8 + x], &buffer[h2][width],   red);
	std::generate(&buffer[h2][width/8], &buffer[h2][width/8 + x], Gradator(white/x));
	const uint32_t h3 = h2 + height/12;
	Row::fill(buffer[h2 + 1], buffer[h3], buffer[h2]);

	std::fill(&buffer[h3][0],                         &buffer[h3][width/8],                   white/100*15);
	std::fill(&buffer[h3][width/8],                   &buffer[h3][width/8 + x/7*3/2],         black);
	std::fill(&buffer[h3][width/8 + x/7*3/2],         &buffer[h3][width/8 + x/7*3/2 + 2*x/7], white);
	std::fill(&buffer[h3][width/8 + x/7*3/2 + 2*x/7], &buffer[h3][width/8 + x],               black);
	std::fill(&buffer[h3][width/8 + x],               &buffer[h3][width],                     white/100*15);
	Row::fill(buffer[h3 + 1], buffer[height], buffer[h3]);
	return buffer;
}

FrameBuffer& Luster::generate(FrameBuffer& buffer)const{std::fill(&buffer[0][0], &buffer[buffer.height()][0], pixel_); return buffer;}

FrameBuffer& Checker::generate(FrameBuffer& buffer)const
{
	const Pixel<uint16_t> pattern1 = invert_ ? black : white;
	const Pixel<uint16_t> pattern2 = invert_ ? white : black;
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	std::fill(&buffer[0][0],         &buffer[0][width/4],   pattern1);
	std::fill(&buffer[0][width/4],   &buffer[0][width/4*2], pattern2);
	std::fill(&buffer[0][width/4*2], &buffer[0][width/4*3], pattern1);
	std::fill(&buffer[0][width/4*3], &buffer[0][width],     pattern2);
	Row::fill(buffer[1],          buffer[height/4],   buffer[0]);
	Row::fill(buffer[height/4*2], buffer[height/4*3], buffer[0]);

	std::fill(&buffer[height/4][0],         &buffer[height/4][width/4],   pattern2);
	std::fill(&buffer[height/4][width/4],   &buffer[height/4][width/4*2], pattern1);
	std::fill(&buffer[height/4][width/4*2], &buffer[height/4][width/4*3], pattern2);
	std::fill(&buffer[height/4][width/4*3], &buffer[height/4][width],     pattern1);
	Row::fill(buffer[height/4+1], buffer[height/4*2], buffer[height/4]);
	Row::fill(buffer[height/4*3], buffer[height],     buffer[height/4]);
	return buffer;
}

FrameBuffer& StairStepH::generate(FrameBuffer& buffer)const
{
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	const uint32_t stair_height = height/stairs_;
	const uint32_t step_width = width/steps_ + (width%steps_ ? 1 : 0);
	bool invert = invert_;
	for(uint32_t row = 0; row < height; row += stair_height){
		Gradator gradator(white/steps_, invert ? white : black, invert);
		for(uint32_t column = 0; column < width; column += step_width){
			std::fill(&buffer[row][column], &buffer[row][std::min(width, column + step_width)],
					gradator());
		}
		Row::fill(buffer[row + 1], buffer[std::min(height, row + stair_height)], buffer[row]);
		invert = !invert;
	}
	return buffer;
}

FrameBuffer& StairStepV::generate(FrameBuffer& buffer)const
{
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	const uint32_t stair_width = width/stairs_;
	const uint32_t step_height = height/steps_ + (height%steps_ ? 1 : 0);
	bool invert = invert_;
	std::vector<Gradator> gradators;
	for(int i = 0; i < stairs_; ++i){
		gradators.push_back(Gradator(white/steps_, invert ? white : black, invert));
		invert = !invert;
	}
	for(uint32_t row = 0; row < height; row += step_height){
		for(uint32_t column = 0; column < width; column += stair_width){
			std::fill(&buffer[row][column], &buffer[row][std::min(width, column + stair_width)],
					gradators.at(column/stair_width)());
		}
		Row::fill(buffer[row + 1], buffer[std::min(height, row + step_height)], buffer[row]);
	}
	return buffer;
}

FrameBuffer& Ramp::generate(FrameBuffer& buffer)const
{
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	std::generate(&buffer[0][0],            &buffer[0][width],            Gradator(red    /width));
	std::generate(&buffer[height/12][0],    &buffer[height/12][width],    Gradator(green  /width));
	std::generate(&buffer[height/12*2][0],  &buffer[height/12*2][width],  Gradator(blue   /width));
	std::generate(&buffer[height/12*3][0],  &buffer[height/12*3][width],  Gradator(cyan   /width));
	std::generate(&buffer[height/12*4][0],  &buffer[height/12*4][width],  Gradator(magenta/width));
	std::generate(&buffer[height/12*5][0],  &buffer[height/12*5][width],  Gradator(yellow /width));
	std::generate(&buffer[height/12*6][0],  &buffer[height/12*6][width],  Gradator(cyan   /width, red));
	std::generate(&buffer[height/12*7][0],  &buffer[height/12*7][width],  Gradator(magenta/width, green));
	std::generate(&buffer[height/12*8][0],  &buffer[height/12*8][width],  Gradator(yellow /width, blue));
	std::generate(&buffer[height/12*9][0],  &buffer[height/12*9][width],  Gradator(red    /width, cyan));
	std::generate(&buffer[height/12*10][0], &buffer[height/12*10][width], Gradator(green  /width, magenta));
	std::generate(&buffer[height/12*11][0], &buffer[height/12*11][width], Gradator(blue   /width, yellow));

	Row::fill(buffer[1],                buffer[height/12],    buffer[0]);
	Row::fill(buffer[height/12    + 1], buffer[height/12*2],  buffer[height/12]);
	Row::fill(buffer[height/12*2  + 1], buffer[height/12*3],  buffer[height/12*2]);
	Row::fill(buffer[height/12*3  + 1], buffer[height/12*4],  buffer[height/12*3]);
	Row::fill(buffer[height/12*4  + 1], buffer[height/12*5],  buffer[height/12*4]);
	Row::fill(buffer[height/12*5  + 1], buffer[height/12*6],  buffer[height/12*5]);
	Row::fill(buffer[height/12*6  + 1], buffer[height/12*7],  buffer[height/12*6]);
	Row::fill(buffer[height/12*7  + 1], buffer[height/12*8],  buffer[height/12*7]);
	Row::fill(buffer[height/12*8  + 1], buffer[height/12*9],  buffer[height/12*8]);
	Row::fill(buffer[height/12*9  + 1], buffer[height/12*10], buffer[height/12*9]);
	Row::fill(buffer[height/12*10 + 1], buffer[height/12*11], buffer[height/12*10]);
	Row::fill(buffer[height/12*11 + 1], buffer[height],       buffer[height/12*11]);
	return buffer;
}

FrameBuffer& CrossHatch::generate(FrameBuffer& buffer)const
{
	const uint32_t width = buffer.width();
	const uint32_t height = buffer.height();
	for(uint32_t i = 0; i < height; i += lattice_height_){
		std::fill(&buffer[i][0], &buffer[i][width], white);
	}
	std::fill(&buffer[height - 1][0], &buffer[height][0], white);

	for(uint32_t i = 0; i < width; i += lattice_width_){
		for(uint32_t j = 0; j < height; ++j){
			buffer[j][i] = white;
		}
	}
	for(uint32_t i = 0; i < height; ++i){
		buffer[i][width - 1] = white;
	}

	const double slope = static_cast<double>(height)/width;
	for(uint32_t i = 0; i < width; ++i){
		buffer[slope*i][i] = white;
		buffer[height - slope*i][i] = white;
	}

	const uint32_t radius = height/2;
	const uint32_t shift_v = height/2;
	const uint32_t shift_h = width/2;
	for(double theta = 0; theta < 2*M_PI; theta += 2.0*M_PI/5000.0){
		uint32_t row    = std::min(height - 1, static_cast<uint32_t>(shift_v + radius*std::sin(theta)));
		uint32_t column = std::min(width  - 1, static_cast<uint32_t>(shift_h + radius*std::cos(theta)));
		buffer[row][column] = white;
	}
	return buffer;
}

#if 201103L <= __cplusplus
FrameBuffer& WhiteNoise::generate(FrameBuffer& buffer)const
{
	RandomColor random_color;
	for(uint32_t row = 0; row < buffer.height(); ++row){
		for(uint32_t column = 0; column < buffer.width(); ++column){
			buffer[row][column] = random_color();
		}
	}
	return buffer;
}
#endif

const unsigned char char_width  = 8; // dots
const unsigned char char_height = 8; // dots
const unsigned char char_tab_width = 4; // chars
const unsigned char char_bitmask[8] = {
	0x80,
	0x40,
	0x20,
	0x10,
	0x08,
	0x04,
	0x02,
	0x01
};

const unsigned char characters[][8] = {
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

FrameBuffer& Character::generate(FrameBuffer& buffer)const{write(buffer, row_, column_, text_, pixel_, scale_); return buffer;}

void Character::write(FrameBuffer& buffer, uint32_t row, uint32_t column,
		unsigned char c, const Pixel<uint16_t>& pixel, int scale)const
{
	if('~' < c || buffer.height() <= row || buffer.width() <= column){
		std::cerr << "warning: not supported: row: " << row
			<< ", col: " << column << ", ascii: "
			<< c << '(' << int(c) << ')' << std::endl;
		return;
	}
	for(unsigned char i = 0; i < char_height; ++i){
		for(unsigned char j = 0; j < char_width; ++j){
			if(characters[c][i] & char_bitmask[j]){
				for(int k = 0; k < scale; ++k){
					for(int l = 0; l < scale; ++l){
						buffer[row + i*scale + k][column + j*scale + l] = pixel;
					}
				}
			}
		}
	}
}

void Character::write(FrameBuffer& buffer, uint32_t row, uint32_t column,
		const std::string& str, const Pixel<uint16_t>& pixel, int scale)const
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
		write(buffer, row, column + j*scale*char_width, str[i], pixel, scale);
		++j;
	}
}

TypeWriter::TypeWriter(const std::string& textfilename): width_(), height_(), text_()
{
	std::ifstream ifs(textfilename.c_str());
	std::string line;
	while(std::getline(ifs, line)){
		width_ = std::max(width_,
				static_cast<uint32_t>(line.size() +
					(char_tab_width - 1)*std::count_if(line.begin(), line.end(), is_tab)));
		text_ += line + '\n';
		++height_;
	}
	width_ *= char_width;
	height_ *= char_height;
}

FrameBuffer& TypeWriter::generate(FrameBuffer& buffer)const{buffer << Character(text_, white); return buffer;}
