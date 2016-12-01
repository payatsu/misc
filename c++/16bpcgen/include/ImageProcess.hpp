#ifndef 16BPCGEN_IMAGEPROCESS_HPP_
#define 16BPCGEN_IMAGEPROCESS_HPP_

class Image;

class ImageProcess{
public:
	virtual ~ImageProcess(){}
	virtual Image& process(Image& image)const = 0;
};

#endif
