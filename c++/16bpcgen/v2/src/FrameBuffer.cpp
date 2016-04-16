#include "FrameBuffer.hpp"

const int bitdepth  = 16;
#ifdef ENABLE_PNG
const int colortype = PNG_COLOR_TYPE_RGB;
#endif
const int pixelsize = 6;

const Pixel black  (0x0000, 0x0000, 0x0000);
const Pixel white  (0xffff, 0xffff, 0xffff);
const Pixel red    (0xffff, 0x0000, 0x0000);
const Pixel green  (0x0000, 0xffff, 0x0000);
const Pixel blue   (0x0000, 0x0000, 0xffff);
const Pixel cyan   (0x0000, 0xffff, 0xffff);
const Pixel magenta(0xffff, 0x0000, 0xffff);
const Pixel yellow (0xffff, 0xffff, 0x0000);
