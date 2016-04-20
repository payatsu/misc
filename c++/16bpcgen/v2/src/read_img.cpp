#include <cstdio>
#ifdef ENABLE_PNG
#	define PNG_NO_SETJMP
#	include <libpng16/png.h>
#endif
#ifdef ENABLE_TIFF
#	include <tiffio.h>
#endif
#include "read_img.hpp"

#ifdef ENABLE_TIFF
FrameBuffer read_16bpc_tiff(const std::string& input_filename, int& result)
{
	return FrameBuffer(0, 0);
}
#endif

#ifdef ENABLE_PNG
FrameBuffer read_16bpc_png(const std::string& input_filename, int& result)
{
	FILE* fp = fopen(input_filename.c_str(), "rb");
	if(!fp){
		std::perror(__func__);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	const int number = 8;
	unsigned char header[number];
	if(std::fread(header, 1, number, fp) != number){
		std::perror(__func__);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}
	if(png_sig_cmp(header, 0, number)){
		std::fprintf(stderr, "not PNG\n");
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
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

	FrameBuffer buffer = FrameBuffer(png_get_image_width(png_ptr, info_ptr),
									 png_get_image_height(png_ptr, info_ptr));
	for(uint32_t i = 0; i < buffer.height(); ++i){
		std::copy(&row_ptrs[i][0], &row_ptrs[i][buffer.width()*pixelsize], reinterpret_cast<uint8_t*>(&buffer[i][0]));
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	result = EXIT_SUCCESS;
	return buffer;
}
#endif
