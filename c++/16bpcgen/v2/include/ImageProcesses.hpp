#ifndef _16BPCGEN_IMAGEPROCESSES_HPP_
#define _16BPCGEN_IMAGEPROCESSES_HPP_

#include "ImageProcess.hpp"
#include "typedef.hpp"
class PixelConverter;

class Area{
public:
	Area(uint32_t w = 0, uint32_t h = 0, uint32_t x = 0, uint32_t y = 0):
		width_(w), height_(h), offset_x_(x), offset_y_(y){}
	const uint32_t width_;
	const uint32_t height_;
	const uint32_t offset_x_;
	const uint32_t offset_y_;
};

class AreaSpecifier: public ImageProcess{
public:
	AreaSpecifier(const Area& area = Area()): area_(area){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const = 0;
	bool is_in(const FrameBuffer& buffer)const;
protected:
	const Area& area_;
};

class Tone: public AreaSpecifier{
public:
	Tone(const PixelConverter& converter, const Area& area = Area()):
		AreaSpecifier(area), converter_(converter){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
private:
	const PixelConverter& converter_;
};

class Crop: public AreaSpecifier{
public:
	Crop(const Area& area): AreaSpecifier(area){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
};

#endif
