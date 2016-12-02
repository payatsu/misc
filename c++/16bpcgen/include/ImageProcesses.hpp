#ifndef BPCGEN_IMAGEPROCESSES_HPP_
#define BPCGEN_IMAGEPROCESSES_HPP_

#include <vector>
#include "ImageProcess.hpp"
class PixelConverter;

class Area{
public:
	Area(column_t w = 0, row_t h = 0, column_t x = 0, row_t y = 0):
		width_(w), height_(h), offset_x_(x), offset_y_(y){}
	const column_t width_;
	const row_t height_;
	const column_t offset_x_;
	const row_t offset_y_;
};

class AreaSpecifier: public ImageProcess{
public:
	AreaSpecifier(const Area& area = Area()): area_(area){}
	virtual Image& process(Image& image)const = 0;
	bool within(const Image& image)const;
protected:
	const Area& area_;
};

class Tone: public AreaSpecifier{
public:
	Tone(const PixelConverter& converter, const Area& area = Area()):
		AreaSpecifier(area), converter_(converter){}
	virtual Image& process(Image& image)const;
private:
	const PixelConverter& converter_;
};

class Normalize: public AreaSpecifier{
public:
	Normalize(const Area& area = Area()): AreaSpecifier(area){}
	virtual Image& process(Image& image)const;
};

class Median: public AreaSpecifier{
public:
	Median(const Area& area = Area()): AreaSpecifier(area){}
	virtual Image& process(Image& image)const;
};

class Crop: public AreaSpecifier{
public:
	Crop(const Area& area): AreaSpecifier(area){}
	virtual Image& process(Image& image)const;
};

class Filter: public ImageProcess{
public:
	typedef std::vector<double> KernelRow;
	typedef std::vector<KernelRow> Kernel;
	Filter(const Kernel& kernel): kernel_(kernel){}
	virtual Image& process(Image& image)const;
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

class HScale: public ImageProcess{
public:
	HScale(column_t width): width_(width){}
	virtual Image& process(Image& image)const;
private:
	column_t width_;
};

class VScale: public ImageProcess{
public:
	VScale(row_t height): height_(height){}
	virtual Image& process(Image& image)const;
private:
	row_t height_;
};

class KeyStone: public ImageProcess{
public:
	enum Vertex{
		TOP_LEFT,
		TOP_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_RIGHT
	};
	KeyStone(Vertex vertex, column_t width_offset, row_t height_offset):
		vertex_(vertex), width_offset_(width_offset), height_offset_(height_offset){}
	virtual Image& process(Image& image)const;
private:
	Vertex vertex_;
	column_t width_offset_;
	row_t height_offset_;
};

#endif
