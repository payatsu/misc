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
	uint16_t samples_per_pixel = 0;
	uint16_t photometric = 0;
	uint32_t length = 0;
	uint32_t width = 0;
	if(!TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bits_per_sample) ||
	   !TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) ||
	   !TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &photometric) ||
	   !TIFFGetField(image, TIFFTAG_IMAGELENGTH, &length) ||
	   !TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width)){
		TIFFClose(image);
		result = EXIT_FAILURE;
		return FrameBuffer(0, 0);
	}
	FrameBuffer buffer = FrameBuffer(width, length);

	const tmsize_t strip_size = TIFFStripSize(image);
	const uint32_t num_strips = TIFFNumberOfStrips(image);
	tmsize_t offset = 0;
	for(uint32_t i = 0; i < num_strips; ++i){
		tmsize_t read_result = 0;
		if((read_result = TIFFReadEncodedStrip(image, i, buffer.head() + offset, strip_size)) == -1){
			std::fprintf(stderr, "%s %d\n", __func__, i);
			TIFFClose(image);
			result = EXIT_FAILURE;
			return FrameBuffer(0, 0);
		}
		offset += read_result;
	}

	// XXX bit depth conversion
	for(uint8_t *p = buffer.head() + offset - 1, *tail = buffer.tail() - 1; buffer.head() <= p; --p, tail -= 2){
		*tail = *p;
	}

	// TODO photometric

	TIFFClose(image);
	result = EXIT_SUCCESS;
	return buffer;
}
#endif
