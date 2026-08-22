#pragma once

#include "core/point_2d.h"

namespace polivex::core {

struct Rectangle2D {
    Point2D minimum;
    Point2D maximum;
};

[[nodiscard]] inline Rectangle2D normalized_rectangle(Rectangle2D rectangle)
{
    if (rectangle.minimum.x > rectangle.maximum.x) {
        const auto temp = rectangle.minimum.x;
        rectangle.minimum.x = rectangle.maximum.x;
        rectangle.maximum.x = temp;
    }

    if (rectangle.minimum.y > rectangle.maximum.y) {
        const auto temp = rectangle.minimum.y;
        rectangle.minimum.y = rectangle.maximum.y;
        rectangle.maximum.y = temp;
    }

    return rectangle;
}

[[nodiscard]] inline double rectangle_width(const Rectangle2D& rectangle)
{
    return rectangle.maximum.x - rectangle.minimum.x;
}

[[nodiscard]] inline double rectangle_height(const Rectangle2D& rectangle)
{
    return rectangle.maximum.y - rectangle.minimum.y;
}

[[nodiscard]] inline Point2D rectangle_center(const Rectangle2D& rectangle)
{
    return {
        (rectangle.minimum.x + rectangle.maximum.x) / 2.0,
        (rectangle.minimum.y + rectangle.maximum.y) / 2.0,
    };
}

[[nodiscard]] inline bool rectangle_contains(const Rectangle2D& rectangle, const Point2D& point)
{
    return point.x >= rectangle.minimum.x && point.x <= rectangle.maximum.x && point.y >= rectangle.minimum.y
        && point.y <= rectangle.maximum.y;
}

}  // namespace polivex::core
