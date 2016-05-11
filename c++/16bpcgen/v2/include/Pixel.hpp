#ifndef _16BPCGEN_PIXEL_HPP_
#define _16BPCGEN_PIXEL_HPP_

#include "typedef.hpp"

template <typename T>
class Pixel{
public:
	typedef T value_type;
	const static value_type max;
	Pixel(value_type r = 0x0, value_type g = 0x0, value_type b = 0x0): R_(r), G_(g), B_(b){}
	template <typename U>
	Pixel(const Pixel<U>& rhs):
		R_(static_cast<value_type>(rhs.R())),
		G_(static_cast<value_type>(rhs.G())),
		B_(static_cast<value_type>(rhs.B())){}
	template <typename U>
	Pixel& operator=(const Pixel<U>& rhs)
	{
		if(this == reinterpret_cast<const Pixel*>(&rhs)){
			return *this;
		}
		R_ = static_cast<value_type>(rhs.R());
		G_ = static_cast<value_type>(rhs.G());
		B_ = static_cast<value_type>(rhs.B());
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
	Pixel operator&(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ & rhs.R_),
			static_cast<value_type>(G_ & rhs.G_),
			static_cast<value_type>(B_ & rhs.B_));
	}
	Pixel operator&=(const Pixel& rhs)
	{
		R_ &= rhs.R_;
		G_ &= rhs.G_;
		B_ &= rhs.B_;
		return *this;
	}
	Pixel operator|(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ | rhs.R_),
			static_cast<value_type>(G_ | rhs.G_),
			static_cast<value_type>(B_ | rhs.B_));
	}
	Pixel operator|=(const Pixel& rhs)
	{
		R_ |= rhs.R_;
		G_ |= rhs.G_;
		B_ |= rhs.B_;
		return *this;
	}
	Pixel operator*(value_type rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ * rhs),
			static_cast<value_type>(G_ * rhs),
			static_cast<value_type>(B_ * rhs));
	}
	Pixel& operator*=(value_type rhs)
	{
		R_ *= rhs.R_;
		G_ *= rhs.G_;
		B_ *= rhs.B_;
		return *this;
	}
	Pixel operator/(value_type rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ / rhs),
			static_cast<value_type>(G_ / rhs),
			static_cast<value_type>(B_ / rhs));
	}
	Pixel& operator/=(value_type rhs)
	{
		R_ /= rhs.R_;
		G_ /= rhs.G_;
		B_ /= rhs.B_;
		return *this;
	}
	Pixel operator<<(uint8_t rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ << rhs),
			static_cast<value_type>(G_ << rhs),
			static_cast<value_type>(B_ << rhs));
	}
	Pixel& operator<<=(uint8_t rhs)
	{
		R_ <<= rhs;
		G_ <<= rhs;
		B_ <<= rhs;
		return *this;
	}
	Pixel operator>>(uint8_t rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ >> rhs),
			static_cast<value_type>(G_ >> rhs),
			static_cast<value_type>(B_ >> rhs));
	}
	Pixel& operator>>=(uint8_t rhs)
	{
		R_ >>= rhs;
		G_ >>= rhs;
		B_ >>= rhs;
		return *this;
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
