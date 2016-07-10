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
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerator.hpp"
#include "Pixel.hpp"

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const unsigned int pixelsize = 6;

void Row::fill(Row first, Row last, const Row& row)
{
	while(first != last){
		std::copy(&row[0], &row[row.width()], &first[0]);
		++first;
	}
}

Image::Image(const std::string& filename): head_(NULL), width_(0), height_(0)
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

Image::Image(const Image& image):
	head_(new uint8_t[image.data_size()]), width_(image.width_), height_(image.height_)
{
	std::copy(image.head(), image.tail(), head());
}

Image& Image::operator=(const Image& image)
{
	if(this == &image){
		return *this;
	}
	delete[] head_;
	width_  = image.width();
	height_ = image.height();
	head_   = new uint8_t[data_size()];
	std::copy(image.head(), image.tail(), head());
	return *this;
}

Image Image::operator<<(const PatternGenerator& generator)const
{
	Image result = Image(width(), height());
	return generator.generate(result);
}

Image& Image::operator<<=(const PatternGenerator& generator)
{
	return generator.generate(*this);
}

Image& Image::operator<<=(std::istream& is)
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

Image Image::operator>>(const ImageProcess& process)const
{
	Image result = Image(*this);
	return process.process(result);
}

Image& Image::operator>>=(const ImageProcess& process)
{
	return process.process(*this);
}

Image Image::operator>>(const PixelConverter& converter)const
{
	Image result = Image(*this);
	return Tone(converter).process(result);
}

Image& Image::operator>>=(const PixelConverter& converter)
{
	return Tone(converter).process(*this);
}

Image Image::operator<<(uint8_t shift)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], lshifter(shift));
	return result;
}

Image Image::operator>>(uint8_t shift)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], rshifter(shift));
	return result;
}

Image& Image::operator<<=(uint8_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], lshifter(shift));
	return *this;
}

Image& Image::operator>>=(uint8_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], rshifter(shift));
	return *this;
}

Image Image::operator+(const Image& image)const
{
	if(width() != image.width()){
		throw std::runtime_error(__func__);
	}
	Image result = Image(width(), height() + image.height());
	std::copy(head(), head() + data_size(), result.head());
	std::copy(image.head(), image.head() + image.data_size(), result.head() + data_size());
	return result;
}

Image Image::operator,(const Image& image)const
{
	if(height() != image.height()){
		throw std::runtime_error(__func__);
	}
	Image result = Image(width() + image.width(), height());
	for(uint32_t h = 0; h < height(); ++h){
		std::copy(head() + h * width() * pixelsize, head() + (h + 1) * width() * pixelsize, result.head() + h * result.width() * pixelsize);
		std::copy(image.head() + h * image.width() * pixelsize, image.head() + (h + 1) * image.width() * pixelsize, result.head() + width() * pixelsize + h * result.width() * pixelsize);
	}
	return result;
}

Image Image::operator&(const Image& image)const
{
	if(width() != image.width() || height() != image.height()){
		throw std::runtime_error(__func__);
	}
	Image result = Image(width(), height());
	for(uint32_t i = 0; i < data_size(); ++i){
		result.head()[i] = head()[i] & image.head()[i];
	}
	return result;
}

Image Image::operator&(const Pixel<uint16_t>& pixel)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], bit_and(pixel));
	return result;
}

Image& Image::operator&=(const Pixel<uint16_t>& pixel)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], bit_and(pixel));
	return *this;
}

Image Image::operator&(uint16_t value)const
{
	return *this & Pixel<uint16_t>(value, value, value);
}

Image& Image::operator&=(uint16_t value)
{
	return *this &= Pixel<uint16_t>(value, value, value);
}

Image Image::operator|(const Image& image)const
{
	if(width() != image.width() || height() != image.height()){
		throw std::runtime_error(__func__);
	}
	Image result = Image(width(), height());
	for(uint32_t i = 0; i < data_size(); ++i){
		result.head()[i] = head()[i] | image.head()[i];
	}
	return result;
}

Image& Image::operator|=(const Pixel<uint16_t>& pixel)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], bit_or(pixel));
	return *this;
}

Image Image::operator|(const Pixel<uint16_t>& pixel)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const Pixel<uint16_t>*>(&(*this)[0][0]),
					const_cast<const Pixel<uint16_t>*>(&(*this)[height()][0]),
					&result[0][0], bit_or(pixel));
	return result;
}

Image& Image::swap(Image& rhs)
{
	if(this == &rhs){
		return *this;
	}
	uint8_t* const tmp_head   = head_;
	const uint32_t tmp_width  = width_;
	const uint32_t tmp_height = height_;
	head_   = rhs.head_;
	width_  = rhs.width_;
	height_ = rhs.height_;
	rhs.head_   = tmp_head;
	rhs.width_  = tmp_width;
	rhs.height_ = tmp_height;
	return *this;
}

Image& Image::write(const std::string& filename)const
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
		return const_cast<Image&>(*this);
	}
}

void            Image::lshifter::operator()(      Pixel<uint16_t>& pixel)const{pixel <<= shift_;}
Pixel<uint16_t> Image::lshifter::operator()(const Pixel<uint16_t>& pixel)const{return pixel << shift_;}
void            Image::rshifter::operator()(      Pixel<uint16_t>& pixel)const{pixel >>= shift_;}
Pixel<uint16_t> Image::rshifter::operator()(const Pixel<uint16_t>& pixel)const{return pixel >> shift_;}
void            Image::bit_and::operator()(      Pixel<uint16_t>& pixel)const{pixel &= pixel_;}
Pixel<uint16_t> Image::bit_and::operator()(const Pixel<uint16_t>& pixel)const{return pixel & pixel_;}
void            Image::bit_or::operator()(      Pixel<uint16_t>& pixel)const{pixel |= pixel_;}
Pixel<uint16_t> Image::bit_or::operator()(const Pixel<uint16_t>& pixel)const{return pixel | pixel_;}

#ifdef ENABLE_TIFF
void Image::read_tiff(const std::string& filename)
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

Image& Image::write_tiff(const std::string& filename)const
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
	return const_cast<Image&>(*this);
}
#endif

#ifdef ENABLE_PNG
void Image::read_png(const std::string& filename)
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

Image& Image::write_png(const std::string& filename)const
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
	return const_cast<Image&>(*this);
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
