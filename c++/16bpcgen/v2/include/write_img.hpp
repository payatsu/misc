#ifndef _16BPCGEN_WRITE_IMG_HPP_
#define _16BPCGEN_WRITE_IMG_HPP_

#include <string>
#include "FrameBuffer.hpp"

#ifdef ENABLE_TIFF
void generate_16bpc_tiff(const std::string& output_filename, FrameBuffer& buffer);
#endif

uint16_t swap_msb_lsb(uint16_t value);
#ifdef ENABLE_PNG
void generate_16bpc_png(const std::string& output_filename, FrameBuffer& buffer);
#endif
void generate_16bpc_img(const std::string& output_basename, FrameBuffer& buffer);
std::string append_extension(const std::string& filename, const std::string& ext);

#endif
