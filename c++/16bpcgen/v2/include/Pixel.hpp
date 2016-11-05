#ifndef _16BPCGEN_PIXEL_HPP_
#define _16BPCGEN_PIXEL_HPP_

#include <iomanip>
#include <ostream>
#include "typedef.hpp"

template <typename T = uint16_t>
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
	template <typename U>
	Pixel operator+(const Pixel<U>& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ + rhs.R()),
			static_cast<value_type>(G_ + rhs.G()),
			static_cast<value_type>(B_ + rhs.B()));
	}
	template <typename U>
	Pixel operator+=(const Pixel<U>& rhs)
	{
		R_ += rhs.R();
		G_ += rhs.G();
		B_ += rhs.B();
		return *this;
	}
	template <typename U>
	Pixel operator-(const Pixel<U>& rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ - rhs.R()),
			static_cast<value_type>(G_ - rhs.G()),
			static_cast<value_type>(B_ - rhs.B()));
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
	std::ostream& print(std::ostream& os)const
	{
		const int w = 4;
		os << std::hex;
		os << std::setw(w) << R_ << ','
		   << std::setw(w) << G_ << ','
		   << std::setw(w) << B_;
		os << resetiosflags(std::ios::hex);
		return os;
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
template <typename T = uint16_t>
std::ostream& operator<<(std::ostream& os, const Pixel<>& p)
{
	return p.print(os);
}

extern const Pixel<> black;
extern const Pixel<> white;
extern const Pixel<> red;
extern const Pixel<> green;
extern const Pixel<> blue;
extern const Pixel<> cyan;
extern const Pixel<> magenta;
extern const Pixel<> yellow;

#endif
