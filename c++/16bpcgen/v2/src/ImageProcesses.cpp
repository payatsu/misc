#include <algorithm>
#include <stdexcept>
#include "Image.hpp"
#include "ImageProcesses.hpp"
#include "PatternGenerators.hpp"
#include "PixelConverter.hpp"

bool AreaSpecifier::within(const Image& image)const
{
	return area_.offset_x_ < image.width()  &&
		area_.offset_y_ < image.height() &&
		area_.offset_x_ + area_.width_  <= image.width() &&
		area_.offset_y_ + area_.height_ <= image.height();
}

Image& Tone::process(Image& image)const
{
	if(!within(image)){
		throw std::range_error(__func__ + std::string(": invalid area specification"));
	}

	const column_t limit_w =
		area_.width_  == 0 && area_.offset_x_ == 0
						? image.width()  : area_.offset_x_ + area_.width_;
	const row_t    limit_h =
		area_.height_ == 0 && area_.offset_y_ == 0
						? image.height() : area_.offset_y_ + area_.height_;

	for(row_t h = area_.offset_y_; h < limit_h; ++h){
		for(column_t w = area_.offset_x_; w < limit_w; ++w){
			converter_.convert(image[h][w]);
		}
	}
	return image;
}

Image& Normalize::process(Image& image)const
{
	if(!within(image)){
		throw std::range_error(__func__ + std::string(": invalid area specification"));
	}

	const column_t limit_w =
		area_.width_  == 0 && area_.offset_x_ == 0
						? image.width()  : area_.offset_x_ + area_.width_;
	const row_t    limit_h =
		area_.height_ == 0 && area_.offset_y_ == 0
						? image.height() : area_.offset_y_ + area_.height_;

	const Pixel<>::value_type max = *std::max_element(
		reinterpret_cast<Pixel<>::value_type*>(&image[0][0]),
		reinterpret_cast<Pixel<>::value_type*>(&image[image.height()][0]));

	for(row_t h = area_.offset_y_; h < limit_h; ++h){
		for(column_t w = area_.offset_x_; w < limit_w; ++w){
			image[h][w] = Pixel<double>(image[h][w]) / (double)max * Pixel<>::max;
		}
	}
	return image;
}

Image& Median::process(Image& image)const
{
	if(!within(image)){
		throw std::range_error(__func__ + std::string(": invalid area specification"));
	}

	const column_t limit_w =
		area_.width_  == 0 && area_.offset_x_ == 0
						? image.width()  : area_.offset_x_ + area_.width_;
	const row_t limit_h =
		area_.height_ == 0 && area_.offset_y_ == 0
						? image.height() : area_.offset_y_ + area_.height_;

	Image result = Image(image.width(), image.height());

	for(row_t h = area_.offset_y_; h < limit_h; ++h){
		const row_t h_lowerbound = h - 1 < image.height() ? h - 1 : 0 ;
		const row_t h_upperbound = std::min(h + 1, image.height());
		for(column_t w = area_.offset_x_; w < limit_w; ++w){
			const column_t w_lowerbound = w - 1 < image.width() ? w - 1 : 0 ;
			const column_t w_upperbound = std::min(w + 1, image.width());
			std::vector<Pixel<>::value_type> values1;
			std::vector<Pixel<>::value_type> values2;
			std::vector<Pixel<>::value_type> values3;
			for(row_t i = h_lowerbound; i < h_upperbound; ++i){
				for(column_t j = w_lowerbound; j < w_upperbound; ++j){
					values1.push_back(image[i][j].R());
					values2.push_back(image[i][j].G());
					values3.push_back(image[i][j].B());
				}
			}
			std::sort(values1.begin(), values1.end());
			std::sort(values2.begin(), values2.end());
			std::sort(values3.begin(), values3.end());
			result[h][w] = Pixel<>(
					values1[values1.size()/2],
					values2[values2.size()/2],
					values3[values3.size()/2]);
		}
	}
	return image.swap(result);
}

Image& Crop::process(Image& image)const
{
	if(!within(image)){
		throw std::range_error(__func__ + std::string(": invalid area specification"));
	}

	const column_t limit_w = area_.offset_x_ + area_.width_;
	const row_t limit_h = area_.offset_y_ + area_.height_;
	Image result = Image(area_.width_, area_.height_);
	for(row_t h = area_.offset_y_, i = 0; h < limit_h; ++h, ++i){
		for(column_t w = area_.offset_x_, j = 0; w < limit_w; ++w, ++j){
			result[i][j] = image[h][w];
		}
	}
	return image.swap(result);
}

Image& Filter::process(Image& image)const
{
	if(!(kernel_.size() % 2) || kernel_.size() < 2){
		throw std::runtime_error(__func__ + std::string(": filter kernel height must be odd more than 1"));
	}
	const std::size_t kernel_width = kernel_[0].size();
	for(std::size_t i = 0; i < kernel_.size(); ++i){
		if(!(kernel_[i].size() % 2) || kernel_[i].size() < 2 || kernel_[i].size() != kernel_width){
			throw std::runtime_error(__func__ + std::string(": fileter kernel width must be odd more than 1"));
		}
	}

	Image result = Image(image.width(), image.height());
	for(row_t h = 0; h < image.height(); ++h){
		const row_t h_lowerbound =
			h - kernel_.size()/2 < image.height() ? h - kernel_.size()/2 : 0 ;
		const row_t h_upperbound = std::min(
			static_cast<row_t>(h + kernel_.size()/2 + 1), image.height());
		for(column_t w = 0; w < image.width(); ++w){
			const column_t w_lowerbound =
				w - kernel_[0].size()/2 < image.width() ? w - kernel_[0].size()/2 : 0 ;
			const column_t w_upperbound = std::min(
				static_cast<column_t>(w + kernel_[0].size()/2 + 1), image.width());
			Pixel<double> pixel = black;
			for(row_t hh = h_lowerbound, i = 0; hh < h_upperbound; ++hh, ++i){
				for(column_t ww = w_lowerbound, j = 0; ww < w_upperbound; ++ww, ++j){
					pixel = pixel + Pixel<double>(image[hh][ww]) * kernel_[i][j];
				}
			}
			result[h][w] = pixel;
		}
	}
	return image.swap(result);
}

Filter::Kernel WeightedSmoothing::init()
{
	Kernel kernel;
	kernel.push_back(KernelRow());
	kernel[0].push_back(0/13.0);
	kernel[0].push_back(0/13.0);
	kernel[0].push_back(1/13.0);
	kernel[0].push_back(0/13.0);
	kernel[0].push_back(0/13.0);
	kernel.push_back(KernelRow());
	kernel[1].push_back(0/13.0);
	kernel[1].push_back(1/13.0);
	kernel[1].push_back(1/13.0);
	kernel[1].push_back(1/13.0);
	kernel[1].push_back(0/13.0);
	kernel.push_back(KernelRow());
	kernel[2].push_back(1/13.0);
	kernel[2].push_back(1/13.0);
	kernel[2].push_back(1/13.0);
	kernel[2].push_back(1/13.0);
	kernel[2].push_back(1/13.0);
	kernel.push_back(KernelRow());
	kernel[3].push_back(0/13.0);
	kernel[3].push_back(1/13.0);
	kernel[3].push_back(1/13.0);
	kernel[3].push_back(1/13.0);
	kernel[3].push_back(0/13.0);
	kernel.push_back(KernelRow());
	kernel[4].push_back(0/13.0);
	kernel[4].push_back(0/13.0);
	kernel[4].push_back(1/13.0);
	kernel[4].push_back(0/13.0);
	kernel[4].push_back(0/13.0);
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

/**
 * @TODO アルゴリズムがイケてないので改善する。
 */
Image& HScale::process(Image& image)const
{
	Image result(width_, image.height());
	for(row_t h = 0; h < image.height(); ++h){
		for(column_t w = 0; w < width_; ++w){
			result[h][w] = image[h][w * image.width() / width_];
		}
	}
	return image.swap(result);
}

/**
 * @TODO アルゴリズムがイケてないので改善する。
 */
Image& VScale::process(Image& image)const
{
	Image result(image.width(), height_);
	for(column_t w = 0; w < image.width(); ++w){
		for(row_t h = 0; h < height_; ++h){
			result[h][w] = image[h * image.height() / height_][w];
		}
	}
	return image.swap(result);
}

Image& KeyStone::process(Image& image)const
{
	Image phase1 = Image(image.width(), image.height());
	Image phase2 = Image(image.width(), image.height());
	phase1 >>= Luster(black);
	phase2 >>= Luster(black);
	switch(vertex_){
	case TOP_LEFT:
		for(row_t h = 0; h < image.height(); ++h){
			const column_t current_offset = width_offset_*(image.height() - h)/image.height();
			for(column_t w = 0; w < image.width() - current_offset; ++w){
				phase1[h][w + current_offset] = image[h][w*image.width()/(image.width() - current_offset)];
			}
		}
		for(column_t w = 0; w < image.width(); ++w){
			const row_t current_offset = height_offset_*(image.width() - w)/(image.width() - width_offset_);
			for(row_t h = 0; h < image.height() - current_offset; ++h){
				phase2[h + current_offset][w] = phase1[h*image.height()/(image.height() - current_offset)][w];
			}
		}
		break;
	case TOP_RIGHT:
		for(row_t h = 0; h < image.height(); ++h){
			const column_t current_offset = width_offset_*(image.height() - h)/image.height();
			for(column_t w = 0; w < image.width() - current_offset; ++w){
				phase1[h][w] = image[h][w*image.width()/(image.width() - current_offset)];
			}
		}
		for(column_t w = 0; w < image.width(); ++w){
			const row_t current_offset = height_offset_*w/(image.width() - width_offset_);
			for(row_t h = 0; h < image.height() - current_offset; ++h){
				phase2[h + current_offset][w] = phase1[h*image.height()/(image.height() - current_offset)][w];
			}
		}
		break;
	case BOTTOM_LEFT:
		for(row_t h = 0; h < image.height(); ++h){
			const column_t current_offset = width_offset_*h/image.height();
			for(column_t w = 0; w < image.width() - current_offset; ++w){
				phase1[h][w + current_offset] = image[h][w*image.width()/(image.width() - current_offset)];
			}
		}
		for(column_t w = 0; w < image.width(); ++w){
			const row_t current_offset = height_offset_*(image.width() - w)/(image.width() - width_offset_);
			for(row_t h = 0; h < image.height() - current_offset; ++h){
				phase2[h][w] = phase1[h*image.height()/(image.height() - current_offset)][w];
			}
		}
		break;
	case BOTTOM_RIGHT:
		for(row_t h = 0; h < image.height(); ++h){
			const column_t current_offset = width_offset_*h/image.height();
			for(column_t w = 0; w < image.width() - current_offset; ++w){
				phase1[h][w] = image[h][w*image.width()/(image.width() - current_offset)];
			}
		}
		for(column_t w = 0; w < image.width(); ++w){
			const row_t current_offset = height_offset_*w/(image.width() - width_offset_);
			for(row_t h = 0; h < image.height() - current_offset; ++h){
				phase2[h][w] = phase1[h*image.height()/(image.height() - current_offset)][w];
			}
		}
		break;
	default:
		break;
	}
	return image.swap(phase2);
}
