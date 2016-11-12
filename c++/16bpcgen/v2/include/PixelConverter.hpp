#ifndef _16BPCGEN_PIXELCONVERTER_HPP_
#define _16BPCGEN_PIXELCONVERTER_HPP_

#include <Pixel.hpp>

class PixelConverter{
public:
	virtual ~PixelConverter(){}
	virtual Pixel<>& convert(Pixel<>& pixel)const = 0;
};

#endif
