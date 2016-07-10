#ifndef _16BPCGEN_IMAGE_HPP_
#define _16BPCGEN_IMAGE_HPP_

#include <iosfwd>
#include "typedef.hpp"
class ImageProcess;
class PatternGenerator;
template <typename T> class Pixel;
class PixelConverter;

extern const int bitdepth;
#ifdef ENABLE_PNG
extern const int colortype;
#endif
extern const unsigned int pixelsize;

class Row{
public:
	Row(uint8_t* row, const uint32_t& width): row_(row), width_(width){}
	const uint32_t& width()const{return width_;}
	Pixel<uint16_t>& operator[](unsigned int column)const{return *reinterpret_cast<Pixel<uint16_t>*>(const_cast<uint8_t*>(row_) + column*pixelsize);}
	Row& operator++(){row_ += width()*pixelsize; return *this;}
	bool operator!=(const Row& rhs)const{return this->row_ != rhs.row_;}
	static void fill(Row first, Row last, const Row& row);
private:
	const uint8_t* row_;
	const uint32_t& width_;
};

class Image{
public:
	Image(const uint32_t& width, const uint32_t& height):
		head_(new uint8_t[height*width*pixelsize]), width_(width), height_(height){}
	Image(const std::string& filename);
	Image(const Image& image);
	Image& operator=(const Image& image);
	~Image(){delete[] head_;}
	Row operator[](unsigned int row)const{return Row(head_ + row*width()*pixelsize, width());}
	Image  operator<< (const PatternGenerator& generator)const;
	Image& operator<<=(const PatternGenerator& generator);
	Image  operator<< (std::istream& is)const;
	Image& operator<<=(std::istream& is);
	Image  operator>> (const ImageProcess& process)const;
	Image& operator>>=(const ImageProcess& process);
	Image  operator>> (const PixelConverter& converter)const;
	Image& operator>>=(const PixelConverter& converter);
	Image& operator>>(const std::string& filename)const{return write(filename);}
	Image  operator<< (uint8_t shift)const;
	Image  operator>> (uint8_t shift)const;
	Image& operator<<=(uint8_t shift);
	Image& operator>>=(uint8_t shift);
	Image  operator+(const Image& image)const;
	Image  operator,(const Image& image)const;
	Image  operator& (const Image& image)const;
	Image  operator& (const Pixel<uint16_t>& pixel)const;
	Image& operator&=(const Pixel<uint16_t>& pixel);
	Image  operator& (uint16_t value)const;
	Image& operator&=(uint16_t value);
	Image  operator| (const Image& image)const;
	Image  operator| (const Pixel<uint16_t>& pixel)const;
	Image& operator|=(const Pixel<uint16_t>& pixel);
	Image& write(const std::string& filename)const;
	uint8_t* head()const{return head_;}
	uint8_t* tail()const{return head_ + data_size();}
	const uint32_t& width()const{return width_;}
	const uint32_t& height()const{return height_;}
	uint32_t data_size()const{return height_*width_*pixelsize;}
	Image& swap(Image& rhs);
private:
	class lshifter{
	public:
		lshifter(uint8_t shift): shift_(shift){}
		void            operator()(      Pixel<uint16_t>& pixel)const;
		Pixel<uint16_t> operator()(const Pixel<uint16_t>& pixel)const;
	private:
		uint8_t shift_;
	};
	class rshifter{
	public:
		rshifter(uint8_t shift): shift_(shift){}
		void            operator()(      Pixel<uint16_t>& pixel)const;
		Pixel<uint16_t> operator()(const Pixel<uint16_t>& pixel)const;
	private:
		uint8_t shift_;
	};
	class bit_and{
	public:
		bit_and(const Pixel<uint16_t>& pixel): pixel_(pixel){}
		void            operator()(      Pixel<uint16_t>& pixel)const;
		Pixel<uint16_t> operator()(const Pixel<uint16_t>& pixel)const;
	private:
		const Pixel<uint16_t>& pixel_;
	};
	class bit_or{
	public:
		bit_or(const Pixel<uint16_t>& pixel): pixel_(pixel){}
		void            operator()(      Pixel<uint16_t>& pixel)const;
		Pixel<uint16_t> operator()(const Pixel<uint16_t>& pixel)const;
	private:
		const Pixel<uint16_t>& pixel_;
	};
#ifdef ENABLE_TIFF
	void read_tiff(const std::string& filename);
	Image& write_tiff(const std::string& filename)const;
#endif
#ifdef ENABLE_PNG
	void read_png(const std::string & filename);
	Image& write_png(const std::string& filename)const;
#endif
	uint8_t* head_;
	uint32_t width_;
	uint32_t height_;
};

bool have_ext(const std::string& filename, const std::string& ext);
std::string append_extension(const std::string& filename, const std::string& ext);

#endif
