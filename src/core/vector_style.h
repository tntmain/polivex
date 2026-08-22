#pragma once

#include <cstdint>

namespace polivex::core {

struct VectorStyle {
    std::uint8_t red = 94;
    std::uint8_t green = 156;
    std::uint8_t blue = 255;
    std::uint8_t opacity = 180;
    std::uint8_t stroke_red = 255;
    std::uint8_t stroke_green = 163;
    std::uint8_t stroke_blue = 71;
    std::uint8_t stroke_opacity = 255;
    double stroke_width = 2.0;
};

}  // namespace polivex::core
