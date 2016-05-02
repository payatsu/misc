#ifndef _16BPCGEN_PIXELCONVERTER_HPP_
#define _16BPCGEN_PIXELCONVERTER_HPP_

template <typename T> class Pixel;

class PixelConverter{
public:
	virtual ~PixelConverter(){}
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const = 0;
};

#endif
