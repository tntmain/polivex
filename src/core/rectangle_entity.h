#pragma once

#include <cstdint>
#include <optional>

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
    Rectangle2D bounds;
    std::optional<SketchPlane> sketch_plane;
    std::optional<VectorStyle> vector_style;
};

}  // namespace polivex::core
