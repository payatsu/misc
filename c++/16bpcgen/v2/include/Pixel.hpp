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
	enum ColorSpace{
		CS_RGB,
		CS_YCBCR601,
		CS_YCBCR709
	};
	Pixel(value_type r_y = 0x0, value_type g_cb = 0x0, value_type b_cr = 0x0, ColorSpace cs = CS_RGB): R_(r_y), G_(g_cb), B_(b_cr)
	{
		switch(cs){
		case CS_RGB:
			break;
		case CS_YCBCR601:
			R_ = 1.164*(r_y - 16.0*max/255)                                + 1.596*(b_cr - 128.0*max/255);
			G_ = 1.164*(r_y - 16.0*max/255) - 0.391*(g_cb - 128.0*max/255) - 0.813*(b_cr - 128.0*max/255);
			B_ = 1.164*(r_y - 16.0*max/255) + 2.018*(g_cb - 128.0*max/255);
			break;
		case CS_YCBCR709:
			R_ = 1.164*(r_y - 16.0*max/255)                                + 1.793*(b_cr - 128.0*max/255);
			G_ = 1.164*(r_y - 16.0*max/255) - 0.213*(g_cb - 128.0*max/255) - 0.533*(b_cr - 128.0*max/255);
			B_ = 1.164*(r_y - 16.0*max/255) + 2.112*(g_cb - 128.0*max/255);
			break;
		default:
			throw std::runtime_error(__func__);
		}
	}
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
	double H()const
	{
		const value_type maximum = std::max(std::max(R_, G_), B_);
		const value_type minimum = std::min(std::min(R_, G_), B_);
		const double diff = maximum - minimum;
		if(minimum == maximum){
			throw std::runtime_error(__func__ + std::string(": Hue undefined."));
		}else if(minimum == B_){
			return 60 * (static_cast<double>(G_) - R_) / max / diff + 60;
		}else if(minimum == R_){
			return 60 * (static_cast<double>(B_) - G_) / max / diff + 180;
		}else{
			return 60 * (static_cast<double>(R_) - B_) / max / diff + 300;
		}
	}
	double S()const
	{
		return static_cast<double>(std::max(std::max(R_, G_), B_) - std::min(std::min(R_, G_), B_))/max;
	}
	double V()const{return static_cast<double>(std::max(std::max(R_, G_), B_))/max;}
	value_type  Y601()const{return  0.257*R_ + 0.504*G_ + 0.098*B_ +  16.0*max/255;}
	value_type Cb601()const{return -0.148*R_ - 0.291*G_ + 0.439*B_ + 128.0*max/255;}
	value_type Cr601()const{return  0.439*R_ - 0.368*G_ - 0.071*B_ + 128.0*max/255;}
	value_type  Y709()const{return  0.183*R_ + 0.614*G_ + 0.062*B_ +  16.0*max/255;}
	value_type Cb709()const{return -0.101*R_ - 0.339*G_ + 0.439*B_ + 128.0*max/255;}
	value_type Cr709()const{return  0.439*R_ - 0.399*G_ - 0.040*B_ + 128.0*max/255;}
private:
	value_type R_;
	value_type G_;
	value_type B_;
};
template <typename T>
std::ostream& operator<<(std::ostream& os, const Pixel<T>& p)
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
