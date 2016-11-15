#ifndef _16BPCGEN_PAINTER_HPP_
#define _16BPCGEN_PAINTER_HPP_

#if 201103L <= __cplusplus
#	include <random>
#endif
#include "Image.hpp"

class Painter{
public:
	virtual ~Painter(){}
	virtual Image::pixel_type operator()() = 0;
};

class UniColor: public Painter{
public:
	UniColor(const Image::pixel_type& pixel): pixel_(pixel){}
	virtual Image::pixel_type operator()(){return pixel_;}
private:
	const Image::pixel_type pixel_;
};

class Gradator: public Painter{
public:
	Gradator(const Image::pixel_type& step, const Image::pixel_type& initial = black, bool invert = false):
		step_(step), state_(initial), invert_(invert){}
	virtual Image::pixel_type operator()()
	{
		Image::pixel_type tmp = state_;
		state_ = invert_ ? state_ - step_ : state_ + step_;
		return tmp;
	}
private:
	Image::pixel_type step_;
	Image::pixel_type state_;
	bool invert_;
};

#if 201103L <= __cplusplus
class RandomColor: public Painter{
public:
	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
	virtual Image::pixel_type operator()(){return Image::pixel_type{distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
private:
	std::mt19937 engine_;
	std::uniform_int_distribution<Image::pixel_type::value_type> distribution_;
};
#endif

#endif
