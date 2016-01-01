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
#include <cstdio>
#include <fstream>
#include <memory>
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
	PseudoPixel step_;
	PseudoPixel state_;
	Gradator(const PseudoPixel& step, const PseudoPixel& initial={}): step_(step), state_(initial){}
	PseudoPixel operator()()
	{
		PseudoPixel tmp = state_;
		state_ = state_ + step_;
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
	Row(png_bytep row): row_(row){}
	Pixel& operator[](int column)const{return *reinterpret_cast<Pixel*>(row_ + column*pixelsize);}
};
struct RowBlock{
	Pixel pixels_[::width];
	static RowBlock* cast(Pixel* ptr){return reinterpret_cast<RowBlock*>(ptr);}
};

struct Image{
	const png_bytep block_;
	const Generator& generator_;
	Image(png_bytep block, const Generator& generator): block_(block), generator_(generator){}
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

struct FullField: FixSizedGenerator{
	virtual void generate(Image image)const override{std::fill(&image[0][0], &image[height()][0], pixel_);}
	FullField(const Pixel& pixel): pixel_(pixel){}
	Pixel pixel_;
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
	bool invert_;
	Checker(bool invert = false): invert_(invert){}
};

struct Stairs: FixSizedGenerator{
	virtual void generate(Image image)const override
	{
		const int stairs = 10;
		const png_uint_32 stair_height = height()/stairs;
		int steps = 2;
		for(png_uint_32 i=0; i<height(); i+=stair_height){
			fill(image, i, stair_height, steps);
			steps *= 2;
		}
	}
	void fill(Image image, png_uint_32 row, png_uint_32 stair_height, int steps)const
	{
		const png_uint_32 step_width = width()/steps + (width()%steps ? 1 : 0);
		const PseudoPixel step_color_width = white/steps;
		for(png_uint_32 i=0, step=0; i<width(); i+=step_width, ++step){
			if(width() <= i+step_width){
				std::fill(&image[row][i], &image[row][width()], step_color_width*step);
			}else{
				std::fill(&image[row][i], &image[row][i+step_width], step_color_width*step);
			}
		}
		for(png_uint_32 i=row+1; i<height() && i<row+stair_height; ++i){
			std::copy(&image[row][0], &image[row][width()], &image[i][0]);
		}
	}
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
	}
	png_uint_32 lattice_width_;
	png_uint_32 lattice_height_;
	CrossHatch(png_uint_32 width, png_uint_32 height): lattice_width_(width), lattice_height_(height){}
};

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
	generate_16bpc_png(argv[0], "colorbar.png", ColorBar());
	generate_16bpc_png(argv[0], "red.png", FullField(red));
	generate_16bpc_png(argv[0], "green.png", FullField(green));
	generate_16bpc_png(argv[0], "blue.png", FullField(blue));
	generate_16bpc_png(argv[0], "checker.png", Checker());
	generate_16bpc_png(argv[0], "checker_inverted.png", Checker(true));
	generate_16bpc_png(argv[0], "stairs.png", Stairs());
	generate_16bpc_png(argv[0], "lamp.png", Lamp());
	generate_16bpc_png(argv[0], "crosshatch.png", CrossHatch(192, 108));
//	generate_16bpc_png(argv[0], "user_defined.png", CSVLoader("input.csv"));
	return 0;
}
