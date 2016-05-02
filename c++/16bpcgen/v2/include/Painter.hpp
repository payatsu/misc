#ifndef _16BPCGEN_PAINTER_HPP_
#define _16BPCGEN_PAINTER_HPP_

#if 201103L <= __cplusplus
#	include <random>
#endif
#include "Pixel.hpp"

class Painter{
public:
	virtual ~Painter(){}
	virtual Pixel<uint16_t> operator()() = 0;
};

class UniColor: public Painter{
public:
	UniColor(const Pixel<uint16_t>& pixel): pixel_(pixel){}
	virtual Pixel<uint16_t> operator()(){return pixel_;}
private:
	const Pixel<uint16_t> pixel_;
};

class Gradator: public Painter{
public:
	Gradator(const Pixel<uint16_t>& step, const Pixel<uint16_t>& initial = black, bool invert = false):
		step_(step), state_(initial), invert_(invert){}
	virtual Pixel<uint16_t> operator()()
	{
		Pixel<uint16_t> tmp = state_;
		state_ = invert_ ? state_ - step_ : state_ + step_;
		return tmp;
	}
private:
	Pixel<uint16_t> step_;
	Pixel<uint16_t> state_;
	bool invert_;
};

#if 201103L <= __cplusplus
class RandomColor: public Painter{
public:
	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
	virtual Pixel<uint16_t> operator()(){return {distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
private:
	std::mt19937 engine_;
	std::uniform_int_distribution<Pixel<uint16_t>::value_type> distribution_;
};
#endif

#endif
