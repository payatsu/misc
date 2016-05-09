#include "Pixel.hpp"

template<>
const Pixel<uint16_t>::value_type Pixel<uint16_t>::max = 0xffff;
template<>
const Pixel<double>::value_type Pixel<double>::max = 0xffff;

const Pixel<uint16_t> black  (0x0,                  0x0,                  0x0);
const Pixel<uint16_t> white  (Pixel<uint16_t>::max, Pixel<uint16_t>::max, Pixel<uint16_t>::max);
const Pixel<uint16_t> red    (Pixel<uint16_t>::max, 0x0,                  0x0);
const Pixel<uint16_t> green  (0x0,                  Pixel<uint16_t>::max, 0x0);
const Pixel<uint16_t> blue   (0x0,                  0x0,                  Pixel<uint16_t>::max);
const Pixel<uint16_t> cyan   (0x0,                  Pixel<uint16_t>::max, Pixel<uint16_t>::max);
const Pixel<uint16_t> magenta(Pixel<uint16_t>::max, 0x0,                  Pixel<uint16_t>::max);
const Pixel<uint16_t> yellow (Pixel<uint16_t>::max, Pixel<uint16_t>::max, 0x0);
