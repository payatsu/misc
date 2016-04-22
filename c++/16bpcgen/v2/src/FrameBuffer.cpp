#include <cstdio>
#include <stdexcept>
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
#ifdef ENABLE_PNG
#	define PNG_NO_SETJMP
#	include <libpng16/png.h>
#endif
#include "FrameBuffer.hpp"

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const int pixelsize = 6;

const Pixel black  (0x0000, 0x0000, 0x0000);
const Pixel white  (0xffff, 0xffff, 0xffff);
const Pixel red    (0xffff, 0x0000, 0x0000);
const Pixel green  (0x0000, 0xffff, 0x0000);
const Pixel blue   (0x0000, 0x0000, 0xffff);
const Pixel cyan   (0x0000, 0xffff, 0xffff);
const Pixel magenta(0xffff, 0x0000, 0xffff);
const Pixel yellow (0xffff, 0xffff, 0x0000);

FrameBuffer::FrameBuffer(const std::string& filename): head_(NULL), width_(0), height_(0)
{
#ifdef ENABLE_PNG
	FILE* fp = fopen(filename.c_str(), "rb");
	if(!fp){
		std::perror(__func__);
		throw std::runtime_error(__func__);
	}

	const int number = 8;
	unsigned char signature[number];
	if(std::fread(signature, sizeof(unsigned char), number, fp) != number){
		std::perror(__func__);
		std::fclose(fp);
		throw std::runtime_error(__func__);
	}
	if(png_sig_cmp(signature, 0, number)){
		std::fclose(fp);
		throw std::runtime_error(__func__);
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		std::fclose(fp);
		throw std::runtime_error(__func__);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		std::fclose(fp);
		throw std::runtime_error(__func__);
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, number);
	png_read_png(png_ptr, info_ptr,
				 PNG_TRANSFORM_STRIP_ALPHA |
				 PNG_TRANSFORM_SWAP_ENDIAN |
				 PNG_TRANSFORM_GRAY_TO_RGB |
				 PNG_TRANSFORM_EXPAND_16,
				 NULL);
	uint8_t** row_ptrs = png_get_rows(png_ptr, info_ptr);

	width_  = png_get_image_width(png_ptr, info_ptr);
	height_ = png_get_image_height(png_ptr, info_ptr);
	head_   = new uint8_t[height_*width_*pixelsize];

	for(uint32_t i = 0; i < height_; ++i){
		std::copy(&row_ptrs[i][0], &row_ptrs[i][width_*pixelsize], reinterpret_cast<uint8_t*>(&this->operator[](i)[0]));
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	std::fclose(fp);
#endif
}

FrameBuffer::FrameBuffer(const FrameBuffer& buffer):
	head_(new uint8_t[buffer.data_size()]), width_(buffer.width_), height_(buffer.height_)
{
	std::copy(buffer.head_, buffer.head_ + buffer.data_size(), head_);
}

FrameBuffer& FrameBuffer::operator<<(std::istream& is)
{
	const uint32_t size = data_size();
	for(uint32_t i = 0; i < size; ++i){
		int c = is.get();
		if(is.eof()){
			break;
		}
		head_[i] = c;
	}
	return *this;
}

FrameBuffer& FrameBuffer::write(const std::string& filename)const
{
	if(have_ext(filename, ".tif")){
#ifdef ENABLE_TIFF
		return write_tiff(filename);
#endif
	}else if(have_ext(filename, ".png")){
#ifdef ENABLE_PNG
		return write_png(filename);
#endif
	}else{
#ifdef ENABLE_TIFF
		write_tiff(filename + ".tif");
#endif
#ifdef ENABLE_PNG
		write_png(filename + ".png");
#endif
		return const_cast<FrameBuffer&>(*this);
	}
}

#ifdef ENABLE_TIFF
FrameBuffer& FrameBuffer::write_tiff(const std::string& filename)const
{
	TIFF* image = TIFFOpen(filename.c_str(), "w");
	if(!image){
		throw std::runtime_error(__func__);
	}
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, width());
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, height());
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, bitdepth);
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, height());
	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(image, TIFFTAG_XRESOLUTION, 150.0);
	TIFFSetField(image, TIFFTAG_YRESOLUTION, 150.0);
	TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFWriteEncodedStrip(image, 0, head(), data_size());
	TIFFClose(image);
	return const_cast<FrameBuffer&>(*this);
}
#endif

#ifdef ENABLE_PNG
FrameBuffer& FrameBuffer::write_png(const std::string& filename)const
{
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		throw std::runtime_error(__func__);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_write_struct(&png_ptr, NULL);
		throw std::runtime_error(__func__);
	}

	FILE* fp = std::fopen(filename.c_str(), "wb");
	if(!fp){
		std::perror(__func__);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw std::runtime_error(__func__);
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width(), height(),
			bitdepth, colortype, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	uint8_t** row_ptrs = new uint8_t*[height()];
	for(uint32_t i = 0; i < height(); ++i){
		row_ptrs[i] = reinterpret_cast<uint8_t*>(&this->operator[](i)[0]);
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ENDIAN, NULL);

	delete[] row_ptrs;
	png_destroy_write_struct(&png_ptr, &info_ptr);
	std::fclose(fp);
	return const_cast<FrameBuffer&>(*this);
}
#endif

bool have_ext(const std::string& filename, const std::string& ext)
{
	const std::string::size_type idx = filename.find(ext);
	return !(idx == std::string::npos || idx + ext.size() != filename.size());
}

std::string append_extension(const std::string& filename, const std::string& ext)
{
	if(!have_ext(filename, ext)){
		return filename + ext;
	}else{
		return filename;
	}
}
