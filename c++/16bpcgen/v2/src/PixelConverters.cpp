#include <stdexcept>
#include "Image.hpp"
#include "PixelConverters.hpp"

Pixel<uint16_t>& Channel::convert(Pixel<uint16_t>& pixel)const
{
	if(!(ch_ & R)){
		pixel.R(0);
	}
	if(!(ch_ & G)){
		pixel.G(0);
	}
	if(!(ch_ & B)){
		pixel.B(0);
	}
	return pixel;
}

Pixel<uint16_t>& GrayScale::convert(Pixel<uint16_t>& pixel)const
{
	const int coefficient = 1024;
	const Pixel<uint16_t>::value_type Y =
		static_cast<Pixel<uint16_t>::value_type>((
			0.2126*coefficient*pixel.R() +
			0.7152*coefficient*pixel.G() +
			0.0722*coefficient*pixel.B())/coefficient);
	return pixel = Pixel<uint16_t>(Y, Y, Y);
}

Pixel<uint16_t>& Threshold::convert(Pixel<uint16_t>& pixel)const
{
	switch(ch()){
	case R:
		return pixel = pixel.R() < threshold_ ? black : white;
		break;
	case G:
		return pixel = pixel.G() < threshold_ ? black : white;
		break;
	case B:
		return pixel = pixel.B() < threshold_ ? black : white;
		break;
	default:
		throw std::invalid_argument(__func__ + std::string(": invalid channel"));
	}
}

Pixel<uint16_t>& Offset::convert(Pixel<uint16_t>& pixel)const
{

	if(ch() & R){
		pixel.R(invert_ ? std::max(pixel.R() - offset_, 0)
				: std::min(pixel.R() + offset_, static_cast<int>(Pixel<uint16_t>::max)));
	}
	if(ch() & G){
		pixel.G(invert_ ? std::max(pixel.G() - offset_, 0)
				: std::min(pixel.G() + offset_, static_cast<int>(Pixel<uint16_t>::max)));
	}
	if(ch() & B){
		pixel.B(invert_ ? std::max(pixel.B() - offset_, 0)
				: std::min(pixel.B() + offset_, static_cast<int>(Pixel<uint16_t>::max)));
	}
	return pixel;
}

Pixel<uint16_t>& Reversal::convert(Pixel<uint16_t>& pixel)const
{
	if(ch() & R){
		pixel.R(Pixel<uint16_t>::max - pixel.R());
	}
	if(ch() & G){
		pixel.G(Pixel<uint16_t>::max - pixel.G());
	}
	if(ch() & B){
		pixel.B(Pixel<uint16_t>::max - pixel.B());
	}
	return pixel;
}

Gamma::Gamma(const std::vector<Pixel<uint16_t>::value_type>& lut, Ch ch):
	Channel(ch), lut_(lut)
{
	if(lut_.size() != static_cast<std::size_t>(Pixel<uint16_t>::max + 1)){
		throw std::invalid_argument(__func__ + std::string(": too short lut"));
	}
}

Pixel<uint16_t>& Gamma::convert(Pixel<uint16_t>& pixel)const
{
	if(ch() & R){
		pixel.R(lut_[pixel.R()]);
	}
	if(ch() & G){
		pixel.G(lut_[pixel.G()]);
	}
	if(ch() & B){
		pixel.G(lut_[pixel.B()]);
	}
	return pixel;
}
