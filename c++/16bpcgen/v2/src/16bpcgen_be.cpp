/**
 * @file
 * @brief Backend of 16bpcgen.
 * @details
 * This program can:
 * - generate PNG and TIFF images of predefined(builtin) patterns,
 * - generate PNG and TIFF images based on binary sequence from stdin.
 */

// TODO ColorStep
// TODO Multi
// TODO Focus

#define _USE_MATH_DEFINES
#define PNG_NO_SETJMP

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
// #include <sstream>
// #include <vector>
#ifdef ENABLE_PNG
#	include <libpng16/png.h>
#endif
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
// #include <zlib.h>
#include "getopt.hpp"

#include "FrameBuffer.hpp"
#include "Painter.hpp"
#include "PatternGenerator.hpp"

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
	const uint32_t width  = store["width"]  == "" ? 1920 : atoi(store["width"].c_str());

	if(store["builtins"] != ""){
		generate_builtin_patterns(width, height);
		return 0;
	}

	FrameBuffer buffer(width, height);
	if(store["input"] == "-"){
		buffer << std::cin;
	}else if(store["input"] != ""){
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
