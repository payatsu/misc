#ifndef _16BPCGEN_PATTERN_GENERATORS_HPP_
#define _16BPCGEN_PATTERN_GENERATORS_HPP_

#include "PatternGenerator.hpp"
#include "Pixel.hpp"

class ColorBar: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};

class Luster: public PatternGenerator{
public:
	Luster(const Pixel<uint16_t>& pixel): pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const Pixel<uint16_t> pixel_;
};

class Checker: public PatternGenerator{
public:
	Checker(bool invert = false): invert_(invert){}
	virtual Image& generate(Image& image)const;
private:
	const bool invert_;
};

class StairStepH: public PatternGenerator{
public:
	StairStepH(unsigned int stairs = 2, unsigned int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual Image& generate(Image& image)const;
private:
	const unsigned int stairs_;
	const unsigned int steps_;
	const bool invert_;
};

class StairStepV: public PatternGenerator{
public:
	StairStepV(unsigned int stairs = 2, unsigned int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual Image& generate(Image& image)const;
private:
	const unsigned int stairs_;
	const unsigned int steps_;
	const bool invert_;
};

class Ramp: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};

class CrossHatch: public PatternGenerator{
public:
	CrossHatch(uint32_t width, uint32_t height, const Pixel<uint16_t>& pixel = white):
		lattice_width_(width), lattice_height_(height), pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const uint32_t lattice_width_;
	const uint32_t lattice_height_;
	const Pixel<uint16_t> pixel_;
};

#if 201103L <= __cplusplus
class WhiteNoise: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};
#endif

extern const unsigned char char_width;
extern const unsigned char char_height;
extern const unsigned char char_tab_width;
extern const unsigned char char_bitmask[];
extern const unsigned char characters[][8];

class Character: public PatternGenerator{
public:
	Character(const std::string& text, const Pixel<uint16_t>& pixel = white,
			unsigned int scale = 1, uint32_t row = 0, uint32_t column = 0):
		text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
	virtual Image& generate(Image& image)const;
private:
	void write(Image& image, uint32_t row, uint32_t column,
			unsigned char c, const Pixel<uint16_t>& pixel, unsigned int scale)const;
	void write(Image& image, uint32_t row, uint32_t column,
			const std::string& str, const Pixel<uint16_t>& pixel, unsigned int scale)const;
private:
	const std::string text_;
	const Pixel<uint16_t> pixel_;
	const unsigned int scale_;
	const uint32_t row_;
	const uint32_t column_;
};

class TypeWriter: public PatternGenerator{
public:
	TypeWriter(const std::string& textfilename, const Pixel<uint16_t>& pixel = white);
	virtual const uint32_t& width()const{return width_;}
	virtual const uint32_t& height()const{return height_;}
	virtual Image& generate(Image& image)const;
private:
	static bool is_tab(char c){return c == '\t';}
	uint32_t width_;
	uint32_t height_;
	std::string text_;
	const Pixel<uint16_t> pixel_;
};

class Line: public PatternGenerator{
public:
	Line(uint32_t from_col, uint32_t from_row, uint32_t to_col, uint32_t to_row, const Pixel<uint16_t>& pixel = white):
		from_col_(from_col), from_row_(from_row), to_col_(to_col), to_row_(to_row), pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const uint32_t from_col_;
	const uint32_t from_row_;
	const uint32_t to_col_;
	const uint32_t to_row_;
	const Pixel<uint16_t> pixel_;
};

class Circle: public PatternGenerator{
public:
	Circle(uint32_t column, uint32_t row, const Pixel<uint16_t>& pixel = white, uint32_t radius = 0, bool fill_enabled = true):
		column_(column), row_(row), pixel_(pixel), radius_(radius), fill_enabled_(fill_enabled){}
	virtual Image& generate(Image& image)const;
private:
	const uint32_t column_;
	const uint32_t row_;
	const Pixel<uint16_t> pixel_;
	const uint32_t radius_;
	const bool fill_enabled_;
};

#endif
