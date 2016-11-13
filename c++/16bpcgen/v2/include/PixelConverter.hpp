#ifndef _16BPCGEN_PIXELCONVERTER_HPP_
#define _16BPCGEN_PIXELCONVERTER_HPP_

#include "Image.hpp"

class PixelConverter{
public:
	virtual ~PixelConverter(){}
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const = 0;
};

#endif
