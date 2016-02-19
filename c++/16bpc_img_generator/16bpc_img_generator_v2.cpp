/**
 * @file
 * @brief Program to generate 16 bpc(bits per component) PNG/TIFF images.
 * @details
 * To compile this program, do as described below.
 * @code
 * $ g++ -DENABLE_PNG -DENABLE_TIFF 16bpc_img_generator.cpp -lpng -ltiff -lz
 * @endcode
 * Then, run like this:
 * @code
 * $ ./a.out width=1920 height=1080 input=/dev/zero output=hoge
 * @endcode
 */

// TODO ColorStep
// TODO Multi
// TODO Focus

#define _USE_MATH_DEFINES
#define PNG_NO_SETJMP

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#if 201103L <= __cplusplus
#	include <random>
#endif
#include <sstream>
#include <vector>
#ifdef ENABLE_PNG
#	include <png.h>
#endif
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
// #include <zlib.h>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const int pixelsize = 6;

class FrameBuffer;

class PatternGenerator{
public:
	virtual ~PatternGenerator(){}
	virtual void generate(FrameBuffer& buffer)const = 0;
};

class Pixel{
public:
	Pixel(uint16_t R = 0x0000, uint16_t G = 0x0000, uint16_t B = 0x0000): R_(R), G_(G), B_(B){}
	Pixel(unsigned long long int binary): R_(binary>>32&0xffff), G_(binary>>16&0xffff), B_(binary&0xffff){}
	Pixel operator+(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ + rhs.R_),
			static_cast<uint16_t>(G_ + rhs.G_),
			static_cast<uint16_t>(B_ + rhs.B_));
	}
	Pixel operator-(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ - rhs.R_),
			static_cast<uint16_t>(G_ - rhs.G_),
			static_cast<uint16_t>(B_ - rhs.B_));
	}
	Pixel operator*(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_*rhs),
			static_cast<uint16_t>(G_*rhs),
			static_cast<uint16_t>(B_*rhs));
	}
	Pixel operator/(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_/rhs),
			static_cast<uint16_t>(G_/rhs),
			static_cast<uint16_t>(B_/rhs));
	}
private:
	uint16_t R_;
	uint16_t G_;
	uint16_t B_;
};
const Pixel black  (0x0000, 0x0000, 0x0000);
const Pixel white  (0xffff, 0xffff, 0xffff);
const Pixel red    (0xffff, 0x0000, 0x0000);
const Pixel green  (0x0000, 0xffff, 0x0000);
const Pixel blue   (0x0000, 0x0000, 0xffff);
const Pixel cyan   (0x0000, 0xffff, 0xffff);
const Pixel magenta(0xffff, 0x0000, 0xffff);
const Pixel yellow (0xffff, 0xffff, 0x0000);

class Row{
public:
	Row(uint8_t* row, const uint32_t& width): row_(row), width_(width){}
	const uint32_t& width()const{return width_;}
	Pixel& operator[](int column)const{return *reinterpret_cast<Pixel*>(row_ + column*pixelsize);}
	Row& operator++(){row_ += width()*pixelsize; return *this;}
	bool operator!=(const Row& rhs)const{return this->row_ != rhs.row_;}
	static void fill(Row first, Row last, const Row& row)
	{
		while(first != last){
			std::copy(&row[0], &row[row.width()], &first[0]);
			++first;
		}
	}
private:
	uint8_t* row_;
	const uint32_t& width_;
};

class FrameBuffer{
public:
	FrameBuffer(const uint32_t& width, const uint32_t& height):
		head_(new uint8_t[height*width*pixelsize]), width_(width), height_(height){}
	~FrameBuffer(){delete[] head_;}
	Row operator[](int row)const{return Row(head_ + row*width()*pixelsize, width());}
	FrameBuffer& operator<<(const PatternGenerator& generator){generator.generate(*this); return *this;}
	FrameBuffer& operator<<(std::istream& is)
	{
		const int size = height()*width()*pixelsize;
		for(int i = 0; i < size; ++i){
			int c = is.get();
			if(is.eof()){
				break;
			}
			head_[i] = c;
		}
		return *this;
	}
	uint8_t* head()const{return head_;}
	const uint32_t& width()const{return width_;}
	const uint32_t& height()const{return height_;}
private:
	FrameBuffer(const FrameBuffer&);
	FrameBuffer& operator=(const FrameBuffer&);
	uint8_t* head_;
	const uint32_t& width_;
	const uint32_t& height_;
};

class Painter{
public:
	virtual ~Painter(){}
	virtual Pixel operator()() = 0;
};

class UniColor: Painter{
public:
	UniColor(const Pixel& pixel): pixel_(pixel){}
	virtual Pixel operator()(){return pixel_;}
private:
	const Pixel pixel_;
};

class Gradator: Painter{
public:
	Gradator(const Pixel& step, const Pixel& initial=black, bool invert=false):
		step_(step), state_(initial), invert_(invert){}
	Pixel operator()()
	{
		Pixel tmp = state_;
		state_ = invert_ ? state_ - step_ : state_ + step_;
		return tmp;
	}
private:
	Pixel step_;
	Pixel state_;
	bool invert_;
};

#if 201103L <= __cplusplus
class RandomColor: Painter{
public:
	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
	virtual Pixel operator()(){return {distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
private:
	std::mt19937 engine_;
	std::uniform_int_distribution<uint16_t> distribution_;
};
#endif

class ColorBar: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const
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
	}
};

class Luster: public PatternGenerator{
public:
	Luster(const Pixel& pixel): pixel_(pixel){}
	virtual void generate(FrameBuffer& buffer)const{std::fill(&buffer[0][0], &buffer[buffer.height()][0], pixel_);}
private:
	const Pixel pixel_;
};

class Checker: public PatternGenerator{
public:
	Checker(bool invert = false): invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const
	{
		const Pixel pattern1 = invert_ ? black : white;
		const Pixel pattern2 = invert_ ? white : black;
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
	}
private:
	const bool invert_;
};

class StairStepH: public PatternGenerator{
public:
	StairStepH(int stairs = 2, int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const
	{
		const uint32_t width = buffer.width();
		const uint32_t height = buffer.height();
		const uint32_t stair_height = height/stairs_;
		const uint32_t step_width = width/steps_ + (width%steps_ ? 1 : 0);
		bool invert = invert_;
		for(uint32_t row = 0; row < height; row += stair_height){
			Gradator gradator(white/steps_, invert ? white : black, invert);
			for(uint32_t column = 0; column < width; column += step_width){
				std::fill(&buffer[row][column], &buffer[row][std::min(width, column+step_width)],
						gradator());
			}
			Row::fill(buffer[row + 1], buffer[std::min(height, row+stair_height)], buffer[row]);
			invert = !invert;
		}
	}
private:
	const int stairs_;
	const int steps_;
	const bool invert_;
};

class StairStepV: public PatternGenerator{
public:
	StairStepV(int stairs = 2, int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const
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
				std::fill(&buffer[row][column], &buffer[row][std::min(width, column+stair_width)],
						gradators.at(column/stair_width)());
			}
			Row::fill(buffer[row + 1], buffer[std::min(height, row+step_height)], buffer[row]);
		}
	}
private:
	const int stairs_;
	const int steps_;
	const bool invert_;
};

class Ramp: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const
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
	}
};

class CrossHatch: public PatternGenerator{
public:
	CrossHatch(uint32_t width, uint32_t height): lattice_width_(width), lattice_height_(height){}
	virtual void generate(FrameBuffer& buffer)const
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
	}
private:
	const uint32_t lattice_width_;
	const uint32_t lattice_height_;
};

#if 201103L <= __cplusplus
class WhiteNoise: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const
	{
		RandomColor random_color;
		for(uint32_t row = 0; row < buffer.height(); ++row){
			for(uint32_t column = 0; column < buffer.width(); ++column){
				buffer[row][column] = random_color();
			}
		}
	}
};
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

class Character: public PatternGenerator{
public:
	Character(const std::string& text, const Pixel& pixel = white,
			int scale = 1, uint32_t row = 0, uint32_t column = 0):
		text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
	virtual void generate(FrameBuffer& buffer)const{write(buffer, row_, column_, text_, pixel_, scale_);}
	void write(FrameBuffer& buffer, uint32_t row, uint32_t column,
			unsigned char c, const Pixel& pixel, int scale)const
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
	void write(FrameBuffer& buffer, uint32_t row, uint32_t column,
			const std::string& str, const Pixel& pixel, int scale)const
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
private:
	const std::string text_;
	const Pixel pixel_;
	const int scale_;
	const uint32_t row_;
	const uint32_t column_;
};

class TypeWriter: public PatternGenerator{
public:
	TypeWriter(const std::string& textfilename): width_(), height_(), text_()
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
	virtual const uint32_t& width()const{return width_;}
	virtual const uint32_t& height()const{return height_;}
	virtual void generate(FrameBuffer& buffer)const{buffer << Character(text_, white);}
private:
	static bool is_tab(char c){return c == '\t';}
	uint32_t width_;
	uint32_t height_;
	std::string text_;
};

//class CSVLoader: public PatternGenerator{
//public:
//	CSVLoader(const std::string& filename): CSVLoader(std::istringstream(read(filename))){}
//	CSVLoader(std::istream&& is): width_(), height_(), pixels_()
//	{
//		std::string line;
//		while(std::getline(is, line)){
//			std::replace(line.begin(), line.end(), ',', ' ');
//			std::istringstream iss(line);
//			std::string token;
//			width_ = 0;
//			while(iss >> token){
//				pixels_.push_back(std::stoull(token, NULL, 0));
//				++width_;
//			}
//			++height_;
//		}
//	}
//	virtual const uint32_t& width()const{return width_;}
//	virtual const uint32_t& height()const{return height_;}
//	virtual void generate(FrameBuffer& buffer)const{std::copy(pixels_.begin(), pixels_.end(), &buffer[0][0]);}
//private:
//	std::string read(const std::string& filename)
//	{
//		gzFile_s* fp = gzopen(filename.c_str(), "rb");
//		if(!fp){
//			std::perror("");
//			return "";
//		}
//		const unsigned int buffer_size = 1024 * 256;
//		if(gzbuffer(fp, buffer_size)){
//			std::perror("");
//			return "";
//		}
//		char buffer[buffer_size];
//		unsigned int len = 0;
//		std::string str;
//		while((len = gzread(fp, buffer, buffer_size)) > 0){
//			str.append(buffer, len);
//		}
//		gzclose(fp);
//		return str;
//	}
//	uint32_t width_;
//	uint32_t height_;
//	std::vector<Pixel> pixels_;
//};

#ifdef ENABLE_TIFF
void generate_16bpc_tiff(const std::string& output_filename, FrameBuffer& buffer)
{
	TIFF* image = TIFFOpen(output_filename.c_str(), "w");
	if(!image){
		perror("");
		return;
	}
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, buffer.width());
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, buffer.height());
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, bitdepth);
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, buffer.height());
	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(image, TIFFTAG_XRESOLUTION, 150.0);
	TIFFSetField(image, TIFFTAG_YRESOLUTION, 150.0);
	TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFWriteEncodedStrip(image, 0, buffer.head(),
			buffer.height()*buffer.width()*pixelsize);
	TIFFClose(image);
}
#endif

uint16_t swap_msb_lsb(uint16_t value)
{
	return value >> 8 | value << 8;
}

#ifdef ENABLE_PNG
void generate_16bpc_png(const std::string& output_filename, FrameBuffer& buffer)
{
	FILE* fp = std::fopen(output_filename.c_str(), "wb");
	if(!fp){
		std::perror("");
		return;
	}
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		return;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_write_struct(&png_ptr, NULL);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, buffer.width(), buffer.height(),
			bitdepth, colortype, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	uint8_t** row_ptrs = new uint8_t*[buffer.height()];
	for(uint32_t i = 0; i < buffer.height(); ++i){
		row_ptrs[i] = buffer.head() + i*buffer.width()*pixelsize;
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs);
	std::transform(
			reinterpret_cast<uint16_t*>(buffer.head()),
			reinterpret_cast<uint16_t*>(buffer.head() + buffer.height()*buffer.width()*pixelsize),
			reinterpret_cast<uint16_t*>(buffer.head()), swap_msb_lsb);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete[] row_ptrs;
	std::fclose(fp);
}
#endif

void generate_16bpc_img(const std::string& output_basename, FrameBuffer& buffer)
{
#ifdef ENABLE_TIFF
	generate_16bpc_tiff(output_basename + ".tif", buffer);
#endif
#ifdef ENABLE_PNG
	generate_16bpc_png(output_basename + ".png", buffer);
#endif
}

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

typedef std::map<std::string, std::string> Store;
bool is_equal(char c){return c == '=';}

Store getopt(int argc, char* argv[])
{
	Store store;
	for(int i = 1; i < argc; ++i){
		if(std::count_if(argv[i], argv[i] + std::strlen(argv[i]), is_equal) == 1){
			const std::string::size_type idx = std::string(argv[i]).find('=');
			const std::string key(argv[i], idx);
			const std::string value(argv[i] + idx + 1);
			store[key] = value;
		}
	}
	return store;
}

std::string append_extension(const std::string& filename, const std::string& ext)
{
	const std::string::size_type idx = filename.find(ext);
	if(idx == std::string::npos || idx + ext.size() != filename.size()){
		return filename + ext;
	}else{
		return filename;
	}
}

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	const uint32_t height = store["height"] == "" ? 1080 : atoi(store["height"].c_str());
	const uint32_t width = store["width"] == "" ? 1920 : atoi(store["width"].c_str());

	if(store["builtins"] != ""){
		generate_builtin_patterns(width, height);
		return 0;
	}

	FrameBuffer buffer(width, height);
	if(store["input"] == ""){
		buffer << std::cin;
	}else{
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
