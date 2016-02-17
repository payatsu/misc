/**
 * @file
 * @brief Program to generate 16 bpc(bits per component) PNG/TIFF images.
 * @details
 * To compile this program, do as described below.
 * @code
 * $ g++ 16bpc_img_generator.cpp -lpng -ltiff -lz
 * @endcode
 * Then, run like this:
 * @code
 * $ ./a.out width=1920 height=1080 input=/dev/zero output=hoge
 * @endcode
 */

// TODO ColorStep
// TODO Multi
// TODO Focus

#define PNG_NO_SETJMP

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
// #include <random>
#include <sstream>
#include <vector>
#include <png.h>
#include <tiffio.h>
// #include <zlib.h>

int bitdepth  = 16;
int colortype = PNG_COLOR_TYPE_RGB;
int pixelsize = 6;

class FrameBuffer;

class PatternGenerator{
public:
	virtual ~PatternGenerator(){}
	virtual void generate(FrameBuffer& buffer)const = 0;
};

class Pixel{
public:
	Pixel(png_uint_16 R = 0x0000, png_uint_16 G = 0x0000, png_uint_16 B = 0x0000): R_(R), G_(G), B_(B){}
	Pixel(unsigned long long int binary): R_(binary>>32&0xffff), G_(binary>>16&0xffff), B_(binary&0xffff){}
	Pixel operator+(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<png_uint_16>(R_ + rhs.R_),
			static_cast<png_uint_16>(G_ + rhs.G_),
			static_cast<png_uint_16>(B_ + rhs.B_));
	}
	Pixel operator-(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<png_uint_16>(R_ - rhs.R_),
			static_cast<png_uint_16>(G_ - rhs.G_),
			static_cast<png_uint_16>(B_ - rhs.B_));
	}
	Pixel operator*(png_uint_16 rhs)const
	{
		return Pixel(
			static_cast<png_uint_16>(R_*rhs),
			static_cast<png_uint_16>(G_*rhs),
			static_cast<png_uint_16>(B_*rhs));
	}
	Pixel operator/(png_uint_16 rhs)const
	{
		return Pixel(
			static_cast<png_uint_16>(R_/rhs),
			static_cast<png_uint_16>(G_/rhs),
			static_cast<png_uint_16>(B_/rhs));
	}
private:
	png_uint_16 R_;
	png_uint_16 G_;
	png_uint_16 B_;
};
Pixel black  (0x0000, 0x0000, 0x0000);
Pixel white  (0xffff, 0xffff, 0xffff);
Pixel red    (0xffff, 0x0000, 0x0000);
Pixel green  (0x0000, 0xffff, 0x0000);
Pixel blue   (0x0000, 0x0000, 0xffff);
Pixel cyan   (0x0000, 0xffff, 0xffff);
Pixel magenta(0xffff, 0x0000, 0xffff);
Pixel yellow (0xffff, 0xffff, 0x0000);

class Row{
public:
	Row(png_bytep row, const png_uint_32& width): row_(row), width_(width){}
	const png_uint_32& width()const{return width_;}
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
	png_bytep row_;
	const png_uint_32& width_;
};

class FrameBuffer{
public:
	FrameBuffer(const png_uint_32& width, const png_uint_32& height):
		head_(new png_byte[height*width*pixelsize]), width_(width), height_(height){}
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
	png_bytep head()const{return head_;}
	const png_uint_32& width()const{return width_;}
	const png_uint_32& height()const{return height_;}
private:
	FrameBuffer(const FrameBuffer&);
	FrameBuffer& operator=(const FrameBuffer&);
	png_bytep head_;
	const png_uint_32& width_;
	const png_uint_32& height_;
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

//class RandomColor: Painter{
//public:
//	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
//	virtual Pixel operator()(){return {distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
//private:
//	std::mt19937 engine_;
//	std::uniform_int_distribution<png_uint_16> distribution_;
//};

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

class ColorBar: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 x = width*3/4;
		std::fill(&buffer[0][0],               &buffer[0][width/8],         white  /100*40);
		std::fill(&buffer[0][width/8],         &buffer[0][width/8 + x/7],   white  /100*75);
		std::fill(&buffer[0][width/8 + x/7],   &buffer[0][width/8 + x/7*2], yellow /100*75);
		std::fill(&buffer[0][width/8 + x/7*2], &buffer[0][width/8 + x/7*3], cyan   /100*75);
		std::fill(&buffer[0][width/8 + x/7*3], &buffer[0][width/8 + x/7*4], green  /100*75);
		std::fill(&buffer[0][width/8 + x/7*4], &buffer[0][width/8 + x/7*5], magenta/100*75);
		std::fill(&buffer[0][width/8 + x/7*5], &buffer[0][width/8 + x/7*6], red    /100*75);
		std::fill(&buffer[0][width/8 + x/7*6], &buffer[0][width/8 + x],     blue   /100*75);
		std::fill(&buffer[0][width/8 + x],     &buffer[0][width],           white  /100*40);
		const png_uint_32 h1 = height*7/12;
		Row::fill(buffer[1], buffer[h1], buffer[0]);

		std::fill(&buffer[h1][0],             &buffer[h1][width/8],       cyan);
		std::fill(&buffer[h1][width/8],       &buffer[h1][width/8 + x/7], white);
		std::fill(&buffer[h1][width/8 + x/7], &buffer[h1][width/8 + x],   white/100*75);
		std::fill(&buffer[h1][width/8 + x],   &buffer[h1][width],         blue);
		const png_uint_32 h2 = h1 + height/12;
		Row::fill(buffer[h1 + 1], buffer[h2], buffer[h1]);

		std::fill(&buffer[h2][0],           &buffer[h2][width/8], yellow);
		std::fill(&buffer[h2][width/8 + x], &buffer[h2][width],   red);
		std::generate(&buffer[h2][width/8], &buffer[h2][width/8 + x], Gradator(white/x));
		const png_uint_32 h3 = h2 + height/12;
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
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
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
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 stair_height = height/stairs_;
		const png_uint_32 step_width = width/steps_ + (width%steps_ ? 1 : 0);
		bool invert = invert_;
		for(png_uint_32 row = 0; row < height; row += stair_height){
			Gradator gradator(white/steps_, invert ? white : black, invert);
			for(png_uint_32 column = 0; column < width; column += step_width){
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
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		const png_uint_32 stair_width = width/stairs_;
		const png_uint_32 step_height = height/steps_ + (height%steps_ ? 1 : 0);
		bool invert = invert_;
		std::vector<Gradator> gradators;
		for(int i = 0; i < stairs_; ++i){
			gradators.push_back(Gradator(white/steps_, invert ? white : black, invert));
			invert = !invert;
		}
		for(png_uint_32 row = 0; row < height; row += step_height){
			for(png_uint_32 column = 0; column < width; column += stair_width){
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
	CrossHatch(png_uint_32 width, png_uint_32 height): lattice_width_(width), lattice_height_(height){}
	virtual void generate(FrameBuffer& buffer)const
	{
		const png_uint_32 width = buffer.width();
		const png_uint_32 height = buffer.height();
		for(png_uint_32 i = 0; i < height; i += lattice_height_){
			std::fill(&buffer[i][0], &buffer[i][width], white);
		}
		std::fill(&buffer[height - 1][0], &buffer[height][0], white);

		for(png_uint_32 i = 0; i < width; i += lattice_width_){
			for(png_uint_32 j = 0; j < height; ++j){
				buffer[j][i] = white;
			}
		}
		for(png_uint_32 i = 0; i < height; ++i){
			buffer[i][width - 1] = white;
		}

		const double slope = static_cast<double>(height)/width;
		for(png_uint_32 i = 0; i < width; ++i){
			buffer[slope*i][i] = white;
			buffer[height - slope*i][i] = white;
		}

		const png_uint_32 radius = height/2;
		const png_uint_32 shift_v = height/2;
		const png_uint_32 shift_h = width/2;
		for(double theta = 0; theta < 2*M_PI; theta += 2.0*M_PI/5000.0){
			png_uint_32 row    = std::min(height - 1, static_cast<png_uint_32>(shift_v + radius*std::sin(theta)));
			png_uint_32 column = std::min(width  - 1, static_cast<png_uint_32>(shift_h + radius*std::cos(theta)));
			buffer[row][column] = white;
		}
	}
private:
	const png_uint_32 lattice_width_;
	const png_uint_32 lattice_height_;
};

//class WhiteNoise: public PatternGenerator{
//public:
//	virtual void generate(FrameBuffer& buffer)const
//	{
//		RandomColor random_color;
//		for(png_uint_32 row = 0; row < buffer.height(); ++row){
//			for(png_uint_32 column = 0; column < buffer.width(); ++column){
//				buffer[row][column] = random_color();
//			}
//		}
//	}
//};

unsigned char char_width  = 8; // dots
unsigned char char_height = 8; // dots
unsigned char char_tab_width = 4; // chars
unsigned char char_bitmask[8] = {
	0b10000000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
	0b00000001,
};
unsigned char characters[][8] = {
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

class Character: public PatternGenerator{
public:
	Character(const std::string& text, const Pixel& pixel = white,
			int scale = 1, png_uint_32 row = 0, png_uint_32 column = 0):
		text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
	virtual void generate(FrameBuffer& buffer)const{write(buffer, row_, column_, text_, pixel_, scale_);}
	void write(FrameBuffer& buffer, png_uint_32 row, png_uint_32 column,
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
	void write(FrameBuffer& buffer, png_uint_32 row, png_uint_32 column,
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
	const png_uint_32 row_;
	const png_uint_32 column_;
};

class TypeWriter: public PatternGenerator{
public:
	TypeWriter(const std::string& textfilename): width_(), height_(), text_()
	{
		std::ifstream ifs(textfilename.c_str());
		std::string line;
		while(std::getline(ifs, line)){
			width_ = std::max(width_,
					static_cast<png_uint_32>(line.size() +
						(char_tab_width - 1)*std::count_if(line.begin(), line.end(), is_tab)));
			text_ += line + '\n';
			++height_;
		}
		width_ *= char_width;
		height_ *= char_height;
	}
	virtual const png_uint_32& width()const{return width_;}
	virtual const png_uint_32& height()const{return height_;}
	virtual void generate(FrameBuffer& buffer)const{buffer << Character(text_, white);}
private:
	static bool is_tab(char c){return c == '\t';}
	png_uint_32 width_;
	png_uint_32 height_;
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
//	virtual const png_uint_32& width()const{return width_;}
//	virtual const png_uint_32& height()const{return height_;}
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
//	png_uint_32 width_;
//	png_uint_32 height_;
//	std::vector<Pixel> pixels_;
//};

png_uint_16 swap_msb_lsb(png_uint_16 value)
{
	return value >> 8 | value << 8;
}

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
	png_bytepp row_ptrs = new png_bytep[buffer.height()];
	for(png_uint_32 i = 0; i < buffer.height(); ++i){
		row_ptrs[i] = buffer.head() + i*buffer.width()*pixelsize;
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs);
	std::transform(
			reinterpret_cast<png_uint_16*>(buffer.head()),
			reinterpret_cast<png_uint_16*>(buffer.head() + buffer.height()*buffer.width()*pixelsize),
			reinterpret_cast<png_uint_16*>(buffer.head()), swap_msb_lsb);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete[] row_ptrs;
	std::fclose(fp);
}

void generate_16bpc_tif(const std::string& output_filename, FrameBuffer& buffer)
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

void generate_16bpc_img(const std::string& output_basename, FrameBuffer& buffer)
{
	generate_16bpc_tif(output_basename + ".tif", buffer);
	generate_16bpc_png(output_basename + ".png", buffer);
}

void generate_builtin_patterns(png_uint_32 width, png_uint_32 height)
{
	FrameBuffer buffer(width, height);

	generate_16bpc_img("colorbar", buffer << ColorBar());
	generate_16bpc_img("white100", buffer << Luster(white));
	generate_16bpc_img("red100", buffer << Luster(red));
	generate_16bpc_img("green100", buffer << Luster(green));
	generate_16bpc_img("blue100", buffer << Luster(blue));
	generate_16bpc_img("white50", buffer << Luster(white/2));
	generate_16bpc_img("red50", buffer << Luster(red/2));
	generate_16bpc_img("green50", buffer << Luster(green/2));
	generate_16bpc_img("blue50", buffer << Luster(blue/2));
	generate_16bpc_img("checker1", buffer << Checker());
	generate_16bpc_img("checker2", buffer << Checker(true));
	generate_16bpc_img("stairstepH1", buffer << StairStepH());
	generate_16bpc_img("stairstepH2", buffer << StairStepH(1, 20, false));
	generate_16bpc_img("stairstepH3", buffer << StairStepH(1, 20, true));
	generate_16bpc_img("stairstepV1", buffer << StairStepV());
	generate_16bpc_img("stairstepV2", buffer << StairStepV(1, 20, false));
	generate_16bpc_img("stairstepV3", buffer << StairStepV(1, 20, true));
	generate_16bpc_img("ramp", buffer << Ramp());
	generate_16bpc_img("crosshatch", buffer << Luster(black) << CrossHatch(192, 108));
	generate_16bpc_img("character", buffer << Luster(black) << Character(" !\"#$%&'()*+,-./\n"
			"0123456789:;<=>?@\nABCDEFGHIJKLMNO\nPQRSTUVWXYZ[\\]^_`\n"
			"abcdefghijklmno\npqrstuvwxyz{|}~", red, 10));
//	generate_16bpc_img("whitenoise", buffer << WhiteNoise());
}

void generate_self(png_uint_32 width, png_uint_32 height)
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
			std::string::size_type idx = std::string(argv[i]).find('=');
			std::string key(argv[i], idx);
			std::string value(argv[i] + idx + 1);
			store[key] = value;
		}
	}
	return store;
}

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);

	bool builtin_patterns_enabled = false;
	std::string input = "-";
	std::string output = "-";
	png_uint_32 height = 2160;
	png_uint_32 width = 3840;
	if(store["builtins"] != ""){
		builtin_patterns_enabled = true;
	}
	if(store["input"] != ""){
		input = store["input"];
	}
	if(store["output"] != ""){
		output = store["output"];
	}
	if(store["height"] != ""){
		height = atoi(store["height"].c_str());
	}
	if(store["width"] != ""){
		width = atoi(store["width"].c_str());
	}

	if(builtin_patterns_enabled){
		generate_builtin_patterns(width, height);
		return 0;
	}

	FrameBuffer buffer(width, height);
	std::ifstream ifs(input.c_str());
	generate_16bpc_png(output, buffer << ifs);

	return 0;
}
