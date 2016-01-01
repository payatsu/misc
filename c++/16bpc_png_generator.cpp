/**
 * @file
 * @brief Program to generate 16 bpc(bits per component) PNG images.
 * @details
 * To compile this program, do as described below.
 * @code
 * $ g++ --std=c++14 16bpc_png_generator.cpp -lpng
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
#include <memory>
#include <random>
#include <sstream>
#include <vector>
#include <png.h>

constexpr png_uint_32 width  = 1920;
constexpr png_uint_32 height = 1080;
constexpr int bitdepth       = 16;
constexpr int colortype      = PNG_COLOR_TYPE_RGB;
constexpr int pixelsize      = 6;

struct Image;

struct Generator{
	virtual png_uint_32 width()const = 0;
	virtual png_uint_32 height()const = 0;
	virtual void generate(Image image)const = 0;
	virtual ~Generator() = default;
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
struct Gradator{
	const PseudoPixel step_;
	PseudoPixel state_;
	const bool invert_;
	constexpr Gradator(const PseudoPixel& step, const PseudoPixel& initial=black, bool invert=false): step_(step), state_(initial), invert_(invert){}
	PseudoPixel operator()()
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
	Pixel pixels_[::width];
	static RowBlock* cast(Pixel* ptr){return reinterpret_cast<RowBlock*>(ptr);}
};

struct Image{
	const png_bytep block_;
	const Generator& generator_;
	constexpr Image(png_bytep block, const Generator& generator): block_(block), generator_(generator){}
	Row operator[](int row)const{return Row(block_ + row*generator_.width()*pixelsize);}
};

struct FixSizedGenerator: Generator{
	virtual png_uint_32 width()const override final{return ::width;}
	virtual png_uint_32 height()const override final{return ::height;}
};

struct ColorBar: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		const png_uint_32 x = width()*3/4;
		std::fill(&image[0][0],               &image[0][width()/8],       white  /100*40);
		std::fill(&image[0][width()/8],       &image[0][width()/8+x/7],   white  /100*75);
		std::fill(&image[0][width()/8+x/7],   &image[0][width()/8+x/7*2], yellow /100*75);
		std::fill(&image[0][width()/8+x/7*2], &image[0][width()/8+x/7*3], cyan   /100*75);
		std::fill(&image[0][width()/8+x/7*3], &image[0][width()/8+x/7*4], green  /100*75);
		std::fill(&image[0][width()/8+x/7*4], &image[0][width()/8+x/7*5], magenta/100*75);
		std::fill(&image[0][width()/8+x/7*5], &image[0][width()/8+x/7*6], red    /100*75);
		std::fill(&image[0][width()/8+x/7*6], &image[0][width()/8+x],     blue   /100*75);
		std::fill(&image[0][width()/8+x],     &image[0][width()],         white  /100*40);
		const png_uint_32 h1 = height()*7/12;
		std::fill(RowBlock::cast(&image[1][0]), RowBlock::cast(&image[h1][0]), *RowBlock::cast(&image[0][0]));

		std::fill(&image[h1][0],             &image[h1][width()/8],     cyan);
		std::fill(&image[h1][width()/8],     &image[h1][width()/8+x/7], white);
		std::fill(&image[h1][width()/8+x/7], &image[h1][width()/8+x],   white/100*75);
		std::fill(&image[h1][width()/8+x],   &image[h1][width()],       blue);
		const png_uint_32 h2 = h1 + height()/12;
		std::fill(RowBlock::cast(&image[h1+1][0]), RowBlock::cast(&image[h2][0]), *RowBlock::cast(&image[h1][0]));

		std::fill(&image[h2][0],           &image[h2][width()/8], yellow);
		std::fill(&image[h2][width()/8+x], &image[h2][width()],   red);
		std::generate(&image[h2][width()/8], &image[h2][width()/8+x], Gradator(white/x));
		const png_uint_32 h3 = h2 + height()/12;
		std::fill(RowBlock::cast(&image[h2+1][0]), RowBlock::cast(&image[h3][0]), *RowBlock::cast(&image[h2][0]));

		std::fill(&image[h3][0],                       &image[h3][width()/8],               white/100*15);
		std::fill(&image[h3][width()/8],               &image[h3][width()/8+x/7*3/2],       black);
		std::fill(&image[h3][width()/8+x/7*3/2],       &image[h3][width()/8+x/7*3/2+2*x/7], white);
		std::fill(&image[h3][width()/8+x/7*3/2+2*x/7], &image[h3][width()/8+x],             black);
		std::fill(&image[h3][width()/8+x],             &image[h3][width()],                 white/100*15);
		std::fill(RowBlock::cast(&image[h3+1][0]), RowBlock::cast(&image[height()][0]), *RowBlock::cast(&image[h3][0]));
	}
};

struct Luster: FixSizedGenerator{
	virtual void generate(Image image)const override{std::fill(&image[0][0], &image[height()][0], pixel_);}
	constexpr Luster(const Pixel& pixel): pixel_(pixel){}
	const Pixel pixel_;
};

struct Checker: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		const auto pattern1 = invert_ ? black : white;
		const auto pattern2 = invert_ ? white : black;
		std::fill(&image[0][0],           &image[0][width()/4],   pattern1);
		std::fill(&image[0][width()/4],   &image[0][width()/4*2], pattern2);
		std::fill(&image[0][width()/4*2], &image[0][width()/4*3], pattern1);
		std::fill(&image[0][width()/4*3], &image[0][width()],     pattern2);
		std::fill(RowBlock::cast(&image[1][0]),            RowBlock::cast(&image[height()/4][0]),   *RowBlock::cast(&image[0][0]));
		std::fill(RowBlock::cast(&image[height()/4*2][0]), RowBlock::cast(&image[height()/4*3][0]), *RowBlock::cast(&image[0][0]));

		std::fill(&image[height()/4][0],           &image[height()/4][width()/4],   pattern2);
		std::fill(&image[height()/4][width()/4],   &image[height()/4][width()/4*2], pattern1);
		std::fill(&image[height()/4][width()/4*2], &image[height()/4][width()/4*3], pattern2);
		std::fill(&image[height()/4][width()/4*3], &image[height()/4][width()],     pattern1);
		std::fill(RowBlock::cast(&image[height()/4+1][0]), RowBlock::cast(&image[height()/4*2][0]), *RowBlock::cast(&image[height()/4][0]));
		std::fill(RowBlock::cast(&image[height()/4*3][0]), RowBlock::cast(&image[height()][0]),     *RowBlock::cast(&image[height()/4][0]));
	}
	const bool invert_;
	constexpr Checker(bool invert = false): invert_(invert){}
};

struct StairStepH: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		const png_uint_32 stair_height = height()/stairs_;
		const png_uint_32 step_width = width()/steps_ + (width()%steps_ ? 1 : 0);
		bool invert = invert_;
		for(png_uint_32 row=0; row<height(); row+=stair_height){
			Gradator gradator{white/steps_, invert ? white : black, invert};
			for(png_uint_32 column=0; column<width(); column+=step_width){
				std::fill(&image[row][column], &image[row][std::min(width(), column+step_width)], gradator());
			}
			std::fill(RowBlock::cast(&image[row+1][0]), RowBlock::cast(&image[std::min(height(), row+stair_height)][0]), *RowBlock::cast(&image[row][0]));
			invert = !invert;
		}
	}
	const int stairs_;
	const int steps_;
	const bool invert_;
	constexpr StairStepH(int stairs=2, int steps=20): stairs_(stairs), steps_(steps), invert_(false){}
	constexpr StairStepH(bool invert): stairs_(1), steps_(20), invert_(invert){}
};

struct StairStepV: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		const png_uint_32 stair_width = width()/stairs_;
		const png_uint_32 step_height = height()/steps_ + (height()%steps_ ? 1 : 0);
		bool invert = invert_;
		std::vector<Gradator> gradators;
		for(int i=0; i<stairs_; ++i){
			gradators.push_back({white/steps_, invert ? white : black, invert});
			invert = !invert;
		}
		for(png_uint_32 row=0; row<height(); row+=step_height){
			for(png_uint_32 column=0; column<width(); column+=stair_width){
				std::fill(&image[row][column], &image[row][std::min(width(), column+stair_width)], gradators.at(column/stair_width)());
			}
			std::fill(RowBlock::cast(&image[row+1][0]), RowBlock::cast(&image[std::min(height(), row+step_height)][0]), *RowBlock::cast(&image[row][0]));
		}
	}
	const int stairs_;
	const int steps_;
	const bool invert_;
	constexpr StairStepV(int stairs=2, int steps=20): stairs_(stairs), steps_(steps), invert_(false){}
	constexpr StairStepV(bool invert): stairs_(1), steps_(20), invert_(invert){}
};

struct Lamp: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		std::generate(&image[0][0],              &image[0][width()],              Gradator(red    /width()));
		std::generate(&image[height()/12][0],    &image[height()/12][width()],    Gradator(green  /width()));
		std::generate(&image[height()/12*2][0],  &image[height()/12*2][width()],  Gradator(blue   /width()));
		std::generate(&image[height()/12*3][0],  &image[height()/12*3][width()],  Gradator(cyan   /width()));
		std::generate(&image[height()/12*4][0],  &image[height()/12*4][width()],  Gradator(magenta/width()));
		std::generate(&image[height()/12*5][0],  &image[height()/12*5][width()],  Gradator(yellow /width()));
		std::generate(&image[height()/12*6][0],  &image[height()/12*6][width()],  Gradator(cyan   /width(), red));
		std::generate(&image[height()/12*7][0],  &image[height()/12*7][width()],  Gradator(magenta/width(), green));
		std::generate(&image[height()/12*8][0],  &image[height()/12*8][width()],  Gradator(yellow /width(), blue));
		std::generate(&image[height()/12*9][0],  &image[height()/12*9][width()],  Gradator(red    /width(), cyan));
		std::generate(&image[height()/12*10][0], &image[height()/12*10][width()], Gradator(green  /width(), magenta));
		std::generate(&image[height()/12*11][0], &image[height()/12*11][width()], Gradator(blue   /width(), yellow));

		std::fill(RowBlock::cast(&image[1][0]),                RowBlock::cast(&image[height()/12][0]),    *RowBlock::cast(&image[0][0]));
		std::fill(RowBlock::cast(&image[height()/12+1][0]),    RowBlock::cast(&image[height()/12*2][0]),  *RowBlock::cast(&image[height()/12][0]));
		std::fill(RowBlock::cast(&image[height()/12*2+1][0]),  RowBlock::cast(&image[height()/12*3][0]),  *RowBlock::cast(&image[height()/12*2][0]));
		std::fill(RowBlock::cast(&image[height()/12*3+1][0]),  RowBlock::cast(&image[height()/12*4][0]),  *RowBlock::cast(&image[height()/12*3][0]));
		std::fill(RowBlock::cast(&image[height()/12*4+1][0]),  RowBlock::cast(&image[height()/12*5][0]),  *RowBlock::cast(&image[height()/12*4][0]));
		std::fill(RowBlock::cast(&image[height()/12*5+1][0]),  RowBlock::cast(&image[height()/12*6][0]),  *RowBlock::cast(&image[height()/12*5][0]));
		std::fill(RowBlock::cast(&image[height()/12*6+1][0]),  RowBlock::cast(&image[height()/12*7][0]),  *RowBlock::cast(&image[height()/12*6][0]));
		std::fill(RowBlock::cast(&image[height()/12*7+1][0]),  RowBlock::cast(&image[height()/12*8][0]),  *RowBlock::cast(&image[height()/12*7][0]));
		std::fill(RowBlock::cast(&image[height()/12*8+1][0]),  RowBlock::cast(&image[height()/12*9][0]),  *RowBlock::cast(&image[height()/12*8][0]));
		std::fill(RowBlock::cast(&image[height()/12*9+1][0]),  RowBlock::cast(&image[height()/12*10][0]), *RowBlock::cast(&image[height()/12*9][0]));
		std::fill(RowBlock::cast(&image[height()/12*10+1][0]), RowBlock::cast(&image[height()/12*11][0]), *RowBlock::cast(&image[height()/12*10][0]));
		std::fill(RowBlock::cast(&image[height()/12*11+1][0]), RowBlock::cast(&image[height()][0]),       *RowBlock::cast(&image[height()/12*11][0]));
	}
};

struct CrossHatch: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		std::fill(&image[0][0], &image[height()][0], black);

		for(png_uint_32 i=0; i<height(); i+=lattice_height_){
			std::fill(&image[i][0], &image[i][width()], white);
		}
		std::fill(&image[height()-1][0], &image[height()][0], white);

		for(png_uint_32 i=0; i<width(); i+=lattice_width_){
			for(png_uint_32 j=0; j<height(); ++j){
				image[j][i] = white;
			}
		}
		for(png_uint_32 i=0; i<height(); ++i){
			image[i][width()-1] = white;
		}

		const double slope = static_cast<double>(height())/width();
		for(png_uint_32 i=0; i<width(); ++i){
			image[slope*i][i] = white;
			image[height()-slope*i][i] = white;
		}

		const png_uint_32 radius = height()/2;
		const png_uint_32 shift_v = height()/2;
		const png_uint_32 shift_h = width()/2;
		for(double theta=0; theta<2*M_PI; theta+=2.0*M_PI/5000.0){
			png_uint_32 row    = std::min(height() - 1, static_cast<png_uint_32>(shift_v + radius*std::sin(theta)));
			png_uint_32 column = std::min(width()  - 1, static_cast<png_uint_32>(shift_h + radius*std::cos(theta)));
			image[row][column] = white;
		}

	}
	const png_uint_32 lattice_width_;
	const png_uint_32 lattice_height_;
	constexpr CrossHatch(png_uint_32 width, png_uint_32 height): lattice_width_(width), lattice_height_(height){}
};

struct GeneratorExample: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		for(png_uint_32 row=0; row<height(); ++row){
			for(png_uint_32 column=0; column<width(); ++column){
				image[row][column] = black;
			}
		}
	}
};

// TODO ColorStep
// TODO Multi
// TODO Character
// TODO Focus
// TODO Pixel, PseudoPixelをColorにリネーム？

constexpr unsigned char char_width  = 8;
constexpr unsigned char char_height = 8;
constexpr unsigned char bits[8] = {
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
		0b00111100,
		0b00111100,
		0b00111100,
		0b00011000,
		0b00000000,
		0b00011000,
		0b00011000,
	},{ // "
		0b00000000,
		0b01100110,
		0b01100110,
		0b00100010,
		0b01000100,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // #
		0b00000000,
		0b01000100,
		0b11111110,
		0b01000100,
		0b01000100,
		0b01000100,
		0b11111110,
		0b01000100,
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
		0b11100001,
		0b10100010,
		0b11100100,
		0b00001000,
		0b00010000,
		0b00100111,
		0b01000101,
		0b10000111,
	},{ // &
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111000,
		0b01000101,
		0b10000010,
		0b01111101,
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
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00011000,
		0b00000110,
	},{ // )
		0b01100000,
		0b00011000,
		0b00000100,
		0b00000100,
		0b00000100,
		0b00000100,
		0b00011000,
		0b01100000,
	},{ // *
		0b00000000,
		0b00000000,
		0b01000100,
		0b00101000,
		0b11111110,
		0b00101000,
		0b01000100,
		0b00000000,
	},{ // +
		0b00000000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b11111110,
		0b00010000,
		0b00010000,
		0b00010000,
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
		0b00001100,
		0b00001000,
		0b00010000,
		0b00010000,
		0b00100000,
		0b00100000,
		0b01000000,
		0b11000000,
	},{ // 0
		0b01111100,
		0b10000110,
		0b10001010,
		0b10010010,
		0b10100010,
		0b11000010,
		0b10000010,
		0b01111100,
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
		0b00110000,
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
		0b01000100,
		0b01000100,
		0b01111100,
		0b10000010,
		0b10000010,
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
		0b01111100,
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
		0b00111110,
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
		0b01111100,
	},{ // M
		0b00000000,
		0b11000110,
		0b10101010,
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
		0b01000110,
	},{ // O
		0b00000000,
		0b00111000,
		0b01000100,
		0b10000010,
		0b10000010,
		0b10000010,
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
		0b01111100,
		0b10000010,
		0b10000010,
		0b10010010,
		0b10001010,
		0b10000100,
		0b01111011,
	},{ // R
		0b00000000,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01111000,
		0b01000100,
		0b01000100,
		0b01000010,
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
		0b11111110,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
	},{ // U
		0b00000000,
		0b01000010,
		0b01000010,
		0b01000010,
		0b01000010,
		0b01000010,
		0b01000010,
		0b00111100,
	},{ // V
		0b00000000,
		0b10000010,
		0b10000010,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00101000,
		0b00010000,
	},{ // W
		0b00000000,
		0b10000010,
		0b10000010,
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
		0b01000100,
		0b01000100,
	},{ // Y
		0b00000000,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00010000,
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
		0b01100000,
		0b00100000,
		0b00010000,
		0b00010000,
		0b00001000,
		0b00001000,
		0b00000100,
		0b00000110,
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
		0b00010000,
		0b00101000,
		0b01000100,
		0b00000000,
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
		0b01111100,
		0b00000000,
	},{ // `
		0b00000000,
		0b00010000,
		0b00001100,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
	},{ // a
		0b00000000,
		0b00000000,
		0b01111000,
		0b00000100,
		0b00111100,
		0b01000100,
		0b01000100,
		0b00111010,
	},{ // b
		0b00000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01011000,
		0b01100100,
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
		0b00110100,
		0b01001100,
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
		0b11111000,
		0b10000000,
		0b11111000,
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
		0b00000000,
		0b10101000,
		0b01010100,
		0b01010100,
		0b01010100,
	},{ // n
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b11011000,
		0b01100100,
		0b01000100,
		0b01000100,
	},{ // o
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00111000,
		0b01000100,
		0b01000100,
		0b00111000,
	},{ // p
		0b00000000,
		0b00000000,
		0b01011000,
		0b01100100,
		0b01000100,
		0b01111000,
		0b01000000,
		0b01000000,
	},{ // q
		0b00000000,
		0b00110100,
		0b01001100,
		0b01001100,
		0b00110100,
		0b00000100,
		0b00000100,
		0b00000100,
	},{ // r
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01011000,
		0b01100000,
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
		0b00000000,
		0b01000100,
		0b01000100,
		0b01000100,
		0b00111010,
	},{ // v
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00010000,
	},{ // w
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01000100,
		0b01010100,
		0b01010100,
		0b00101000,
	},{ // x
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b01101100,
		0b10010000,
		0b00101001,
		0b01000110,
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

struct Character: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		Luster(black).generate(image);
		// write(image, 0, 0, '~', white);
		write(image, 0, 0, " !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", white);
	}
	void write(Image image, png_uint_32 row, png_uint_32 column, char c, const Pixel& pixel)const
	{
		for(unsigned char i=0; i<char_height; ++i){
			for(unsigned char j=0; j<char_width; ++j){
				if(characters[c][i] & bits[j]){
					image[row+i][column+j] = pixel;
				}
			}
		}
	}
	void write(Image image, png_uint_32 row, png_uint_32 column, const std::string& str, const Pixel& pixel)const
	{
		for(std::size_t i=0; i<str.size(); ++i){
			write(image, row, column+char_width*i, str[i], pixel);
		}
	}
};

// TODO #RRRRGGGGBBBBの形式を受け付けられるようにする。
// TODO 0x000000000000の形式を受け付けられるようにする。
struct CSVLoader: Generator{
	virtual png_uint_32 width()const override{return width_;}
	virtual png_uint_32 height()const override{return height_;}
	virtual void generate(Image image)const override{std::copy(pixels_.begin(), pixels_.end(), &image[0][0]);}
	png_uint_32 width_;
	png_uint_32 height_;
	std::vector<Pixel> pixels_;
	CSVLoader(const std::string& csvfile): width_(), height_(), pixels_()
	{
		std::ifstream ifs(csvfile);
		std::string line;
		while(std::getline(ifs, line)){
			std::replace(line.begin(), line.end(), ',', ' ');
			std::istringstream iss(line);
			unsigned long long int value;
			width_ = 0;
			while(iss >> value){
				pixels_.push_back(value);
				++width_;
			}
			++height_;
		}
	}
};

void generate_16bpc_png(const std::string& prog_name, const std::string& output_filename, const Generator& generator)
{
	std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen(output_filename.c_str(), "wb"), std::fclose);
	if(!fp){
		std::perror(prog_name.c_str());
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
	generator.generate(Image(block.get(), generator));
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

int main(int argc, char* argv[])
{
	generate_16bpc_png(argv[0], "colorbar.png",    ColorBar());
	generate_16bpc_png(argv[0], "white100.png",    Luster(white));
	generate_16bpc_png(argv[0], "red100.png",      Luster(red));
	generate_16bpc_png(argv[0], "green100.png",    Luster(green));
	generate_16bpc_png(argv[0], "blue100.png",     Luster(blue));
	generate_16bpc_png(argv[0], "white50.png",     Luster(white/2));
	generate_16bpc_png(argv[0], "red50.png",       Luster(red  /2));
	generate_16bpc_png(argv[0], "green50.png",     Luster(green/2));
	generate_16bpc_png(argv[0], "blue50.png",      Luster(blue /2));
	generate_16bpc_png(argv[0], "checker1.png",    Checker());
	generate_16bpc_png(argv[0], "checker2.png",    Checker(true));
	generate_16bpc_png(argv[0], "stairstepH1.png", StairStepH());
	generate_16bpc_png(argv[0], "stairstepH2.png", StairStepH(false));
	generate_16bpc_png(argv[0], "stairstepH3.png", StairStepH(true));
	generate_16bpc_png(argv[0], "stairstepV1.png", StairStepV());
	generate_16bpc_png(argv[0], "stairstepV2.png", StairStepV(false));
	generate_16bpc_png(argv[0], "stairstepV3.png", StairStepV(true));
	generate_16bpc_png(argv[0], "lamp.png",        Lamp());
	generate_16bpc_png(argv[0], "crosshatch.png",  CrossHatch(192, 108));
	generate_16bpc_png(argv[0], "character.png",   Character());
//	generate_16bpc_png(argv[0], "userdefined.png", CSVLoader("input.csv"));
	generate_16bpc_png(argv[0], "example.png",     GeneratorExample());
	return 0;
}
