#ifndef _16BPCGEN_IMAGEPROCESS_HPP_
#define _16BPCGEN_IMAGEPROCESS_HPP_

class FrameBuffer;

class ImageProcess{
public:
	virtual ~ImageProcess(){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const = 0;
};

#endif
