#include "Pixel.hpp"

const Pixel black  (0x0,        0x0,        0x0);
const Pixel white  (Pixel::max, Pixel::max, Pixel::max);
const Pixel red    (Pixel::max, 0x0,        0x0);
const Pixel green  (0x0,        Pixel::max, 0x0);
const Pixel blue   (0x0,        0x0,        Pixel::max);
const Pixel cyan   (0x0,        Pixel::max, Pixel::max);
const Pixel magenta(Pixel::max, 0x0,        Pixel::max);
const Pixel yellow (Pixel::max, Pixel::max, 0x0);
