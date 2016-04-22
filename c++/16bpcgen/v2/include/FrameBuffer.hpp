#ifndef _16BPCGEN_FRAMEBUFFER_HPP_
#define _16BPCGEN_FRAMEBUFFER_HPP_

#include <algorithm>
#include <iostream>
#ifdef ENABLE_PNG
#	include <libpng16/png.h>
#endif

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

extern const int bitdepth;
#ifdef ENABLE_PNG
extern const int colortype;
#endif
extern const int pixelsize;

class FrameBuffer;

class PatternGenerator{
public:
	virtual ~PatternGenerator(){}
	virtual void generate(FrameBuffer& buffer)const = 0;
};

class Pixel{
public:
	Pixel(uint16_t R = 0x0000, uint16_t G = 0x0000, uint16_t B = 0x0000): R_(R), G_(G), B_(B){}
	Pixel(unsigned long long int binary): R_(binary>>32&0xffff), G_(binary>>16&0xffff), B_(binary&0xffff){}
	Pixel operator+(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ + rhs.R_),
			static_cast<uint16_t>(G_ + rhs.G_),
			static_cast<uint16_t>(B_ + rhs.B_));
	}
	Pixel operator-(const Pixel& rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_ - rhs.R_),
			static_cast<uint16_t>(G_ - rhs.G_),
			static_cast<uint16_t>(B_ - rhs.B_));
	}
	Pixel operator*(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_*rhs),
			static_cast<uint16_t>(G_*rhs),
			static_cast<uint16_t>(B_*rhs));
	}
	Pixel operator/(uint16_t rhs)const
	{
		return Pixel(
			static_cast<uint16_t>(R_/rhs),
			static_cast<uint16_t>(G_/rhs),
			static_cast<uint16_t>(B_/rhs));
	}
private:
	uint16_t R_;
	uint16_t G_;
	uint16_t B_;
};

extern const Pixel black;
extern const Pixel white;
extern const Pixel red;
extern const Pixel green;
extern const Pixel blue;
extern const Pixel cyan;
extern const Pixel magenta;
extern const Pixel yellow;

class Row{
public:
	Row(uint8_t* row, const uint32_t& width): row_(row), width_(width){}
	const uint32_t& width()const{return width_;}
	Pixel& operator[](int column)const{return *reinterpret_cast<Pixel*>(const_cast<uint8_t*>(row_) + column*pixelsize);}
	Row& operator++(){row_ += width()*pixelsize; return *this;}
	bool operator!=(const Row& rhs)const{return this->row_ != rhs.row_;}
	static void fill(Row first, Row last, const Row& row)
	{
		while(first != last){
			std::copy(&row[0], &row[row.width()], &first[0]);
			++first;
		}
	}
private:
	const uint8_t* row_;
	const uint32_t& width_;
};

class FrameBuffer{
public:
	FrameBuffer(const uint32_t& width, const uint32_t& height):
		head_(new uint8_t[height*width*pixelsize]), width_(width), height_(height){}
	FrameBuffer(const FrameBuffer&);
	~FrameBuffer(){delete[] head_;}
	Row operator[](int row)const{return Row(head_ + row*width()*pixelsize, width());}
	FrameBuffer& operator<<(const PatternGenerator& generator){generator.generate(*this); return *this;}
	FrameBuffer& operator<<(std::istream& is);
	uint8_t* head()const{return head_;}
	uint8_t* tail()const{return head_ + height_*width_*pixelsize;}
	const uint32_t& width()const{return width_;}
	const uint32_t& height()const{return height_;}
	uint32_t size()const{return height_*width_;}
private:
	FrameBuffer& operator=(const FrameBuffer&);
	uint8_t* head_;
	const uint32_t width_;
	const uint32_t height_;
};

#endif
