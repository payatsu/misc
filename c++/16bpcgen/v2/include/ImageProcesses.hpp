#ifndef _16BPCGEN_IMAGEPROCESSES_HPP_
#define _16BPCGEN_IMAGEPROCESSES_HPP_

#include <vector>
#include "ImageProcess.hpp"
#include "Pixel.hpp"
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
	bool within(const FrameBuffer& buffer)const;
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

class Normalize: public AreaSpecifier{
public:
	Normalize(const Area& area = Area()): AreaSpecifier(area){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
};

class Median: public AreaSpecifier{
public:
	Median(const Area& area = Area()): AreaSpecifier(area){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
};

class Crop: public AreaSpecifier{
public:
	Crop(const Area& area): AreaSpecifier(area){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
};

class Filter: public ImageProcess{
public:
	typedef std::vector<double> KernelRow;
	typedef std::vector<KernelRow> Kernel;
	Filter(const Kernel& kernel): kernel_(kernel){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const;
private:
	Kernel kernel_;
};

class WeightedSmoothing: public Filter{
public:
	WeightedSmoothing(): Filter(init()){}
private:
	static Kernel init();
};

class UnSharpMask: public Filter{
public:
	UnSharpMask(): Filter(init()){}
private:
	static Kernel init();
};

class Prewitt: public Filter{
public:
	Prewitt(): Filter(init()){}
private:
	static Kernel init();
};

class Sobel: public Filter{
public:
	Sobel(): Filter(init()){}
private:
	static Kernel init();
};

class Laplacian3x3: public Filter{
public:
	Laplacian3x3(): Filter(init()){}
private:
	static Kernel init();
};

class Laplacian5x5: public Filter{
public:
	Laplacian5x5(): Filter(init()){}
private:
	static Kernel init();
};

#endif
