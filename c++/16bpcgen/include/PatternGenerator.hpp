#ifndef BPCGEN_PATTERN_GENERATOR_HPP_
#define BPCGEN_PATTERN_GENERATOR_HPP_

#include "ImageProcess.hpp"

class PatternGenerator: public ImageProcess{
public:
	virtual ~PatternGenerator(){}
	virtual Image& process(Image& image)const{return generate(image);}
	virtual Image& generate(Image& image)const = 0;
};

#endif
