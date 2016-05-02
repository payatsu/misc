#include <algorithm>
#include <stdexcept>
#include "FrameBuffer.hpp"
#include "ImageProcesses.hpp"
#include "PixelConverter.hpp"

bool AreaSpecifier::within(const FrameBuffer& buffer)const
{
	return area_.offset_x_ < buffer.width()  &&
		area_.offset_y_ < buffer.height() &&
		area_.offset_x_ + area_.width_  <= buffer.width() &&
		area_.offset_y_ + area_.height_ <= buffer.height();
}

FrameBuffer& Tone::process(FrameBuffer& buffer)const
{
	if(!within(buffer)){
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

FrameBuffer& Normalize::process(FrameBuffer& buffer)const
{
	const Pixel<uint16_t>::value_type max = *std::max_element(
		reinterpret_cast<Pixel<uint16_t>::value_type*>(&buffer[0][0]),
		reinterpret_cast<Pixel<uint16_t>::value_type*>(&buffer[buffer.height()][0]));

	if(!within(buffer)){
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
			const double r = buffer[h][w].R() / (double)max * Pixel<uint16_t>::max;
			const double g = buffer[h][w].G() / (double)max * Pixel<uint16_t>::max;
			const double b = buffer[h][w].B() / (double)max * Pixel<uint16_t>::max;
			buffer[h][w] = Pixel<uint16_t>(
				static_cast<Pixel<uint16_t>::value_type>(r),
				static_cast<Pixel<uint16_t>::value_type>(g),
				static_cast<Pixel<uint16_t>::value_type>(b)
				);
		}
	}
	return buffer;
}

FrameBuffer& Crop::process(FrameBuffer& buffer)const
{
	if(!within(buffer)){
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

FrameBuffer& Filter::process(FrameBuffer& buffer)const
{
	if(!(kernel_.size() % 2) || kernel_.size() < 2){
		throw std::runtime_error(__func__);
	}
	const std::size_t kernel_width = kernel_[0].size();
	for(std::size_t i = 0; i < kernel_.size(); ++i){
		if(!(kernel_[i].size() % 2) || kernel_[i].size() < 2 || kernel_[i].size() != kernel_width){
			throw std::runtime_error(__func__);
		}
	}

	FrameBuffer tmp = FrameBuffer(buffer.width(), buffer.height());
	for(uint32_t h = 0; h < buffer.height(); ++h){
		const uint32_t h_lowerbound =
			h - kernel_.size()/2 < buffer.height() ? h - kernel_.size()/2 : 0 ;
		const uint32_t h_upperbound = std::min(
			static_cast<uint32_t>(h + kernel_.size()/2 + 1), buffer.height());
		for(uint32_t w = 0; w < buffer.width(); ++w){
			const uint32_t w_lowerbound =
				w - kernel_[0].size()/2 < buffer.width() ? w - kernel_[0].size()/2 : 0 ;
			const uint32_t w_upperbound = std::min(
				static_cast<uint32_t>(w + kernel_[0].size()/2 + 1), buffer.width());
			Pixel<uint16_t> pixel = black;
			for(uint32_t hh = h_lowerbound, i = 0; hh < h_upperbound; ++hh, ++i){
				for(uint32_t ww = w_lowerbound, j = 0; ww < w_upperbound; ++ww, ++j){
					pixel = pixel + buffer[hh][ww] * kernel_[i][j];
				}
			}
			tmp[h][w] = pixel;
		}
	}
	return buffer = tmp;
}
