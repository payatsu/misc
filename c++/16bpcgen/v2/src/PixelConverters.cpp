#include <stdexcept>
#include "Image.hpp"
#include "PixelConverters.hpp"

Image::pixel_type& Channel::convert(Image::pixel_type& pixel)const
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

Image::pixel_type& GrayScale::convert(Image::pixel_type& pixel)const
{
	const int coefficient = 1024;
	const Image::pixel_type::value_type Y =
		static_cast<Image::pixel_type::value_type>((
			0.2126*coefficient*pixel.R() +
			0.7152*coefficient*pixel.G() +
			0.0722*coefficient*pixel.B())/coefficient);
	return pixel = Image::pixel_type(Y, Y, Y);
}

Image::pixel_type& Threshold::convert(Image::pixel_type& pixel)const
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

Image::pixel_type& Offset::convert(Image::pixel_type& pixel)const
{

	if(ch() & R){
		pixel.R(invert_ ? std::max(pixel.R() - offset_, 0)
				: std::min(pixel.R() + offset_, static_cast<int>(Image::pixel_type::max)));
	}
	if(ch() & G){
		pixel.G(invert_ ? std::max(pixel.G() - offset_, 0)
				: std::min(pixel.G() + offset_, static_cast<int>(Image::pixel_type::max)));
	}
	if(ch() & B){
		pixel.B(invert_ ? std::max(pixel.B() - offset_, 0)
				: std::min(pixel.B() + offset_, static_cast<int>(Image::pixel_type::max)));
	}
	return pixel;
}

Image::pixel_type& Reversal::convert(Image::pixel_type& pixel)const
{
	if(ch() & R){
		pixel.R(Image::pixel_type::max - pixel.R());
	}
	if(ch() & G){
		pixel.G(Image::pixel_type::max - pixel.G());
	}
	if(ch() & B){
		pixel.B(Image::pixel_type::max - pixel.B());
	}
	return pixel;
}

Gamma::Gamma(const std::vector<Image::pixel_type::value_type>& lut, Ch c):
	Channel(c), lut_(lut)
{
	if(lut_.size() != static_cast<std::size_t>(Image::pixel_type::max + 1)){
		throw std::invalid_argument(__func__ + std::string(": too short lut"));
	}
}

Image::pixel_type& Gamma::convert(Image::pixel_type& pixel)const
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
