#ifndef 16BPCGEN_PIXELCONVERTERS_HPP_
#define 16BPCGEN_PIXELCONVERTERS_HPP_

#include <vector>
#include "PixelConverter.hpp"

class Channel: public PixelConverter{
public:
	enum{
		R = 0x1,
		G = 0x2,
		B = 0x4
	};
	typedef byte_t Ch;
	Channel(Ch c = R | G | B): ch_(c){}
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
	Ch ch()const{return ch_;}
private:
	const Ch ch_;
};

class GrayScale: public PixelConverter{
public:
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
};

class Threshold: public Channel{
public:
	Threshold(Image::pixel_type::value_type threshold, Ch c):
		Channel(c), threshold_(threshold){}
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
private:
	const Image::pixel_type::value_type threshold_;
};

class Offset: public Channel{
public:
	Offset(Image::pixel_type::value_type offset, bool invert = false, Ch c = R | G | B):
		Channel(c), offset_(offset), invert_(invert){}
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
private:
	const Image::pixel_type::value_type offset_;
	const bool invert_;
};

class Reversal: public Channel{
public:
	Reversal(Ch c = R | G | B): Channel(c){}
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
};

class Gamma: public Channel{
public:
	Gamma(const std::vector<Image::pixel_type::value_type>& lut, Ch c = R | G | B);
	virtual Image::pixel_type& convert(Image::pixel_type& pixel)const;
private:
	std::vector<Image::pixel_type::value_type> lut_;
};

#endif
