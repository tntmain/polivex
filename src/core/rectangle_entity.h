#pragma once

#include <algorithm>
#include <array>
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
    Point2D pivot;
    std::array<Point2D, 4> vertices {};
    double rotation_degrees = 0.0;
    double corner_radius = 0.0;
    std::array<double, 4> corner_radii {};
    bool has_custom_vertices = false;
};

[[nodiscard]] inline std::array<Point2D, 4> rectangle_vertices(const Rectangle2D& rectangle)
{
    return std::array<Point2D, 4> {{
        Point2D {rectangle.minimum.x, rectangle.maximum.y},
        Point2D {rectangle.maximum.x, rectangle.maximum.y},
        Point2D {rectangle.maximum.x, rectangle.minimum.y},
        Point2D {rectangle.minimum.x, rectangle.minimum.y},
    }};
}

[[nodiscard]] inline Rectangle2D rectangle_bounds_from_vertices(const std::array<Point2D, 4>& vertices)
{
    auto minimum_x = vertices.front().x;
    auto maximum_x = vertices.front().x;
    auto minimum_y = vertices.front().y;
    auto maximum_y = vertices.front().y;

    for (const auto& vertex : vertices) {
        minimum_x = std::min(minimum_x, vertex.x);
        maximum_x = std::max(maximum_x, vertex.x);
        minimum_y = std::min(minimum_y, vertex.y);
        maximum_y = std::max(maximum_y, vertex.y);
    }

    return {
        {minimum_x, minimum_y},
        {maximum_x, maximum_y},
    };
}

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
