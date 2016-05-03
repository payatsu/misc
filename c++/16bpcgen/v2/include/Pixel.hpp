#ifndef _16BPCGEN_PIXEL_HPP_
#define _16BPCGEN_PIXEL_HPP_

#include "typedef.hpp"

template <typename T>
class Pixel{
public:
	typedef T value_type;
	const static value_type max = 0xffff;
	Pixel(value_type r = 0x0, value_type g = 0x0, value_type b = 0x0): R_(r), G_(g), B_(b){}
	Pixel(unsigned long long int binary): R_(binary>>32&max), G_(binary>>16&max), B_(binary&max){}
	template <typename U>
	Pixel(const Pixel<U>& pixel):
		R_(static_cast<value_type>(pixel.R())),
		G_(static_cast<value_type>(pixel.G())),
		B_(static_cast<value_type>(pixel.B())){}
	template <typename U>
	Pixel& operator=(const Pixel<U>& pixel)
	{
		if(this == reinterpret_cast<const Pixel*>(&pixel)){
			return *this;
		}
		R_ = static_cast<value_type>(pixel.R());
		G_ = static_cast<value_type>(pixel.G());
		B_ = static_cast<value_type>(pixel.B());
		return *this;
	}
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
	void R(value_type r){R_ = r;}
	void G(value_type g){G_ = g;}
	void B(value_type b){B_ = b;}
private:
	value_type R_;
	value_type G_;
	value_type B_;
};

extern const Pixel<uint16_t> black;
extern const Pixel<uint16_t> white;
extern const Pixel<uint16_t> red;
extern const Pixel<uint16_t> green;
extern const Pixel<uint16_t> blue;
extern const Pixel<uint16_t> cyan;
extern const Pixel<uint16_t> magenta;
extern const Pixel<uint16_t> yellow;

#endif
