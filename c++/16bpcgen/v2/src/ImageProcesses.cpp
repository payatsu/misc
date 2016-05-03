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
	if(!within(buffer)){
		throw std::runtime_error(__func__);
	}

	const uint32_t limit_w =
		area_.width_  == 0 && area_.offset_x_ == 0
						? buffer.width()  : area_.offset_x_ + area_.width_;
	const uint32_t limit_h =
		area_.height_ == 0 && area_.offset_y_ == 0
						? buffer.height() : area_.offset_y_ + area_.height_;

	const Pixel<uint16_t>::value_type max = *std::max_element(
		reinterpret_cast<Pixel<uint16_t>::value_type*>(&buffer[0][0]),
		reinterpret_cast<Pixel<uint16_t>::value_type*>(&buffer[buffer.height()][0]));

	for(uint32_t h = area_.offset_y_; h < limit_h; ++h){
		for(uint32_t w = area_.offset_x_; w < limit_w; ++w){
			buffer[h][w] = Pixel<double>(buffer[h][w]) / (double)max * Pixel<uint16_t>::max;
		}
	}
	return buffer;
}

FrameBuffer& Median::process(FrameBuffer& buffer)const
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

	FrameBuffer result = buffer;

	for(uint32_t h = area_.offset_y_; h < limit_h; ++h){
		const uint32_t h_lowerbound = h - 1 < buffer.height() ? h - 1 : 0 ;
		const uint32_t h_upperbound = std::min(h + 1, buffer.height());
		for(uint32_t w = area_.offset_x_; w < limit_w; ++w){
			const uint32_t w_lowerbound = w - 1 < buffer.width() ? w - 1 : 0 ;
			const uint32_t w_upperbound = std::min(w + 1, buffer.width());
			std::vector<Pixel<uint16_t>::value_type> values1;
			std::vector<Pixel<uint16_t>::value_type> values2;
			std::vector<Pixel<uint16_t>::value_type> values3;
			for(uint32_t i = h_lowerbound; i < h_upperbound; ++i){
				for(uint32_t j = w_lowerbound; j < w_upperbound; ++j){
					values1.push_back(buffer[i][j].R());
					values2.push_back(buffer[i][j].G());
					values3.push_back(buffer[i][j].B());
				}
			}
			std::sort(values1.begin(), values1.end());
			std::sort(values2.begin(), values2.end());
			std::sort(values3.begin(), values3.end());
			result[h][w] = Pixel<uint16_t>(
					values1[values1.size()/2],
					values2[values2.size()/2],
					values3[values3.size()/2]);
		}
	}
	return buffer = result;
}

FrameBuffer& Crop::process(FrameBuffer& buffer)const
{
	if(!within(buffer)){
		throw std::runtime_error(__func__);
	}

	const uint32_t limit_w = area_.offset_x_ + area_.width_;
	const uint32_t limit_h = area_.offset_y_ + area_.height_;
	FrameBuffer result = FrameBuffer(area_.width_, area_.height_);
	for(uint32_t h = area_.offset_y_, i = 0; h < limit_h; ++h, ++i){
		for(uint32_t w = area_.offset_x_, j = 0; w < limit_w; ++w, ++j){
			result[i][j] = buffer[h][w];
		}
	}
	return buffer = result;
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

	FrameBuffer result = FrameBuffer(buffer.width(), buffer.height());
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
			Pixel<double> pixel = black;
			for(uint32_t hh = h_lowerbound, i = 0; hh < h_upperbound; ++hh, ++i){
				for(uint32_t ww = w_lowerbound, j = 0; ww < w_upperbound; ++ww, ++j){
					pixel = pixel + Pixel<double>(buffer[hh][ww]) * kernel_[i][j];
				}
			}
			result[h][w] = pixel;
		}
	}
	return buffer = result;
}

Filter::Kernel WeightedSmoothing::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(0/ 13.0);
	kernel[0].push_back(0/ 13.0);
	kernel[0].push_back(1/ 13.0);
	kernel[0].push_back(0/ 13.0);
	kernel[0].push_back(0/ 13.0);
	kernel.push_back(KernelRow());
	kernel[1].push_back(0/ 13.0);
	kernel[1].push_back(1/ 13.0);
	kernel[1].push_back(1/ 13.0);
	kernel[1].push_back(1/ 13.0);
	kernel[1].push_back(0/ 13.0);
	kernel.push_back(KernelRow());
	kernel[2].push_back(1/ 13.0);
	kernel[2].push_back(1/ 13.0);
	kernel[2].push_back(1/ 13.0);
	kernel[2].push_back(1/ 13.0);
	kernel[2].push_back(1/ 13.0);
	kernel.push_back(KernelRow());
	kernel[3].push_back(0/ 13.0);
	kernel[3].push_back(1/ 13.0);
	kernel[3].push_back(1/ 13.0);
	kernel[3].push_back(1/ 13.0);
	kernel[3].push_back(0/ 13.0);
	kernel.push_back(KernelRow());
	kernel[4].push_back(0/ 13.0);
	kernel[4].push_back(0/ 13.0);
	kernel[4].push_back(1/ 13.0);
	kernel[4].push_back(0/ 13.0);
	kernel[4].push_back(0/ 13.0);
	return kernel;
}

Filter::Kernel UnSharpMask::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back( 0);
	kernel[0].push_back(-1);
	kernel[0].push_back( 0);
	kernel.push_back(KernelRow());
	kernel[1].push_back(-1);
	kernel[1].push_back( 5);
	kernel[1].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[2].push_back( 0);
	kernel[2].push_back(-1);
	kernel[2].push_back( 0);
	return kernel;
}

Filter::Kernel Prewitt::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(-1);
	kernel[0].push_back(-1);
	kernel[0].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[1].push_back( 0);
	kernel[1].push_back( 0);
	kernel[1].push_back( 0);
	kernel.push_back(KernelRow());
	kernel[2].push_back( 1);
	kernel[2].push_back( 1);
	kernel[2].push_back( 1);
	return kernel;
}

Filter::Kernel Sobel::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(-1);
	kernel[0].push_back(-2);
	kernel[0].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[1].push_back( 0);
	kernel[1].push_back( 0);
	kernel[1].push_back( 0);
	kernel.push_back(KernelRow());
	kernel[2].push_back( 1);
	kernel[2].push_back( 2);
	kernel[2].push_back( 1);
	return kernel;
}

Filter::Kernel Laplacian3x3::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(-1);
	kernel[0].push_back(-1);
	kernel[0].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[1].push_back(-1);
	kernel[1].push_back( 8);
	kernel[1].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[2].push_back(-1);
	kernel[2].push_back(-1);
	kernel[2].push_back(-1);
	return kernel;
}

Filter::Kernel Laplacian5x5::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(-1);
	kernel[0].push_back(-3);
	kernel[0].push_back(-4);
	kernel[0].push_back(-3);
	kernel[0].push_back(-1);
	kernel.push_back(KernelRow());
	kernel[1].push_back(-3);
	kernel[1].push_back( 0);
	kernel[1].push_back( 6);
	kernel[1].push_back( 0);
	kernel[1].push_back(-3);
	kernel.push_back(KernelRow());
	kernel[2].push_back(-4);
	kernel[2].push_back( 6);
	kernel[2].push_back(20);
	kernel[2].push_back( 6);
	kernel[2].push_back(-4);
	kernel.push_back(KernelRow());
	kernel[3].push_back(-3);
	kernel[3].push_back( 0);
	kernel[3].push_back( 6);
	kernel[3].push_back( 0);
	kernel[3].push_back(-3);
	kernel.push_back(KernelRow());
	kernel[4].push_back(-1);
	kernel[4].push_back(-3);
	kernel[4].push_back(-4);
	kernel[4].push_back(-3);
	kernel[4].push_back(-1);
	return kernel;
}
