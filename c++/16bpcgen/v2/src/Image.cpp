#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#ifdef ENABLE_TIFF
#include <tiffio.h>
#endif
#ifdef ENABLE_PNG
#define PNG_NO_SETJMP
#include <png.h>
#endif
#ifdef ENABLE_JPEG
#include <jpeglib.h>
#endif
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerator.hpp"

const byte_t bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const byte_t pixelsize = 6;

void Row::fill(Row first, Row last, const Row& row)
{
	while(first != last){
		std::copy(&row[0], &row[row.width()], &first[0]);
		++first;
	}
}

Image::Image(const Image& image):
	head_(new byte_t[image.data_size()]), width_(image.width_), height_(image.height_)
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
	head_   = new byte_t[data_size()];
	std::copy(image.head(), image.tail(), head());
	return *this;
}

Image Image::operator<<(const PatternGenerator& generator)const
{
	Image result = Image(*this);
	return generator.generate(result);
}

Image& Image::operator<<=(const PatternGenerator& generator)
{
	return generator.generate(*this);
}

Image& Image::operator<<=(std::istream& is)
{
	const std::size_t size = data_size();
	for(std::size_t i = 0; i < size; ++i){
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

Image Image::operator<<(byte_t shift)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const pixel_type*>(&(*this)[0][0]),
					const_cast<const pixel_type*>(&(*this)[height()][0]),
					&result[0][0], lshifter(shift));
	return result;
}

Image& Image::operator<<=(byte_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], lshifter(shift));
	return *this;
}

Image Image::operator>>(byte_t shift)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const pixel_type*>(&(*this)[0][0]),
					const_cast<const pixel_type*>(&(*this)[height()][0]),
					&result[0][0], rshifter(shift));
	return result;
}

Image& Image::operator>>=(byte_t shift)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], rshifter(shift));
	return *this;
}

Image Image::operator&(const Image& image)const
{
	if(width() != image.width() || height() != image.height()){
		throw std::invalid_argument(__func__ + std::string(": image width/height unmatch"));
	}
	Image result = Image(width(), height());
	for(std::size_t i = 0; i < data_size(); ++i){
		result.head()[i] = head()[i] & image.head()[i];
	}
	return result;
}

Image Image::operator&(const Image::pixel_type& pixel)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const pixel_type*>(&(*this)[0][0]),
					const_cast<const pixel_type*>(&(*this)[height()][0]),
					&result[0][0], bit_and(pixel));
	return result;
}

Image& Image::operator&=(const Image::pixel_type& pixel)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], bit_and(pixel));
	return *this;
}

Image Image::operator|(const Image& image)const
{
	if(width() != image.width() || height() != image.height()){
		throw std::invalid_argument(__func__ + std::string(": image width/height unmatch"));
	}
	Image result = Image(width(), height());
	for(std::size_t i = 0; i < data_size(); ++i){
		result.head()[i] = head()[i] | image.head()[i];
	}
	return result;
}

Image Image::operator|(const Image::pixel_type& pixel)const
{
	Image result = Image(width(), height());
	std::transform(const_cast<const pixel_type*>(&(*this)[0][0]),
					const_cast<const pixel_type*>(&(*this)[height()][0]),
					&result[0][0], bit_or(pixel));
	return result;
}

Image& Image::operator|=(const Image::pixel_type& pixel)
{
	std::for_each(&(*this)[0][0], &(*this)[height()][0], bit_or(pixel));
	return *this;
}

Image Image::operator()(const Image& image, byte_t orientation)const
{
	if(orientation & ORI_HORI && height() == image.height()){
		Image result = Image(width() + image.width(), height());
		for(column_t h = 0; h < height(); ++h){
			std::copy(head() + h * width() * pixelsize, head() + (h + 1) * width() * pixelsize, result.head() + h * result.width() * pixelsize);
			std::copy(image.head() + h * image.width() * pixelsize, image.head() + (h + 1) * image.width() * pixelsize, result.head() + width() * pixelsize + h * result.width() * pixelsize);
		}
		return result;
	}else if(orientation & ORI_VERT && width() == image.width()){
		Image result = Image(width(), height() + image.height());
		std::copy(head(), head() + data_size(), result.head());
		std::copy(image.head(), image.head() + image.data_size(), result.head() + data_size());
		return result;
	}else{
		throw std::invalid_argument(__func__ + std::string(": can not join. image width/height unmatch"));
	}
}

Image& Image::swap(Image& rhs)
{
	if(this == &rhs){
		return *this;
	}
	byte_t* const  tmp_head   = head_;
	const column_t tmp_width  = width_;
	const row_t    tmp_height = height_;
	head_   = rhs.head_;
	width_  = rhs.width_;
	height_ = rhs.height_;
	rhs.head_   = tmp_head;
	rhs.width_  = tmp_width;
	rhs.height_ = tmp_height;
	return *this;
}

Image& Image::read(const std::string& filename)
{
	if(has_ext(filename, ".tif") || has_ext(filename, ".tiff")){
#ifdef ENABLE_TIFF
		read_tiff(filename);
#else
		throw std::runtime_error(__func__ + std::string(": can not read. unsupported file format: ") + filename);
#endif
	}else if(has_ext(filename, ".png")){
#ifdef ENABLE_PNG
		read_png(filename);
#else
		throw std::runtime_error(__func__ + std::string(": can not read. unsupported file format: ") + filename);
#endif
	}else if(has_ext(filename, ".jpg") || has_ext(filename, ".jpeg")){
#ifdef ENABLE_JPEG
		read_jpeg(filename);
#else
		throw std::runtime_error(__func__ + std::string(": can not read. unsupported file format: ") + filename);
#endif
	}else{
		throw std::runtime_error(__func__ + std::string(": can not read. unsupported file format: ") + filename);
	}
	return *this;
}

Image& Image::write(const std::string& filename)const
{
	if(has_ext(filename, ".tif") || has_ext(filename, ".tiff")){
#ifdef ENABLE_TIFF
		write_tiff(filename);
#else
		throw std::runtime_error(__func__ + std::string(": can not write. unsupported file format: ") + filename);
#endif
	}else if(has_ext(filename, ".png")){
#ifdef ENABLE_PNG
		write_png(filename);
#else
		throw std::runtime_error(__func__ + std::string(": can not write. unsupported file format: ") + filename);
#endif
	}else{
#ifdef ENABLE_TIFF
		write_tiff(filename + ".tif");
#endif
#ifdef ENABLE_PNG
		write_png(filename + ".png");
#endif
#if !defined(ENABLE_TIFF) && !defined(ENABLE_PNG)
		throw std::runtime_error(__func__ + std::string(": can not write. no available file format: ") + filename);
#endif
	}
	return const_cast<Image&>(*this);
}

void              Image::lshifter::operator()(      Image::pixel_type& pixel)const{pixel <<= shift_;}
Image::pixel_type Image::lshifter::operator()(const Image::pixel_type& pixel)const{return pixel << shift_;}
void              Image::rshifter::operator()(      Image::pixel_type& pixel)const{pixel >>= shift_;}
Image::pixel_type Image::rshifter::operator()(const Image::pixel_type& pixel)const{return pixel >> shift_;}
void              Image::bit_and:: operator()(      Image::pixel_type& pixel)const{pixel &= pixel_;}
Image::pixel_type Image::bit_and:: operator()(const Image::pixel_type& pixel)const{return pixel & pixel_;}
void              Image::bit_or::  operator()(      Image::pixel_type& pixel)const{pixel |= pixel_;}
Image::pixel_type Image::bit_or::  operator()(const Image::pixel_type& pixel)const{return pixel | pixel_;}

#ifdef ENABLE_TIFF
void Image::read_tiff(const std::string& filename)
{
	TIFF* tif = TIFFOpen(filename.c_str(), "r");
	if(!tif){
		throw std::invalid_argument(__func__);
	}

	uint16_t bits_per_sample   = 0;
	uint16_t samples_per_pixel = 0;
	uint16_t photometric       = 0;
	uint32_t image_length      = 0;
	uint32_t image_width       = 0;
	if( !TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE,   &bits_per_sample)   ||
		!TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) ||
		!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC,     &photometric)       ||
		!TIFFGetField(tif, TIFFTAG_IMAGELENGTH,     &image_length)      ||
		!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,      &image_width)){
		TIFFClose(tif);
		throw std::runtime_error(__func__);
	}
	if(photometric != PHOTOMETRIC_RGB){
		std::ostringstream oss;
		oss << __func__ << ": can not read. unsupported photometric: " << std::hex << std::setw(4) << photometric;
		TIFFClose(tif);
		throw std::runtime_error(oss.str());
	}

	width_  = image_width;
	height_ = image_length;
	head_   = new byte_t[data_size()];

	const tmsize_t strip_size = TIFFStripSize(tif);
	const uint32_t num_strips = TIFFNumberOfStrips(tif);
	tmsize_t offset = 0;
	for(uint32_t i = 0; i < num_strips; ++i){
		tmsize_t read_result = 0;
		if((read_result = TIFFReadEncodedStrip(tif, i, head_ + offset, strip_size)) == -1){
			TIFFClose(tif);
			throw std::runtime_error(__func__);
		}
		offset += read_result;
	}

	switch(bits_per_sample){
	case 8:
		for(byte_t *p = head_ + offset - 1, *last = tail() - 1; head_ <= p; --p, last -= 2){
			*last = *p;
		}
		break;
	case 16:
		break;
	default:
		std::ostringstream oss;
		oss << __func__ << ": can not read. unsupported bit depth: " << bits_per_sample;
		throw std::runtime_error(oss.str());
	}

	TIFFClose(tif);
}

Image& Image::write_tiff(const std::string& filename)const
{
	TIFF* tif = TIFFOpen(filename.c_str(), "w");
	if(!tif){
		throw std::runtime_error(__func__);
	}
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width());
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height());
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitdepth);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, height());
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_XRESOLUTION, 163.44);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, 163.44);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "powered by " PROGRAM_NAME);
	TIFFWriteEncodedStrip(tif, 0, head(), data_size());
	TIFFClose(tif);
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

	const byte_t number = 8;
	byte_t signature[number];
	if(std::fread(signature, sizeof(byte_t), number, fp) != number){
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
	byte_t** row_ptrs = png_get_rows(png_ptr, info_ptr);

	width_  = png_get_image_width(png_ptr, info_ptr);
	height_ = png_get_image_height(png_ptr, info_ptr);
	head_   = new byte_t[data_size()];

	for(row_t i = 0; i < height(); ++i){
		std::copy(&row_ptrs[i][0], &row_ptrs[i][width()*pixelsize], reinterpret_cast<byte_t*>(&this->operator[](i)[0]));
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

	byte_t** row_ptrs = new byte_t*[height()];
	for(row_t i = 0; i < height(); ++i){
		row_ptrs[i] = reinterpret_cast<byte_t*>(&this->operator[](i)[0]);
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ENDIAN, NULL);

	delete[] row_ptrs;
	png_destroy_write_struct(&png_ptr, &info_ptr);
	std::fclose(fp);
	return const_cast<Image&>(*this);
}
#endif

#ifdef ENABLE_JPEG
void Image::read_jpeg(const std::string& filename)
{
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	FILE* fp = fopen(filename.c_str(), "rb");
	if(!fp){
		std::perror(__func__);
		throw std::runtime_error(__func__);
	}
	jpeg_stdio_src(&cinfo, fp);

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;

	jpeg_start_decompress(&cinfo);
	width_  = cinfo.output_width;
	height_ = cinfo.output_height;
	head_   = new byte_t[data_size()];

	JSAMPARRAY img = new JSAMPROW[height()];
	for(row_t i = 0; i < height(); ++i){
		img[i] = head_ + i*width()*3;
	}
	while(cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, img + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
	}
	delete[] img;

	jpeg_finish_decompress(&cinfo);
	std::fclose(fp);
	jpeg_destroy_decompress(&cinfo);

	for(byte_t *p = head_ + data_size()/2 - 1, *last = tail() - 1; head_ <= p; --p, last -= 2){
		*last = *p;
	}
}
#endif
