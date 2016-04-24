#ifndef _16BPCGEN_PIXELCONVERTER_HPP_
#define _16BPCGEN_PIXELCONVERTER_HPP_

class Pixel;

class PixelConverter{
public:
	virtual ~PixelConverter(){}
	virtual Pixel& convert(Pixel& pixel)const = 0;
};

#endif
