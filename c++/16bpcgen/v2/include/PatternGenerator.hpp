#ifndef _16BPCGEN_PATTERN_GENERATOR_HPP_
#define _16BPCGEN_PATTERN_GENERATOR_HPP_

class FrameBuffer;

class PatternGenerator{
public:
	virtual ~PatternGenerator(){}
	virtual void generate(FrameBuffer& buffer)const = 0;
};

#endif
