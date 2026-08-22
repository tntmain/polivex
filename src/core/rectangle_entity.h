#pragma once

#include <cmath>
#include <cstdint>
#include <optional>
#include <numbers>
#include <string>

#include "core/rectangle_2d.h"
#include "core/sketch_plane.h"
#include "core/vector_style.h"

namespace polivex::core {

using EntityId = std::uint64_t;

enum class RectangleKind {
    Vector,
    Sketch,
};

struct RectangleEntity {
    EntityId id = 0;
    RectangleKind kind = RectangleKind::Vector;
    std::string name;
    Rectangle2D bounds;
    std::optional<SketchPlane> sketch_plane;
    std::optional<VectorStyle> vector_style;
    double rotation_degrees = 0.0;
    double corner_radius = 0.0;
};

[[nodiscard]] inline Rectangle2D rotated_frame_bounds(const RectangleEntity& rectangle)
{
    const auto center = rectangle_center(rectangle.bounds);
    const auto half_width = rectangle_width(rectangle.bounds) / 2.0;
    const auto half_height = rectangle_height(rectangle.bounds) / 2.0;
    const auto radians = rectangle.rotation_degrees * std::numbers::pi / 180.0;
    const auto cosine = std::abs(std::cos(radians));
    const auto sine = std::abs(std::sin(radians));
    const auto frame_half_width = half_width * cosine + half_height * sine;
    const auto frame_half_height = half_width * sine + half_height * cosine;

    return {
        {center.x - frame_half_width, center.y - frame_half_height},
        {center.x + frame_half_width, center.y + frame_half_height},
    };
}

}  // namespace polivex::core
