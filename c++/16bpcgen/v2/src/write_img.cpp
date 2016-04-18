#include <cstdio>
#ifdef ENABLE_PNG
#	include <libpng16/png.h>
#endif
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
#include "write_img.hpp"

#ifdef ENABLE_TIFF
void generate_16bpc_tiff(const std::string& output_filename, FrameBuffer& buffer)
{
	TIFF* image = TIFFOpen(output_filename.c_str(), "w");
	if(!image){
		std::perror("");
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

std::string append_extension(const std::string& filename, const std::string& ext)
{
	const std::string::size_type idx = filename.find(ext);
	if(idx == std::string::npos || idx + ext.size() != filename.size()){
		return filename + ext;
	}else{
		return filename;
	}
}
