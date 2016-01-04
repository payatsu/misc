/**
 * @file
 * @brief Program to generate 16 bpc(bits per component) PNG images.
 * @details
 * To compile this program, do as described below.
 * @code
 * $ g++ --std=c++14 16bpc_png_generator.cpp -lpng -lz
 * @endcode
 * Then, to run this program, do as described below.
 * @code
 * $ ./a.out
 * @endcode
 */

#define PNG_NO_SETJMP

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <vector>
#include <png.h>
#include <zlib.h>

constexpr png_uint_32 width  = 1920;
constexpr png_uint_32 height = 1200;
constexpr int bitdepth       = 16;
constexpr int colortype      = PNG_COLOR_TYPE_RGB;
constexpr int pixelsize      = 6;

struct FrameBuffer;

struct PatternGenerator{
	virtual const png_uint_32& width()const = 0;
	virtual const png_uint_32& height()const = 0;
	virtual void generate(FrameBuffer buffer)const = 0;
	virtual ~PatternGenerator() = default;
};

struct PseudoPixel{
	png_uint_16 R_;
	png_uint_16 G_;
	png_uint_16 B_;
	constexpr PseudoPixel(png_uint_16 R=0x0000, png_uint_16 G=0x0000, png_uint_16 B=0x0000):R_(R), G_(G), B_(B){}
	constexpr PseudoPixel operator+(const PseudoPixel& rhs)const{return {static_cast<png_uint_16>(R_+rhs.R_), static_cast<png_uint_16>(G_+rhs.G_), static_cast<png_uint_16>(B_+rhs.B_)};}
	constexpr PseudoPixel operator-(const PseudoPixel& rhs)const{return {static_cast<png_uint_16>(R_-rhs.R_), static_cast<png_uint_16>(G_-rhs.G_), static_cast<png_uint_16>(B_-rhs.B_)};}
	constexpr PseudoPixel operator*(png_uint_16 rhs)const{return {static_cast<png_uint_16>(R_*rhs), static_cast<png_uint_16>(G_*rhs), static_cast<png_uint_16>(B_*rhs)};}
	constexpr PseudoPixel operator/(png_uint_16 rhs)const{return {static_cast<png_uint_16>(R_/rhs), static_cast<png_uint_16>(G_/rhs), static_cast<png_uint_16>(B_/rhs)};}
};
constexpr PseudoPixel black  {0x0000, 0x0000, 0x0000};
constexpr PseudoPixel white  {0xffff, 0xffff, 0xffff};
constexpr PseudoPixel red    {0xffff, 0x0000, 0x0000};
constexpr PseudoPixel green  {0x0000, 0xffff, 0x0000};
constexpr PseudoPixel blue   {0x0000, 0x0000, 0xffff};
constexpr PseudoPixel cyan   {0x0000, 0xffff, 0xffff};
constexpr PseudoPixel magenta{0xffff, 0x0000, 0xffff};
constexpr PseudoPixel yellow {0xffff, 0xffff, 0x0000};

struct Painter{
	virtual PseudoPixel operator()() = 0;
	virtual ~Painter() = default;
};
struct UniColor: Painter{
	virtual PseudoPixel operator()()override{return pixel_;}
	constexpr UniColor(const PseudoPixel& pixel): pixel_(pixel){}
	const PseudoPixel pixel_;
};
struct RandomColor: Painter{
	virtual PseudoPixel operator()(){return {distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
	std::mt19937 engine_;
	std::uniform_int_distribution<png_uint_16> distribution_;
};
struct Gradator: Painter{
	const PseudoPixel step_;
	PseudoPixel state_;
	const bool invert_;
	constexpr Gradator(const PseudoPixel& step, const PseudoPixel& initial=black, bool invert=false): step_(step), state_(initial), invert_(invert){}
	PseudoPixel operator()()override
	{
		PseudoPixel tmp = state_;
		state_ = invert_ ? state_ - step_ : state_ + step_;
		return tmp;
	}
};

/**
 * @attention Big-Endian
 */
struct Pixel{
	png_byte Rmsb_;
	png_byte Rlsb_;
	png_byte Gmsb_;
	png_byte Glsb_;
	png_byte Bmsb_;
	png_byte Blsb_;
	constexpr Pixel(unsigned long long int binary): Rmsb_(binary>>40&0xff), Rlsb_(binary>>32&0xff), Gmsb_(binary>>24&0xff), Glsb_(binary>>16&0xff), Bmsb_(binary>>8&0xff), Blsb_(binary&0xff){}
	constexpr Pixel(const PseudoPixel& pseudo): Rmsb_(pseudo.R_>>8), Rlsb_(pseudo.R_&0xff), Gmsb_(pseudo.G_>>8), Glsb_(pseudo.G_&0xff), Bmsb_(pseudo.B_>>8), Blsb_(pseudo.B_&0xff){}
};

struct Row{
	const png_bytep row_;
	constexpr Row(png_bytep row): row_(row){}
	Pixel& operator[](int column)const{return *reinterpret_cast<Pixel*>(row_ + column*pixelsize);}
};
struct RowBlock{
	Pixel pixels_[::width]; // XXX
	static RowBlock* cast(Pixel* ptr){return reinterpret_cast<RowBlock*>(ptr);}
};

struct FrameBuffer{
	const png_bytep block_;
	const png_uint_32& width_;
	const png_uint_32& height_;
	const png_uint_32& width()const{return width_;}
	const png_uint_32& height()const{return height_;}
	constexpr FrameBuffer(png_bytep block, const png_uint_32& width, const png_uint_32& height): block_(block), width_(width), height_(height){}
	Row operator[](int row)const{return Row(block_ + row*width_*pixelsize);}
	const FrameBuffer& operator<<(const PatternGenerator& generator)const{generator.generate(*this); return *this;}
};

struct FixedSize: PatternGenerator{
	virtual const png_uint_32& width()const override final{return ::width;}
	virtual const png_uint_32& height()const override final{return ::height;}
};

struct ColorBar: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 x = width*3/4;
		std::fill(&buffer[0][0],             &buffer[0][width/8],       white  /100*40);
		std::fill(&buffer[0][width/8],       &buffer[0][width/8+x/7],   white  /100*75);
		std::fill(&buffer[0][width/8+x/7],   &buffer[0][width/8+x/7*2], yellow /100*75);
		std::fill(&buffer[0][width/8+x/7*2], &buffer[0][width/8+x/7*3], cyan   /100*75);
		std::fill(&buffer[0][width/8+x/7*3], &buffer[0][width/8+x/7*4], green  /100*75);
		std::fill(&buffer[0][width/8+x/7*4], &buffer[0][width/8+x/7*5], magenta/100*75);
		std::fill(&buffer[0][width/8+x/7*5], &buffer[0][width/8+x/7*6], red    /100*75);
		std::fill(&buffer[0][width/8+x/7*6], &buffer[0][width/8+x],     blue   /100*75);
		std::fill(&buffer[0][width/8+x],     &buffer[0][width],         white  /100*40);
		const png_uint_32 h1 = height*7/12;
		std::fill(RowBlock::cast(&buffer[1][0]), RowBlock::cast(&buffer[h1][0]), *RowBlock::cast(&buffer[0][0]));

		std::fill(&buffer[h1][0],           &buffer[h1][width/8],     cyan);
		std::fill(&buffer[h1][width/8],     &buffer[h1][width/8+x/7], white);
		std::fill(&buffer[h1][width/8+x/7], &buffer[h1][width/8+x],   white/100*75);
		std::fill(&buffer[h1][width/8+x],   &buffer[h1][width],       blue);
		const png_uint_32 h2 = h1 + height/12;
		std::fill(RowBlock::cast(&buffer[h1+1][0]), RowBlock::cast(&buffer[h2][0]), *RowBlock::cast(&buffer[h1][0]));

		std::fill(&buffer[h2][0],           &buffer[h2][width/8],   yellow);
		std::fill(&buffer[h2][width/8+x],   &buffer[h2][width],     red);
		std::generate(&buffer[h2][width/8], &buffer[h2][width/8+x], Gradator(white/x));
		const png_uint_32 h3 = h2 + height/12;
		std::fill(RowBlock::cast(&buffer[h2+1][0]), RowBlock::cast(&buffer[h3][0]), *RowBlock::cast(&buffer[h2][0]));

		std::fill(&buffer[h3][0],                     &buffer[h3][width/8],               white/100*15);
		std::fill(&buffer[h3][width/8],               &buffer[h3][width/8+x/7*3/2],       black);
		std::fill(&buffer[h3][width/8+x/7*3/2],       &buffer[h3][width/8+x/7*3/2+2*x/7], white);
		std::fill(&buffer[h3][width/8+x/7*3/2+2*x/7], &buffer[h3][width/8+x],             black);
		std::fill(&buffer[h3][width/8+x],             &buffer[h3][width],                 white/100*15);
		std::fill(RowBlock::cast(&buffer[h3+1][0]), RowBlock::cast(&buffer[height][0]), *RowBlock::cast(&buffer[h3][0]));
	}
};

struct Luster: FixedSize{
	virtual void generate(FrameBuffer buffer)const override{std::fill(&buffer[0][0], &buffer[buffer.height()][0], pixel_);}
	constexpr Luster(const Pixel& pixel): pixel_(pixel){}
	const Pixel pixel_;
};

struct Checker: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const auto pattern1 = invert_ ? black : white;
		const auto pattern2 = invert_ ? white : black;
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		std::fill(&buffer[0][0],         &buffer[0][width/4],   pattern1);
		std::fill(&buffer[0][width/4],   &buffer[0][width/4*2], pattern2);
		std::fill(&buffer[0][width/4*2], &buffer[0][width/4*3], pattern1);
		std::fill(&buffer[0][width/4*3], &buffer[0][width],     pattern2);
		std::fill(RowBlock::cast(&buffer[1][0]),          RowBlock::cast(&buffer[height/4][0]),   *RowBlock::cast(&buffer[0][0]));
		std::fill(RowBlock::cast(&buffer[height/4*2][0]), RowBlock::cast(&buffer[height/4*3][0]), *RowBlock::cast(&buffer[0][0]));

		std::fill(&buffer[height/4][0],         &buffer[height/4][width/4],   pattern2);
		std::fill(&buffer[height/4][width/4],   &buffer[height/4][width/4*2], pattern1);
		std::fill(&buffer[height/4][width/4*2], &buffer[height/4][width/4*3], pattern2);
		std::fill(&buffer[height/4][width/4*3], &buffer[height/4][width],     pattern1);
		std::fill(RowBlock::cast(&buffer[height/4+1][0]), RowBlock::cast(&buffer[height/4*2][0]), *RowBlock::cast(&buffer[height/4][0]));
		std::fill(RowBlock::cast(&buffer[height/4*3][0]), RowBlock::cast(&buffer[height][0]),     *RowBlock::cast(&buffer[height/4][0]));
	}
	const bool invert_;
	constexpr Checker(bool invert = false): invert_(invert){}
};

struct StairStepH: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 stair_height = height/stairs_;
		const png_uint_32 step_width = width/steps_ + (width%steps_ ? 1 : 0);
		bool invert = invert_;
		for(png_uint_32 row=0; row<height; row+=stair_height){
			Gradator gradator{white/steps_, invert ? white : black, invert};
			for(png_uint_32 column=0; column<width; column+=step_width){
				std::fill(&buffer[row][column], &buffer[row][std::min(width, column+step_width)], gradator());
			}
			std::fill(RowBlock::cast(&buffer[row+1][0]), RowBlock::cast(&buffer[std::min(height, row+stair_height)][0]), *RowBlock::cast(&buffer[row][0]));
			invert = !invert;
		}
	}
	const int stairs_;
	const int steps_;
	const bool invert_;
	constexpr StairStepH(int stairs=2, int steps=20): stairs_(stairs), steps_(steps), invert_(false){}
	constexpr StairStepH(bool invert): stairs_(1), steps_(20), invert_(invert){}
};

struct StairStepV: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 stair_width = width/stairs_;
		const png_uint_32 step_height = height/steps_ + (height%steps_ ? 1 : 0);
		bool invert = invert_;
		std::vector<Gradator> gradators;
		for(int i=0; i<stairs_; ++i){
			gradators.push_back({white/steps_, invert ? white : black, invert});
			invert = !invert;
		}
		for(png_uint_32 row=0; row<height; row+=step_height){
			for(png_uint_32 column=0; column<width; column+=stair_width){
				std::fill(&buffer[row][column], &buffer[row][std::min(width, column+stair_width)], gradators.at(column/stair_width)());
			}
			std::fill(RowBlock::cast(&buffer[row+1][0]), RowBlock::cast(&buffer[std::min(height, row+step_height)][0]), *RowBlock::cast(&buffer[row][0]));
		}
	}
	const int stairs_;
	const int steps_;
	const bool invert_;
	constexpr StairStepV(int stairs=2, int steps=20): stairs_(stairs), steps_(steps), invert_(false){}
	constexpr StairStepV(bool invert): stairs_(1), steps_(20), invert_(invert){}
};

struct Lamp: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
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

		std::fill(RowBlock::cast(&buffer[1][0]),              RowBlock::cast(&buffer[height/12][0]),    *RowBlock::cast(&buffer[0][0]));
		std::fill(RowBlock::cast(&buffer[height/12+1][0]),    RowBlock::cast(&buffer[height/12*2][0]),  *RowBlock::cast(&buffer[height/12][0]));
		std::fill(RowBlock::cast(&buffer[height/12*2+1][0]),  RowBlock::cast(&buffer[height/12*3][0]),  *RowBlock::cast(&buffer[height/12*2][0]));
		std::fill(RowBlock::cast(&buffer[height/12*3+1][0]),  RowBlock::cast(&buffer[height/12*4][0]),  *RowBlock::cast(&buffer[height/12*3][0]));
		std::fill(RowBlock::cast(&buffer[height/12*4+1][0]),  RowBlock::cast(&buffer[height/12*5][0]),  *RowBlock::cast(&buffer[height/12*4][0]));
		std::fill(RowBlock::cast(&buffer[height/12*5+1][0]),  RowBlock::cast(&buffer[height/12*6][0]),  *RowBlock::cast(&buffer[height/12*5][0]));
		std::fill(RowBlock::cast(&buffer[height/12*6+1][0]),  RowBlock::cast(&buffer[height/12*7][0]),  *RowBlock::cast(&buffer[height/12*6][0]));
		std::fill(RowBlock::cast(&buffer[height/12*7+1][0]),  RowBlock::cast(&buffer[height/12*8][0]),  *RowBlock::cast(&buffer[height/12*7][0]));
		std::fill(RowBlock::cast(&buffer[height/12*8+1][0]),  RowBlock::cast(&buffer[height/12*9][0]),  *RowBlock::cast(&buffer[height/12*8][0]));
		std::fill(RowBlock::cast(&buffer[height/12*9+1][0]),  RowBlock::cast(&buffer[height/12*10][0]), *RowBlock::cast(&buffer[height/12*9][0]));
		std::fill(RowBlock::cast(&buffer[height/12*10+1][0]), RowBlock::cast(&buffer[height/12*11][0]), *RowBlock::cast(&buffer[height/12*10][0]));
		std::fill(RowBlock::cast(&buffer[height/12*11+1][0]), RowBlock::cast(&buffer[height][0]),       *RowBlock::cast(&buffer[height/12*11][0]));
	}
};

struct CrossHatch: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		for(png_uint_32 i=0; i<height; i+=lattice_height_){
			std::fill(&buffer[i][0], &buffer[i][width], white);
		}
		std::fill(&buffer[height-1][0], &buffer[height][0], white);

		for(png_uint_32 i=0; i<width; i+=lattice_width_){
			for(png_uint_32 j=0; j<height; ++j){
				buffer[j][i] = white;
			}
		}
		for(png_uint_32 i=0; i<height; ++i){
			buffer[i][width-1] = white;
		}

		const double slope = static_cast<double>(height)/width;
		for(png_uint_32 i=0; i<width; ++i){
			buffer[slope*i][i] = white;
			buffer[height-slope*i][i] = white;
		}

		const png_uint_32 radius = height/2;
		const png_uint_32 shift_v = height/2;
		const png_uint_32 shift_h = width/2;
		for(double theta=0; theta<2*M_PI; theta+=2.0*M_PI/5000.0){
			png_uint_32 row    = std::min(height - 1, static_cast<png_uint_32>(shift_v + radius*std::sin(theta)));
			png_uint_32 column = std::min(width  - 1, static_cast<png_uint_32>(shift_h + radius*std::cos(theta)));
			buffer[row][column] = white;
		}
	}
	const png_uint_32 lattice_width_;
	const png_uint_32 lattice_height_;
	constexpr CrossHatch(png_uint_32 width, png_uint_32 height): lattice_width_(width), lattice_height_(height){}
};

struct WhiteNoise: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		RandomColor random_color;
		for(png_uint_32 row=0; row<buffer.height(); ++row){
			for(png_uint_32 column=0; column<buffer.width(); ++column){
				buffer[row][column] = random_color();
			}
		}
	}
};

// TODO ColorStep
// TODO Multi
// TODO Focus
// TODO Generators should not have width and/or height params.

constexpr unsigned char char_width  = 8; // dots
constexpr unsigned char char_height = 8; // dots
constexpr unsigned char char_tab_width = 4; // chars
constexpr unsigned char char_bitmask[8] = {
	0b10000000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
	0b00000001,
};
constexpr unsigned char characters[][8] = {
	{ // NUL
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // SOH
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // STX
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ETX
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // EOT
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ENQ
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ACK
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // BEL
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // BS
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // HT
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // LF
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // VT
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // FF
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // CR
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // SO
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // SI
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DLE
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DC1
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DC2
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DC3
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DC4
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // NAK
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // SYN
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ETB
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // CAN
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // EM
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // SUB
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ESC
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // FS
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // GS
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // RS
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // US
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // ' '
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // !
		0b00000000,
		0b00111000,
		0b00111000,
		0b00111000,
		0b00111000,
		0b00010000,
		0b00000000,
		0b00010000,
	},{ // "
		0b00000000,
		0b01101100,
		0b01101100,
		0b00100100,
		0b01001000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // #
		0b00000000,
		0b01001000,
		0b11111100,
		0b01001000,
		0b01001000,
		0b01001000,
		0b11111100,
		0b01001000,
	},{ // $
		0b00010000,
		0b01111100,
		0b10010010,
		0b01110000,
		0b00011100,
		0b10010010,
		0b01111100,
		0b00010000,
	},{ // %
		0b00000000,
		0b11100100,
		0b10100100,
		0b11101000,
		0b00010000,
		0b00101110,
		0b01001010,
		0b01001110,
	},{ // &
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111000,
		0b01000101,
		0b01000010,
		0b00111101,
	},{ // '
		0b00000000,
		0b00110000,
		0b00110000,
		0b00010000,
		0b00100000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // (
		0b00000110,
		0b00011000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00011000,
		0b00000110,
	},{ // )
		0b01100000,
		0b00011000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00011000,
		0b01100000,
	},{ // *
		0b00000000,
		0b00010000,
		0b01010100,
		0b00111000,
		0b00010000,
		0b00111000,
		0b01010100,
		0b00010000,
	},{ // +
		0b00000000,
		0b00000000,
		0b00010000,
		0b00010000,
		0b01111100,
		0b00010000,
		0b00010000,
		0b00000000,
	},{ // ,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01100000,
		0b00100000,
		0b01000000,
	},{ // -
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01111100,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // .
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01100000,
		0b01100000,
		0b00000000,
	},{ // /
		0b00001000,
		0b00001000,
		0b00010000,
		0b00010000,
		0b00100000,
		0b00100000,
		0b01000000,
		0b01000000,
	},{ // 0
		0b00000000,
		0b00111000,
		0b01000100,
		0b01001100,
		0b01010100,
		0b01100100,
		0b01000100,
		0b00111000,
	},{ // 1
		0b00000000,
		0b00010000,
		0b01110000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b01111100,
	},{ // 2
		0b00000000,
		0b00111000,
		0b01000100,
		0b00000100,
		0b00011000,
		0b00100000,
		0b01000000,
		0b01111100,
	},{ // 3
		0b00000000,
		0b00111000,
		0b01000100,
		0b00000100,
		0b00111000,
		0b00000100,
		0b01000100,
		0b00111000,
	},{ // 4
		0b00000000,
		0b00001000,
		0b00011000,
		0b00101000,
		0b01001000,
		0b01111100,
		0b00001000,
		0b00001000,
	},{ // 5
		0b00000000,
		0b01111000,
		0b01000000,
		0b01000000,
		0b01111000,
		0b00000100,
		0b01000100,
		0b00111000,
	},{ // 6
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // 7
		0b00000000,
		0b01111100,
		0b01000100,
		0b00000100,
		0b00001000,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // 8
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // 9
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111100,
		0b00000100,
		0b01000100,
		0b00111000,
	},{ // :
		0b00000000,
		0b00110000,
		0b00110000,
		0b00000000,
		0b00110000,
		0b00110000,
		0b00000000,
		0b00000000,
	},{ /// ;
		0b00000000,
		0b00110000,
		0b00110000,
		0b00000000,
		0b00110000,
		0b00110000,
		0b00010000,
		0b00100000,
	},{ // <
		0b00000000,
		0b00001100,
		0b00010000,
		0b00100000,
		0b01000000,
		0b00100000,
		0b00010000,
		0b00001100,
	},{ // =
		0b00000000,
		0b00000000,
		0b01111100,
		0b00000000,
		0b00000000,
		0b01111100,
		0b00000000,
		0b00000000,
	},{ // >
		0b00000000,
		0b01100000,
		0b00010000,
		0b00001000,
		0b00000100,
		0b00001000,
		0b00010000,
		0b01100000,
	},{ // ?
		0b00000000,
		0b00111000,
		0b01000100,
		0b00000100,
		0b00011000,
		0b00010000,
		0b00000000,
		0b00010000,
	},{ // @
		0b00000000,
		0b01111100,
		0b10000010,
		0b10110010,
		0b10001010,
		0b00111010,
		0b01001010,
		0b00110100,
	},{ // A
		0b00000000,
		0b00010000,
		0b00101000,
		0b00101000,
		0b01000100,
		0b01111100,
		0b01000100,
		0b01000100,
	},{ // B
		0b00000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
	},{ // C
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000100,
		0b00111000,
	},{ // D
		0b00000000,
		0b01111000,
		0b01000100,
		0b01000010,
		0b01000010,
		0b01000010,
		0b01000100,
		0b01111000,
	},{ // E
		0b00000000,
		0b01111100,
		0b01000000,
		0b01000000,
		0b01111000,
		0b01000000,
		0b01000000,
		0b01111100,
	},{ // F
		0b00000000,
		0b01111100,
		0b01000000,
		0b01000000,
		0b01111000,
		0b01000000,
		0b01000000,
		0b01000000,
	},{ // G
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000000,
		0b01011100,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // H
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b01111100,
		0b01000100,
		0b01000100,
		0b01000100,
	},{ // I
		0b00000000,
		0b00111000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00111000,
	},{ // J
		0b00000000,
		0b00011110,
		0b00000100,
		0b00000100,
		0b00000100,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // K
		0b00000000,
		0b01000100,
		0b01001000,
		0b01001000,
		0b01010000,
		0b01101000,
		0b01000100,
		0b01000010,
	},{ // L
		0b00000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01111000,
	},{ // M
		0b00000000,
		0b10000010,
		0b11000110,
		0b10101010,
		0b10010010,
		0b10010010,
		0b10000010,
		0b10000010,
	},{ // N
		0b00000000,
		0b01100010,
		0b01010010,
		0b01010010,
		0b01001010,
		0b01001010,
		0b01000110,
		0b01000010,
	},{ // O
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b01010100,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // P
		0b00000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
		0b01000000,
		0b01000000,
		0b01000000,
	},{ // Q
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b01010100,
		0b01010100,
		0b01001100,
		0b00111010,
	},{ // R
		0b00000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
		0b01001000,
		0b01000100,
		0b01000100,
	},{ // S
		0b00000000,
		0b01111000,
		0b10000100,
		0b10000000,
		0b01111000,
		0b00000100,
		0b10000100,
		0b01111000,
	},{ // T
		0b00000000,
		0b01111100,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // U
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // V
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00101000,
		0b00101000,
		0b00010000,
	},{ // W
		0b00000000,
		0b01000100,
		0b01000100,
		0b01010100,
		0b01010100,
		0b01010100,
		0b00101000,
		0b00101000,
	},{ // X
		0b00000000,
		0b01000100,
		0b00101000,
		0b00010000,
		0b00010000,
		0b00101000,
		0b00101000,
		0b01000100,
	},{ // Y
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // Z
		0b00000000,
		0b11111100,
		0b00001000,
		0b00010000,
		0b00100000,
		0b01000000,
		0b10000000,
		0b11111100,
	},{ // [
		0b00000000,
		0b00011110,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00011110,
	},{ // '\'
		0b01000000,
		0b01000000,
		0b00100000,
		0b00100000,
		0b00010000,
		0b00010000,
		0b00001000,
		0b00001000,
	},{ // ]
		0b00000000,
		0b01111000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b01111000,
	},{ // ^
		0b00000000,
		0b00010000,
		0b00101000,
		0b01000100,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // _
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01111100,
	},{ // `
		0b00000000,
		0b00011000,
		0b00000100,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // a
		0b00000000,
		0b00000000,
		0b00011000,
		0b00000100,
		0b00011100,
		0b00100100,
		0b00100100,
		0b00111010,
	},{ // b
		0b00000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
	},{ // c
		0b00000000,
		0b00000000,
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000000,
		0b01000100,
		0b00111000,
	},{ // d
		0b00000000,
		0b00000100,
		0b00000100,
		0b00000100,
		0b00111100,
		0b01000100,
		0b01000100,
		0b00111100,
	},{ // e
		0b00000000,
		0b00000000,
		0b00000000,
		0b00111000,
		0b01000100,
		0b01111100,
		0b01000000,
		0b00111000,
	},{ // f
		0b00000000,
		0b00001000,
		0b00010100,
		0b00010000,
		0b00111100,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // g
		0b00000000,
		0b00111010,
		0b01000100,
		0b01111000,
		0b01000000,
		0b01111000,
		0b01000100,
		0b00111000,
	},{ // h
		0b00000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01011000,
		0b01100100,
		0b01000100,
		0b01000100,
	},{ // i
		0b00000000,
		0b00010000,
		0b00010000,
		0b00000000,
		0b00110000,
		0b00010000,
		0b00010000,
		0b00011000,
	},{ // j
		0b00000000,
		0b00011000,
		0b00000000,
		0b00111000,
		0b00001000,
		0b00001000,
		0b01001000,
		0b00110000,
	},{ // k
		0b00000000,
		0b01000000,
		0b01000000,
		0b01001000,
		0b01010000,
		0b01100000,
		0b01010000,
		0b01001000,
	},{ // l
		0b00000000,
		0b01100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00111000,
	},{ // m
		0b00000000,
		0b00000000,
		0b00000000,
		0b10101000,
		0b01010100,
		0b01010100,
		0b01010100,
		0b01010100,
	},{ // n
		0b00000000,
		0b00000000,
		0b00000000,
		0b11011000,
		0b01100100,
		0b01000100,
		0b01000100,
		0b01000100,
	},{ // o
		0b00000000,
		0b00000000,
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // p
		0b00000000,
		0b00000000,
		0b01011000,
		0b01100100,
		0b01100100,
		0b01011000,
		0b01000000,
		0b01000000,
	},{ // q
		0b00000000,
		0b00000000,
		0b00110100,
		0b01001100,
		0b01001100,
		0b00110100,
		0b00000100,
		0b00000100,
	},{ // r
		0b00000000,
		0b00000000,
		0b00000000,
		0b01011000,
		0b01100000,
		0b01000000,
		0b01000000,
		0b01000000,
	},{ // s
		0b00000000,
		0b00000000,
		0b00000000,
		0b01111000,
		0b10000000,
		0b01111000,
		0b00000100,
		0b01111000,
	},{ // t
		0b00000000,
		0b00000000,
		0b00000000,
		0b00100000,
		0b01111000,
		0b00100000,
		0b00100100,
		0b00011000,
	},{ // u
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00111010,
	},{ // v
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00101000,
		0b00010000,
	},{ // w
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01000100,
		0b01010100,
		0b01010100,
		0b00101000,
	},{ // x
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b00101000,
		0b00010000,
		0b00101000,
		0b01000100,
	},{ // y
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01000100,
		0b00111100,
		0b00000100,
		0b01111000,
	},{ // z
		0b00000000,
		0b00000000,
		0b00000000,
		0b01111000,
		0b00010000,
		0b00100000,
		0b01000000,
		0b01111000,
	},{ // {
		0b00000000,
		0b00000110,
		0b00001000,
		0b00001000,
		0b00010000,
		0b00001000,
		0b00001000,
		0b00000110,
	},{ // |
		0b00000000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // }
		0b00000000,
		0b01100000,
		0b00010000,
		0b00010000,
		0b00001000,
		0b00010000,
		0b00010000,
		0b01100000,
	},{ // ~
		0b00000000,
		0b00101000,
		0b01010000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // DEL
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},
};

struct Character: FixedSize{
	virtual void generate(FrameBuffer buffer)const override
	{
		write(buffer, row_, column_, text_, pixel_, scale_);
	}
	void write(FrameBuffer buffer, png_uint_32 row, png_uint_32 column, unsigned char c, const Pixel& pixel, int scale)const
	{
		if('~' < c || buffer.height() <= row || buffer.width() <= column){
			std::cerr << "warning: not supported: row: " << row << ", col: " << column << ", ascii: " << c << '(' << int(c) << ')' << std::endl;
			return;
		}
		for(unsigned char i=0; i<char_height; ++i){
			for(unsigned char j=0; j<char_width; ++j){
				if(characters[c][i] & char_bitmask[j]){
					for(int k=0; k<scale; ++k){
						for(int l=0; l<scale; ++l){
							buffer[row+i*scale+k][column+j*scale+l] = pixel;
						}
					}
				}
			}
		}
	}
	void write(FrameBuffer buffer, png_uint_32 row, png_uint_32 column, const std::string& str, const Pixel& pixel, int scale)const
	{
		for(std::string::size_type i=0, j=0; i<str.size(); ++i){
			if(str[i] == '\n'){
				row += scale*char_height;
				j = 0;
				continue;
			}else if(str[i] == '\t'){
				j += char_tab_width;
				continue;
			}
			write(buffer, row, column+j*scale*char_width, str[i], pixel, scale);
			++j;
		}
	}
	const std::string text_;
	const Pixel pixel_;
	const int scale_;
	const png_uint_32 row_;
	const png_uint_32 column_;
	Character(const std::string& text, const Pixel& pixel=white, int scale=1, png_uint_32 row=0, png_uint_32 column=0): text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
};

struct TypeWriter: PatternGenerator{
	virtual const png_uint_32& width()const override{return width_;}
	virtual const png_uint_32& height()const override{return height_;}
	virtual void generate(FrameBuffer buffer)const override
	{
		buffer << Character(text_, white);
	}
	png_uint_32 width_;
	png_uint_32 height_;
	std::string text_;
	TypeWriter(const std::string& textfilename): width_(), height_(), text_()
	{
		std::ifstream ifs(textfilename);
		std::string line;
		while(std::getline(ifs, line)){
			width_ = std::max(width_, static_cast<png_uint_32>(line.size() + (char_tab_width-1)*std::count_if(line.begin(), line.end(), [](char c){return c == '\t';})));
			text_ += line + '\n';
			++height_;
		}
		width_ *= char_width;
		height_ *= char_height;
	}
};

struct CSVLoader: PatternGenerator{
	virtual const png_uint_32& width()const override{return width_;}
	virtual const png_uint_32& height()const override{return height_;}
	virtual void generate(FrameBuffer buffer)const override{std::copy(pixels_.begin(), pixels_.end(), &buffer[0][0]);}
	png_uint_32 width_;
	png_uint_32 height_;
	std::vector<Pixel> pixels_;
	CSVLoader(const std::string& filename): CSVLoader(std::istringstream(read(filename))){}
	CSVLoader(std::istream&& is): width_(), height_(), pixels_()
	{
		std::string line;
		while(std::getline(is, line)){
			std::replace(line.begin(), line.end(), ',', ' ');
			std::istringstream iss(line);
			std::string token;
			width_ = 0;
			while(iss >> token){
				pixels_.push_back(std::stoull(token, nullptr, 0));
				++width_;
			}
			++height_;
		}
	}
	std::string read(const std::string& filename)
	{
		gzFile_s* fp = gzopen(filename.c_str(), "rb");
		if(!fp){
			std::perror("");
			return "";
		}
		constexpr unsigned int buffer_size = 1024 * 256;
		if(gzbuffer(fp, buffer_size)){
			std::perror("");
			return "";
		}
		char buffer[buffer_size];
		unsigned int len = 0;
		std::string str;
		while((len = gzread(fp, buffer, buffer_size)) > 0){
			str.append(buffer, len);
		}
		gzclose(fp);
		return str;
	}
};

/// @deprecated
struct Overlayer: PatternGenerator{
	virtual const png_uint_32& width()const override{return first_.width();}
	virtual const png_uint_32& height()const override{return first_.height();}
	virtual void generate(FrameBuffer buffer)const override{buffer << first_ << second_;}
	Overlayer(const PatternGenerator& first, const PatternGenerator& second): first_(first), second_(second){}
	const PatternGenerator& first_;
	const PatternGenerator& second_;
};

struct Overwriter: PatternGenerator{
	virtual const png_uint_32& width()const override{return generators_.at(0)->width();}
	virtual const png_uint_32& height()const override{return generators_.at(0)->height();}
	virtual void generate(FrameBuffer buffer)const override{for(const auto& g: generators_){buffer << *g;}}

	Overwriter(const std::vector<std::shared_ptr<const PatternGenerator>>& generators):generators_(generators){}
	const std::vector<std::shared_ptr<const PatternGenerator>>& generators_;
};

void generate_16bpc_png(const std::string& output_filename, const PatternGenerator& generator)
{
	std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen(output_filename.c_str(), "wb"), std::fclose);
	if(!fp){
		std::perror("");
		return;
	}
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(!png_ptr){
		return;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_write_struct(&png_ptr, nullptr);
		return;
	}
	png_init_io(png_ptr, fp.get());
	png_set_IHDR(png_ptr, info_ptr, generator.width(), generator.height(), bitdepth, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	std::unique_ptr<png_byte[]> block(new png_byte[generator.height()*generator.width()*pixelsize]);
	std::unique_ptr<png_bytep[]> row_ptrs(new png_bytep[generator.height()]);
	for(png_uint_32 i=0; i<generator.height(); ++i){
		row_ptrs[i] = block.get() + i*generator.width()*pixelsize;
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs.get());
	generator.generate(FrameBuffer(block.get(), generator.width(), generator.height()));
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

int main(int argc, char* argv[])
{
	if(1<argc && std::string("builtins") == argv[1]){
		generate_16bpc_png("colorbar.png",    ColorBar());
		generate_16bpc_png("white100.png",    Luster(white));
		generate_16bpc_png("red100.png",      Luster(red));
		generate_16bpc_png("green100.png",    Luster(green));
		generate_16bpc_png("blue100.png",     Luster(blue));
		generate_16bpc_png("white50.png",     Luster(white/2));
		generate_16bpc_png("red50.png",       Luster(red  /2));
		generate_16bpc_png("green50.png",     Luster(green/2));
		generate_16bpc_png("blue50.png",      Luster(blue /2));
		generate_16bpc_png("checker1.png",    Checker());
		generate_16bpc_png("checker2.png",    Checker(true));
		generate_16bpc_png("stairstepH1.png", StairStepH());
		generate_16bpc_png("stairstepH2.png", StairStepH(false));
		generate_16bpc_png("stairstepH3.png", StairStepH(true));
		generate_16bpc_png("stairstepV1.png", StairStepV());
		generate_16bpc_png("stairstepV2.png", StairStepV(false));
		generate_16bpc_png("stairstepV3.png", StairStepV(true));
		generate_16bpc_png("lamp.png",        Lamp());
		generate_16bpc_png("crosshatch.png",  Overwriter({std::make_shared<Luster>(black), std::make_shared<CrossHatch>(192, 108)}));
		generate_16bpc_png("character.png",   Overwriter({std::make_shared<Luster>(black), std::make_shared<Character>(" !\"#$%&'()*+,-./\n0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\nabcdefghijklmno\npqrstuvwxyz{|}~", red, 10)}));
		generate_16bpc_png("sourcecode.png",  TypeWriter("16bpc_png_generator.cpp"));
		generate_16bpc_png("whitenoise.png",     WhiteNoise());
	}
	if(std::ifstream("userdefined.csv")){
		generate_16bpc_png("userdefined.png", CSVLoader("userdefined.csv"));
	}else if(std::ifstream("userdefined.csv.gz")){
		generate_16bpc_png("userdefined.png", CSVLoader("userdefined.csv.gz"));
	}
	if(std::ifstream("happy_new_year.csv.gz")){
		generate_16bpc_png("happy_new_year.png", Overwriter({std::make_shared<CSVLoader>("happy_new_year.csv.gz"), std::make_shared<Character>("Happy New Year!!", black, 15, height/2 - char_height*15/2, 0)}));
	}
	return 0;
}
