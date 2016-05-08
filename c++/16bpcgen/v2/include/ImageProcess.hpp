#ifndef _16BPCGEN_IMAGEPROCESS_HPP_
#define _16BPCGEN_IMAGEPROCESS_HPP_

class Image;

class ImageProcess{
public:
	virtual ~ImageProcess(){}
	virtual Image& process(Image& image)const = 0;
};

#endif
