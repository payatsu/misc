#ifndef _16BPCGEN_FRAMEBUFFER_HPP_
#define _16BPCGEN_FRAMEBUFFER_HPP_

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
extern const int pixelsize;

class Row{
public:
	Row(uint8_t* row, const uint32_t& width): row_(row), width_(width){}
	const uint32_t& width()const{return width_;}
	Pixel<uint16_t>& operator[](int column)const{return *reinterpret_cast<Pixel<uint16_t>*>(const_cast<uint8_t*>(row_) + column*pixelsize);}
	Row& operator++(){row_ += width()*pixelsize; return *this;}
	bool operator!=(const Row& rhs)const{return this->row_ != rhs.row_;}
	static void fill(Row first, Row last, const Row& row);
private:
	const uint8_t* row_;
	const uint32_t& width_;
};

class FrameBuffer{
public:
	FrameBuffer(const uint32_t& width, const uint32_t& height):
		head_(new uint8_t[height*width*pixelsize]), width_(width), height_(height){}
	FrameBuffer(const std::string& filename);
	FrameBuffer(const FrameBuffer& buffer);
	FrameBuffer& operator=(const FrameBuffer& buffer);
	~FrameBuffer(){delete[] head_;}
	Row operator[](int row)const{return Row(head_ + row * width() * pixelsize, width());}
	FrameBuffer& operator<<(const PatternGenerator& generator);
	FrameBuffer& operator<<(std::istream& is);
	FrameBuffer& operator<<(uint8_t shift);
	FrameBuffer& operator>>(const ImageProcess& process);
	FrameBuffer& operator>>(const PixelConverter& converter);
	FrameBuffer& operator>>(const std::string& filename)const{return write(filename);}
	FrameBuffer& operator>>(uint8_t shift);
	FrameBuffer operator+(const FrameBuffer& buffer)const;
	FrameBuffer operator,(const FrameBuffer& buffer)const;
	FrameBuffer& write(const std::string& filename)const;
	uint8_t* head()const{return head_;}
	uint8_t* tail()const{return head_ + data_size();}
	const uint32_t& width()const{return width_;}
	const uint32_t& height()const{return height_;}
	uint32_t data_size()const{return height_*width_*pixelsize;}
private:
	class lshifter{
	public:
		lshifter(uint8_t shift): shift_(shift){}
		void operator()(Pixel<uint16_t>& pixel)const;
	private:
		uint8_t shift_;
	};
	class rshifter{
	public:
		rshifter(uint8_t shift): shift_(shift){}
		void operator()(Pixel<uint16_t>& pixel)const;
	private:
		uint8_t shift_;
	};
#ifdef ENABLE_TIFF
	void read_tiff(const std::string& filename);
	FrameBuffer& write_tiff(const std::string& filename)const;
#endif
#ifdef ENABLE_PNG
	void read_png(const std::string & filename);
	FrameBuffer& write_png(const std::string& filename)const;
#endif
	uint8_t* head_;
	uint32_t width_;
	uint32_t height_;
};

bool have_ext(const std::string& filename, const std::string& ext);
std::string append_extension(const std::string& filename, const std::string& ext);
#endif
