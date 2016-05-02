#ifndef _16BPCGEN_PIXEL_HPP_
#define _16BPCGEN_PIXEL_HPP_

#include "typedef.hpp"

class Pixel{
public:
	typedef uint16_t value_type;
	const static value_type max = 0xffff;
	Pixel(value_type R = 0x0, value_type G = 0x0, value_type B = 0x0): R_(R), G_(G), B_(B){}
	Pixel(unsigned long long int binary): R_(binary>>32&max), G_(binary>>16&max), B_(binary&max){}
	Pixel operator+(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ + rhs.R_),
			static_cast<value_type>(G_ + rhs.G_),
			static_cast<value_type>(B_ + rhs.B_));
	}
	Pixel operator-(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ - rhs.R_),
			static_cast<value_type>(G_ - rhs.G_),
			static_cast<value_type>(B_ - rhs.B_));
	}
	Pixel operator*(value_type rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ * rhs),
			static_cast<value_type>(G_ * rhs),
			static_cast<value_type>(B_ * rhs));
	}
	Pixel operator/(value_type rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ / rhs),
			static_cast<value_type>(G_ / rhs),
			static_cast<value_type>(B_ / rhs));
	}
	value_type R()const{return R_;}
	value_type G()const{return G_;}
	value_type B()const{return B_;}
	void R(value_type R){R_ = R;}
	void G(value_type G){G_ = G;}
	void B(value_type B){B_ = B;}
private:
	value_type R_;
	value_type G_;
	value_type B_;
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
