#ifndef _16BPCGEN_READ_IMG_HPP_
#define _16BPCGEN_READ_IMG_HPP_

#include <string>
#include "FrameBuffer.hpp"

#ifdef ENABLE_TIFF
FrameBuffer read_16bpc_tiff(const std::string& input_filename, int& result);
#endif

#ifdef ENABLE_PNG
FrameBuffer read_16bpc_png(const std::string& input_filename, int& result);
#endif

#endif
