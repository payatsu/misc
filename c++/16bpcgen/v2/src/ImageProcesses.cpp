#include <stdexcept>
#include "FrameBuffer.hpp"
#include "ImageProcesses.hpp"
#include "Pixel.hpp"
#include "PixelConverter.hpp"

bool AreaSpecifier::is_in(const FrameBuffer& buffer)const
{
	return area_.offset_x_ < buffer.width()  &&
		   area_.offset_y_ < buffer.height() &&
		   area_.offset_x_ + area_.width_  <= buffer.width() &&
		   area_.offset_y_ + area_.height_ <= buffer.height();
}

FrameBuffer& Tone::process(FrameBuffer& buffer)const
{
	if(!is_in(buffer)){
		throw std::runtime_error(__func__);
	}

	const uint32_t limit_w =
		area_.width_  == 0 && area_.offset_x_ == 0
						   ? buffer.width()  : area_.offset_x_ + area_.width_;
	const uint32_t limit_h =
		area_.height_ == 0 && area_.offset_y_ == 0
						   ? buffer.height() : area_.offset_y_ + area_.height_;

	for(uint32_t h = area_.offset_y_; h < limit_h; ++h){
		for(uint32_t w = area_.offset_x_; w < limit_w; ++w){
			converter_.convert(buffer[h][w]);
		}
	}
	return buffer;
}

FrameBuffer& Crop::process(FrameBuffer& buffer)const
{
	if(!is_in(buffer)){
		throw std::runtime_error(__func__);
	}

	const uint32_t limit_w = area_.offset_x_ + area_.width_;
	const uint32_t limit_h = area_.offset_y_ + area_.height_;
	FrameBuffer tmp = FrameBuffer(area_.width_, area_.height_);
	for(uint32_t h = area_.offset_y_, i = 0; h < limit_h; ++h, ++i){
		for(uint32_t w = area_.offset_x_, j = 0; w < limit_w; ++w, ++j){
			tmp[i][j] = buffer[h][w];
		}
	}
	return buffer = tmp;
}
