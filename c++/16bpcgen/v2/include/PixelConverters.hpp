#ifndef _16BPCGEN_PIXELCONVERTERS_HPP_
#define _16BPCGEN_PIXELCONVERTERS_HPP_

#include <vector>
#include "Pixel.hpp"
#include "PixelConverter.hpp"
#include "typedef.hpp"

class Channel: public PixelConverter{
public:
	enum{
		R = 0x1,
		G = 0x2,
		B = 0x4
	};
	typedef uint8_t Ch;
	Channel(Ch ch = R | G | B): ch_(ch){}
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
	Ch ch()const{return ch_;}
private:
	const Ch ch_;
};

class GrayScale: public PixelConverter{
public:
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
};

class Threshold: public Channel{
public:
	Threshold(Pixel<uint16_t>::value_type threshold, Ch ch):
		Channel(ch), threshold_(threshold){}
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
private:
	const Pixel<uint16_t>::value_type threshold_;
};

class Offset: public Channel{
public:
	Offset(Pixel<uint16_t>::value_type offset, bool invert = false, Ch ch = R | G | B):
		Channel(ch), offset_(offset), invert_(invert){}
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
private:
	const Pixel<uint16_t>::value_type offset_;
	const bool invert_;
};

class Reversal: public Channel{
public:
	Reversal(Ch ch = R | G | B): Channel(ch){}
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
};

class Gamma: public Channel{
public:
	Gamma(const std::vector<Pixel<uint16_t>::value_type>& lut, Ch ch = R | G | B);
	virtual Pixel<uint16_t>& convert(Pixel<uint16_t>& pixel)const;
private:
	std::vector<Pixel<uint16_t>::value_type> lut_;
};

#endif
