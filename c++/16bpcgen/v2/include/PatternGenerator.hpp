#ifndef _16BPCGEN_PATTERN_GENERATOR_HPP_
#define _16BPCGEN_PATTERN_GENERATOR_HPP_

#include "ImageProcess.hpp"

class PatternGenerator: public ImageProcess{
public:
	virtual ~PatternGenerator(){}
	virtual FrameBuffer& process(FrameBuffer& buffer)const{return generate(buffer);}
	virtual FrameBuffer& generate(FrameBuffer& buffer)const = 0;
};

#endif
