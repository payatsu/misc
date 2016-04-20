#include "FrameBuffer.hpp"

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const int pixelsize = 6;

const Pixel black  (0x0000, 0x0000, 0x0000);
const Pixel white  (0xffff, 0xffff, 0xffff);
const Pixel red    (0xffff, 0x0000, 0x0000);
const Pixel green  (0x0000, 0xffff, 0x0000);
const Pixel blue   (0x0000, 0x0000, 0xffff);
const Pixel cyan   (0x0000, 0xffff, 0xffff);
const Pixel magenta(0xffff, 0x0000, 0xffff);
const Pixel yellow (0xffff, 0xffff, 0x0000);

FrameBuffer::FrameBuffer(const FrameBuffer& buffer):
	head_(new uint8_t[buffer.height_*buffer.width_*pixelsize]), width_(buffer.width_), height_(buffer.height_)
{
	std::copy(buffer.head_, buffer.head_ + buffer.height_*buffer.width_*pixelsize, head_);
}

FrameBuffer& FrameBuffer::operator<<(std::istream& is)
{
	const int size = height()*width()*pixelsize;
	for(int i = 0; i < size; ++i){
		int c = is.get();
		if(is.eof()){
			break;
		}
		head_[i] = c;
	}
	return *this;
}
