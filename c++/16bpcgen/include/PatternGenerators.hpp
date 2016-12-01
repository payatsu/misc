#ifndef 16BPCGEN_PATTERN_GENERATORS_HPP_
#define 16BPCGEN_PATTERN_GENERATORS_HPP_

#include "PatternGenerator.hpp"
#include "Image.hpp"

class ColorBar: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};

class Luster: public PatternGenerator{
public:
	Luster(const Image::pixel_type& pixel): pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const Image::pixel_type pixel_;
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
	StairStepH(byte_t stairs = 2, byte_t steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual Image& generate(Image& image)const;
private:
	const byte_t stairs_;
	const byte_t steps_;
	const bool invert_;
};

class StairStepV: public PatternGenerator{
public:
	StairStepV(byte_t stairs = 2, byte_t steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual Image& generate(Image& image)const;
private:
	const byte_t stairs_;
	const byte_t steps_;
	const bool invert_;
};

class Ramp: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};

class CrossHatch: public PatternGenerator{
public:
	CrossHatch(column_t width, row_t height, const Image::pixel_type& pixel = white):
		lattice_width_(width), lattice_height_(height), pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const column_t lattice_width_;
	const row_t lattice_height_;
	const Image::pixel_type pixel_;
};

#if 201103L <= __cplusplus
class WhiteNoise: public PatternGenerator{
public:
	virtual Image& generate(Image& image)const;
};
#endif

extern const byte_t char_width;
extern const byte_t char_height;
extern const byte_t char_tab_width;
extern const byte_t char_bitmask[];
extern const byte_t characters[][8];

class Character: public PatternGenerator{
public:
	Character(const std::string& text, const Image::pixel_type& pixel = white,
			byte_t scale = 1, row_t row = 0, column_t column = 0):
		text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
	virtual Image& generate(Image& image)const;
private:
	void write(Image& image, row_t row, column_t column,
			unsigned char c, const Image::pixel_type& pixel, byte_t scale)const;
	void write(Image& image, row_t row, column_t column,
			const std::string& str, const Image::pixel_type& pixel, byte_t scale)const;
private:
	const std::string text_;
	const Image::pixel_type pixel_;
	const byte_t scale_;
	const row_t row_;
	const column_t column_;
};

class TypeWriter: public PatternGenerator{
public:
	TypeWriter(const std::string& textfilename, const Image::pixel_type& pixel = white);
	virtual const column_t& width()const{return width_;}
	virtual const row_t& height()const{return height_;}
	virtual Image& generate(Image& image)const;
private:
	static bool is_tab(unsigned char c){return c == '\t';}
	column_t width_;
	row_t height_;
	std::string text_;
	const Image::pixel_type pixel_;
};

class Line: public PatternGenerator{
public:
	Line(column_t from_col, row_t from_row, column_t to_col, row_t to_row, const Image::pixel_type& pixel = white):
		from_col_(from_col), from_row_(from_row), to_col_(to_col), to_row_(to_row), pixel_(pixel){}
	virtual Image& generate(Image& image)const;
private:
	const column_t from_col_;
	const row_t from_row_;
	const column_t to_col_;
	const row_t to_row_;
	const Image::pixel_type pixel_;
};

class Circle: public PatternGenerator{
public:
	typedef column_t radius_t;
	Circle(column_t column, row_t row, const Image::pixel_type& pixel = white, radius_t radius = 0, bool fill_enabled = true):
		column_(column), row_(row), pixel_(pixel), radius_(radius), fill_enabled_(fill_enabled){}
	virtual Image& generate(Image& image)const;
private:
	const column_t column_;
	const row_t row_;
	const Image::pixel_type pixel_;
	const radius_t radius_;
	const bool fill_enabled_;
};

#endif
