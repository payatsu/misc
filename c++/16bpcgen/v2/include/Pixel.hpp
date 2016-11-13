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
		CS_YCBCR709,
		CS_HSV
	};
	explicit Pixel(value_type r_y = 0x0, value_type g_cb = 0x0, value_type b_cr = 0x0, ColorSpace cs = CS_RGB): R_(r_y), G_(g_cb), B_(b_cr)
	{
		switch(cs){
		case CS_RGB:
			break;
		case CS_YCBCR601:{
			if( r_y < 16.0*max/255.0 || 235.0*max/255.0 < r_y  ||
			   g_cb < 16.0*max/255.0 || 240.0*max/255.0 < g_cb ||
			   b_cr < 16.0*max/255.0 || 240.0*max/255.0 < b_cr){
				throw std::range_error(__func__ + std::string(": range violation"));
			}
			const double  Ytmp = (r_y  -  16.0*max/255.0)*255.0/219.0;
			const double Cbtmp = (g_cb - 128.0*max/255.0)*255.0/224.0;
			const double Crtmp = (b_cr - 128.0*max/255.0)*255.0/224.0;
			R_ = std::min(std::max(Ytmp               + 1.402*Crtmp, 0.0), static_cast<double>(max));
			G_ = std::min(std::max(Ytmp - 0.344*Cbtmp - 0.714*Crtmp, 0.0), static_cast<double>(max));
			B_ = std::min(std::max(Ytmp + 1.772*Cbtmp,               0.0), static_cast<double>(max));
			break;
		}
		case CS_YCBCR709:{
			if( r_y < 16.0*max/255.0 || 235.0*max/255.0 < r_y  ||
			   g_cb < 16.0*max/255.0 || 240.0*max/255.0 < g_cb ||
			   b_cr < 16.0*max/255.0 || 240.0*max/255.0 < b_cr){
				throw std::range_error(__func__ + std::string(": range violation"));
			}
			const double  Ytmp = (r_y  -  16.0*max/255.0)*255.0/219.0;
			const double Cbtmp = (g_cb - 128.0*max/255.0)*255.0/224.0;
			const double Crtmp = (b_cr - 128.0*max/255.0)*255.0/224.0;
			R_ = std::min(std::max(Ytmp                + 1.5748*Crtmp, 0.0), static_cast<double>(max));
			G_ = std::min(std::max(Ytmp - 0.1873*Cbtmp - 0.4681*Crtmp, 0.0), static_cast<double>(max));
			B_ = std::min(std::max(Ytmp + 1.8556*Cbtmp,                0.0), static_cast<double>(max));
			break;
		}
		case CS_HSV:{
			const value_type maximum = b_cr;
			const value_type minimum = maximum - ( g_cb / max * maximum);
			switch(static_cast<int>(r_y / 60.0)){
			case 0:
				R_ = maximum;
				G_ = (r_y / 60.0) * (maximum - minimum) + minimum;
				B_ = minimum;
				break;
			case 1:
				R_ = ((120.0 - r_y) / 60.0) * (maximum - minimum) + minimum;
				G_ = maximum;
				B_ = minimum;
				break;
			case 2:
				R_ = minimum;
				G_ = maximum;
				B_ = ((r_y - 120.0) / 60.0) * (maximum - minimum) + minimum;
				break;
			case 3:
				R_ = minimum;
				G_ = ((240.0 - r_y) / 60.0) * (maximum - minimum) + minimum;
				B_ = maximum;
				break;
			case 4:
				R_ = ((r_y - 240.0) / 60.0) * (maximum - minimum) + minimum;
				G_ = minimum;
				B_ = maximum;
				break;
			case 5:
			case 6:
				R_ = maximum;
				G_ = minimum;
				B_ = ((360.0 - r_y) / 60.0) * (maximum - minimum) + minimum;
				break;
			default:
				throw std::range_error(__func__ + std::string(": range violation"));
				break;
			}
			break;
		}
		default:
			throw std::runtime_error(__func__ + std::string(": unknown color space"));
		}
	}
	template <typename U>
	Pixel(const Pixel<U>& rhs):
		R_(static_cast<value_type>(static_cast<double>(rhs.R())*max/Pixel<U>::max)),
		G_(static_cast<value_type>(static_cast<double>(rhs.G())*max/Pixel<U>::max)),
		B_(static_cast<value_type>(static_cast<double>(rhs.B())*max/Pixel<U>::max)){}
	template <typename U>
	Pixel& operator=(const Pixel<U>& rhs)
	{
		if(this == reinterpret_cast<const Pixel*>(&rhs)){
			return *this;
		}
		R_ = static_cast<value_type>(static_cast<double>(rhs.R())*max/Pixel<U>::max);
		G_ = static_cast<value_type>(static_cast<double>(rhs.G())*max/Pixel<U>::max);
		B_ = static_cast<value_type>(static_cast<double>(rhs.B())*max/Pixel<U>::max);
		return *this;
	}
	template <typename U>
	Pixel operator+(const Pixel<U>& rhs)const
	{
		Pixel tmp(rhs);
		return Pixel(R_ + tmp.R(), G_ + tmp.G(), B_ + tmp.B());
	}
	template <typename U>
	Pixel operator+=(const Pixel<U>& rhs)
	{
		Pixel tmp(rhs);
		R_ += tmp.R();
		G_ += tmp.G();
		B_ += tmp.B();
		return *this;
	}
	template <typename U>
	Pixel operator-(const Pixel<U>& rhs)const
	{
		Pixel tmp(rhs);
		return Pixel(R_ - tmp.R(), G_ - tmp.G(), B_ - tmp.B());
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
	Pixel operator<<(byte_t rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ << rhs),
			static_cast<value_type>(G_ << rhs),
			static_cast<value_type>(B_ << rhs));
	}
	Pixel& operator<<=(byte_t rhs)
	{
		R_ <<= rhs;
		G_ <<= rhs;
		B_ <<= rhs;
		return *this;
	}
	Pixel operator>>(byte_t rhs)const
	{
		return Pixel(
			static_cast<value_type>(R_ >> rhs),
			static_cast<value_type>(G_ >> rhs),
			static_cast<value_type>(B_ >> rhs));
	}
	Pixel& operator>>=(byte_t rhs)
	{
		R_ >>= rhs;
		G_ >>= rhs;
		B_ >>= rhs;
		return *this;
	}
	std::ostream& print(std::ostream& os)const
	{
		const int decw = 5;
		const int hexw = sizeof(value_type);
		os << "R=" << std::setw(decw) << R_ << "(0x" << std::hex << std::setw(hexw) << std::setfill('0') << R_ << std::resetiosflags(std::ios::hex) << std::setfill(' ') << "), "
		   << "G=" << std::setw(decw) << G_ << "(0x" << std::hex << std::setw(hexw) << std::setfill('0') << G_ << std::resetiosflags(std::ios::hex) << std::setfill(' ') << "), "
		   << "B=" << std::setw(decw) << B_ << "(0x" << std::hex << std::setw(hexw) << std::setfill('0') << B_ << std::resetiosflags(std::ios::hex) << std::setfill(' ') << ")";
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
			throw std::runtime_error(__func__ + std::string(": hue undefined"));
		}else if(minimum == B_){
			return 60.0 * (static_cast<double>(G_) - R_) / max / diff +  60.0;
		}else if(minimum == R_){
			return 60.0 * (static_cast<double>(B_) - G_) / max / diff + 180.0;
		}else{
			return 60.0 * (static_cast<double>(R_) - B_) / max / diff + 300.0;
		}
	}
	double S()const
	{
		return static_cast<double>(std::max(std::max(R_, G_), B_) - std::min(std::min(R_, G_), B_))/max;
	}
	double V()const{return static_cast<double>(std::max(std::max(R_, G_), B_))/max;}
	value_type  Y601()const{return ( 0.2990*R_ + 0.5870*G_ + 0.1140*B_)*219.0/255.0 +  16.0*max/255.0;}
	value_type Cb601()const{return (-0.1687*R_ - 0.3312*G_ + 0.5000*B_)*224.0/255.0 + 128.0*max/255.0;}
	value_type Cr601()const{return ( 0.5000*R_ - 0.4186*G_ - 0.0813*B_)*224.0/255.0 + 128.0*max/255.0;}
	value_type  Y709()const{return ( 0.2126*R_ + 0.7152*G_ + 0.0722*B_)*219.0/255.0 +  16.0*max/255.0;}
	value_type Cb709()const{return (-0.1146*R_ - 0.3854*G_ + 0.5000*B_)*224.0/255.0 + 128.0*max/255.0;}
	value_type Cr709()const{return ( 0.5000*R_ - 0.4542*G_ - 0.0458*B_)*224.0/255.0 + 128.0*max/255.0;}
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
