#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
#ifdef ENABLE_PNG
#	define PNG_NO_SETJMP
#	include <libpng16/png.h>
#endif
#include "FrameBuffer.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerator.hpp"
#include "Pixel.hpp"

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const int pixelsize = 6;

void Row::fill(Row first, Row last, const Row& row)
{
	while(first != last){
		std::copy(&row[0], &row[row.width()], &first[0]);
		++first;
	}
}

FrameBuffer::FrameBuffer(const std::string& filename): head_(NULL), width_(0), height_(0)
{
	if(have_ext(filename, ".tif")){
#ifdef ENABLE_TIFF
		read_tiff(filename);
#endif
	}else if(have_ext(filename, ".png")){
#ifdef ENABLE_PNG
		read_png(filename);
#endif
	}else{
		throw std::runtime_error(__func__ + std::string(": not supported file format"));
	}
}

FrameBuffer::FrameBuffer(const FrameBuffer& buffer):
	head_(new uint8_t[buffer.data_size()]), width_(buffer.width_), height_(buffer.height_)
{
	std::copy(buffer.head(), buffer.tail(), head());
}

FrameBuffer& FrameBuffer::operator=(const FrameBuffer& buffer)
{
	if(this == &buffer){
		return *this;
	}
	delete[] head_;
	width_  = buffer.width();
	height_ = buffer.height();
	head_   = new uint8_t[data_size()];
	std::copy(buffer.head(), buffer.tail(), head());
	return *this;
}

FrameBuffer FrameBuffer::operator<<(const PatternGenerator& generator)const
{
	FrameBuffer result = FrameBuffer(width(), height());
	return generator.generate(result);
}

FrameBuffer& FrameBuffer::operator<<=(const PatternGenerator& generator)
{
	return generator.generate(*this);
}

FrameBuffer& FrameBuffer::operator<<=(std::istream& is)
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

FrameBuffer FrameBuffer::operator>>(const ImageProcess& process)const
{
	FrameBuffer result = FrameBuffer(*this);
	return process.process(result);
}

FrameBuffer& FrameBuffer::operator>>=(const ImageProcess& process)
{
	return process.process(*this);
}

FrameBuffer FrameBuffer::operator>>(const PixelConverter& converter)const
{
	FrameBuffer result = FrameBuffer(*this);
	return Tone(converter).process(result);
}

FrameBuffer& FrameBuffer::operator>>=(const PixelConverter& converter)
{
	return Tone(converter).process(*this);
}

FrameBuffer FrameBuffer::operator<<(uint8_t shift)const
{
	FrameBuffer result = FrameBuffer(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], lshifter(shift));
	return result;
}

FrameBuffer FrameBuffer::operator>>(uint8_t shift)const
{
	FrameBuffer result = FrameBuffer(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], rshifter(shift));
	return result;
}

FrameBuffer& FrameBuffer::operator<<=(uint8_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], lshifter(shift));
	return *this;
}

FrameBuffer& FrameBuffer::operator>>=(uint8_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], rshifter(shift));
	return *this;
}

FrameBuffer FrameBuffer::operator+(const FrameBuffer& buffer)const
{
	if(width() != buffer.width()){
		throw std::runtime_error(__func__);
	}
	FrameBuffer result = FrameBuffer(width(), height() + buffer.height());
	std::copy(head(), head() + data_size(), result.head());
	std::copy(buffer.head(), buffer.head() + buffer.data_size(), result.head() + data_size());
	return result;
}

FrameBuffer FrameBuffer::operator,(const FrameBuffer& buffer)const
{
	if(height() != buffer.height()){
		throw std::runtime_error(__func__);
	}
	FrameBuffer result = FrameBuffer(width() + buffer.width(), height());
	for(uint32_t h = 0; h < height(); ++h){
		std::copy(head() + h * width() * pixelsize, head() + (h + 1) * width() * pixelsize, result.head() + h * result.width() * pixelsize);
		std::copy(buffer.head() + h * buffer.width() * pixelsize, buffer.head() + (h + 1) * buffer.width() * pixelsize, result.head() + width() * pixelsize + h * result.width() * pixelsize);
	}
	return result;
}

//FrameBuffer FrameBuffer::operator&(const FrameBuffer& buffer)const
//{
//	if(width() != buffer.width() || height() != buffer.height()){
//		throw std::runtime_error(__func__);
//	}
//	FrameBuffer result = FrameBuffer(width(), height());
//
//
//	return result;
//}

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

void            FrameBuffer::lshifter::operator()(      Pixel<uint16_t>& pixel)const{pixel <<= shift_;}
Pixel<uint16_t> FrameBuffer::lshifter::operator()(const Pixel<uint16_t>& pixel)const{return pixel << shift_;}
void            FrameBuffer::rshifter::operator()(      Pixel<uint16_t>& pixel)const{pixel >>= shift_;}
Pixel<uint16_t> FrameBuffer::rshifter::operator()(const Pixel<uint16_t>& pixel)const{return pixel >> shift_;}

#ifdef ENABLE_TIFF
void FrameBuffer::read_tiff(const std::string& filename)
{
	TIFF* image = TIFFOpen(filename.c_str(), "r");
	if(!image){
		throw std::runtime_error(__func__);
	}

	uint16_t bits_per_sample = 0;
	uint16_t samples_per_pixel = 0;
	uint16_t photometric = 0;
	uint32_t image_length = 0;
	uint32_t image_width = 0;
	if(!TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bits_per_sample) ||
		!TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) ||
		!TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photometric) ||
		!TIFFGetField(image, TIFFTAG_IMAGELENGTH, &image_length) ||
		!TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &image_width)){
		TIFFClose(image);
		throw std::runtime_error(__func__);
	}
	if(photometric != PHOTOMETRIC_RGB){
		std::cerr << __func__ << ": not supported photometric: " << photometric << std::endl;
	}

	width_  = image_width;
	height_ = image_length;
	head_   = new uint8_t[data_size()];

	const tmsize_t strip_size = TIFFStripSize(image);
	const uint32_t num_strips = TIFFNumberOfStrips(image);
	tmsize_t offset = 0;
	for(uint32_t i = 0; i < num_strips; ++i){
		tmsize_t read_result = 0;
		if((read_result = TIFFReadEncodedStrip(image, i, head_ + offset, strip_size)) == -1){
			std::cerr << __func__ << std::endl;
			TIFFClose(image);
			throw std::runtime_error(__func__);
		}
		offset += read_result;
	}

	switch(bits_per_sample){
	case 8:
		for(uint8_t *p = head_ + offset - 1, *last = tail() - 1; head_ <= p; --p, last -= 2){
			*last = *p;
		}
		break;
	case 16:
		break;
	default:
		throw std::runtime_error(__func__ +  std::string("not supported bit depth"));
	}

	TIFFClose(image);
}

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
void FrameBuffer::read_png(const std::string& filename)
{
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
	head_   = new uint8_t[data_size()];

	for(uint32_t i = 0; i < height_; ++i){
		std::copy(&row_ptrs[i][0], &row_ptrs[i][width_*pixelsize], reinterpret_cast<uint8_t*>(&this->operator[](i)[0]));
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	std::fclose(fp);
}

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
