#ifndef _16BPCGEN_PIXEL_HPP_
#define _16BPCGEN_PIXEL_HPP_

#include "typedef.hpp"

class Pixel{
public:
	Pixel(uint16_t R = 0x0000, uint16_t G = 0x0000, uint16_t B = 0x0000): R_(R), G_(G), B_(B){}
	Pixel(unsigned long long int binary): R_(binary>>32&0xffff), G_(binary>>16&0xffff), B_(binary&0xffff){}
	Pixel operator+(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ + rhs.R_),
			static_cast<uint16_t>(G_ + rhs.G_),
			static_cast<uint16_t>(B_ + rhs.B_));
	}
	Pixel operator-(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ - rhs.R_),
			static_cast<uint16_t>(G_ - rhs.G_),
			static_cast<uint16_t>(B_ - rhs.B_));
	}
	Pixel operator*(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_*rhs),
			static_cast<uint16_t>(G_*rhs),
			static_cast<uint16_t>(B_*rhs));
	}
	Pixel operator/(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_/rhs),
			static_cast<uint16_t>(G_/rhs),
			static_cast<uint16_t>(B_/rhs));
	}
private:
	uint16_t R_;
	uint16_t G_;
	uint16_t B_;
};

extern const Pixel black;
extern const Pixel white;
extern const Pixel red;
extern const Pixel green;
extern const Pixel blue;
extern const Pixel cyan;
extern const Pixel magenta;
extern const Pixel yellow;

#endif
