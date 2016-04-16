#ifndef _16BPCGEN_PATTERN_GENERATOR_HPP_
#define _16BPCGEN_PATTERN_GENERATOR_HPP_

#include "FrameBuffer.hpp"

class ColorBar: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const;
};

class Luster: public PatternGenerator{
public:
	Luster(const Pixel& pixel): pixel_(pixel){}
	virtual void generate(FrameBuffer& buffer)const;
private:
	const Pixel pixel_;
};

class Checker: public PatternGenerator{
public:
	Checker(bool invert = false): invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const;
private:
	const bool invert_;
};

class StairStepH: public PatternGenerator{
public:
	StairStepH(int stairs = 2, int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const;
private:
	const int stairs_;
	const int steps_;
	const bool invert_;
};

class StairStepV: public PatternGenerator{
public:
	StairStepV(int stairs = 2, int steps = 20, bool invert = false):
		stairs_(stairs), steps_(steps), invert_(invert){}
	virtual void generate(FrameBuffer& buffer)const;
private:
	const int stairs_;
	const int steps_;
	const bool invert_;
};

class Ramp: public PatternGenerator{ public:
	virtual void generate(FrameBuffer& buffer)const;
};

class CrossHatch: public PatternGenerator{
public:
	CrossHatch(uint32_t width, uint32_t height): lattice_width_(width), lattice_height_(height){}
	virtual void generate(FrameBuffer& buffer)const;
private:
	const uint32_t lattice_width_;
	const uint32_t lattice_height_;
};

#if 201103L <= __cplusplus
class WhiteNoise: public PatternGenerator{
public:
	virtual void generate(FrameBuffer& buffer)const;
};
#endif

extern const unsigned char char_width;
extern const unsigned char char_height;
extern const unsigned char char_tab_width;
extern const unsigned char char_bitmask[];
extern const unsigned char characters[][8];

class Character: public PatternGenerator{
public:
	Character(const std::string& text, const Pixel& pixel = white,
			int scale = 1, uint32_t row = 0, uint32_t column = 0):
		text_(text), pixel_(pixel), scale_(scale), row_(row), column_(column){}
	virtual void generate(FrameBuffer& buffer)const;
	void write(FrameBuffer& buffer, uint32_t row, uint32_t column,
			unsigned char c, const Pixel& pixel, int scale)const;
	void write(FrameBuffer& buffer, uint32_t row, uint32_t column,
			const std::string& str, const Pixel& pixel, int scale)const;
private:
	const std::string text_;
	const Pixel pixel_;
	const int scale_;
	const uint32_t row_;
	const uint32_t column_;
};

class TypeWriter: public PatternGenerator{
public:
	TypeWriter(const std::string& textfilename);
	virtual const uint32_t& width()const{return width_;}
	virtual const uint32_t& height()const{return height_;}
	virtual void generate(FrameBuffer& buffer)const{buffer << Character(text_, white);}
private:
	static bool is_tab(char c){return c == '\t';}
	uint32_t width_;
	uint32_t height_;
	std::string text_;
};

#endif
