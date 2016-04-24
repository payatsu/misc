#include <stdexcept>
#include "PixelConverters.hpp"

Pixel& Channel::convert(Pixel& pixel)const
{
	if(!(ch_ & R)){
		pixel.setR(0);
	}
	if(!(ch_ & G)){
		pixel.setG(0);
	}
	if(!(ch_ & B)){
		pixel.setB(0);
	}
	return pixel;
}

Pixel& GrayScale::convert(Pixel& pixel)const
{
	const int coefficient = 1024;
	const Pixel::value_type Y = (
		0.2126*coefficient*pixel.R() +
		0.7152*coefficient*pixel.G() +
		0.0722*coefficient*pixel.B())/coefficient;
	return pixel = Pixel(Y, Y, Y);
}

Pixel& Threshold::convert(Pixel& pixel)const
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
		throw std::runtime_error(__func__);
	}
}

Pixel& Offset::convert(Pixel& pixel)const
{

	if(ch() & R){
		pixel.setR(invert_ ? std::max(pixel.R() - offset_, 0)
				   : std::min(pixel.R() + offset_, static_cast<int>(Pixel::max)));
	}
	if(ch() & G){
		pixel.setG(invert_ ? std::max(pixel.G() - offset_, 0)
				   : std::min(pixel.G() + offset_, static_cast<int>(Pixel::max)));
	}
	if(ch() & B){
		pixel.setB(invert_ ? std::max(pixel.B() - offset_, 0)
				   : std::min(pixel.B() + offset_, static_cast<int>(Pixel::max)));
	}
	return pixel;
}

Pixel& Reversal::convert(Pixel& pixel)const
{
	if(ch() & R){
		pixel.setR(Pixel::max - pixel.R());
	}
	if(ch() & G){
		pixel.setG(Pixel::max - pixel.G());
	}
	if(ch() & B){
		pixel.setB(Pixel::max - pixel.B());
	}
	return pixel;
}
