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
	TIFF* image = TIFFOpen(input_filename.c_str(), "r");
	if(!image){
		std::perror(__func__);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	uint16_t bits_per_sample = 0;
	if(!TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bits_per_sample)){
		std::fprintf(stderr, "%s\n", __func__);
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	uint16_t samples_per_pixel = 0;
	if(!TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel)){
		std::fprintf(stderr, "%s\n", __func__);
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	uint16_t photometric = 0;
	if(!TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photometric)){
		std::fprintf(stderr, "%s\n", __func__);
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	uint32_t length = 0;
	if(!TIFFGetField(image, TIFFTAG_IMAGELENGTH, &length)){
		std::fprintf(stderr, "%s\n", __func__);
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	uint32_t width = 0;
	if(!TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width)){
		std::fprintf(stderr, "%s\n", __func__);
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	FrameBuffer buffer = FrameBuffer(width, length);

	tmsize_t strip_size = TIFFStripSize(image);
	uint32_t num_strips = TIFFNumberOfStrips(image);
	tmsize_t offset = 0;
	tmsize_t read_result = 0;
	for(uint32_t i = 0; i < num_strips; ++i){
		if((read_result = TIFFReadEncodedStrip(image, i, buffer.head() + offset, strip_size)) == -1){
			std::fprintf(stderr, "%s %d\n", __func__, i);
			TIFFClose(image);
			result = EXIT_FAILURE;
			return FrameBuffer(0, 0);
		}
		offset += read_result;
	}

	// photometric

	// fill order

	TIFFClose(image);
	result = EXIT_SUCCESS;
	return buffer;
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
	unsigned char signature[number];
	if(std::fread(signature, sizeof(unsigned char), number, fp) != number){
		std::perror(__func__);
		std::fclose(fp);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}
	if(png_sig_cmp(signature, 0, number)){
		std::fprintf(stderr, "not PNG\n");
		std::fclose(fp);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		std::fclose(fp);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		std::fclose(fp);
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
	std::fclose(fp);
	result = EXIT_SUCCESS;
	return buffer;
}
#endif
