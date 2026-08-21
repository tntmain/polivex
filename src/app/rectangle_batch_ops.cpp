#include "app/rectangle_batch_ops.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <vector>

namespace polivex::app {

namespace {

struct RectangleSnapshot {
    polivex::core::EntityId id = 0;
    polivex::core::Rectangle2D bounds;
    polivex::core::Rectangle2D frame_bounds;
};

enum class Axis {
    Horizontal,
    Vertical,
};

polivex::core::Rectangle2D frame_bounds_for(const polivex::core::RectangleEntity& rectangle)
{
    const auto& bounds = rectangle.bounds;
    const auto center_x = (bounds.minimum.x + bounds.maximum.x) / 2.0;
    const auto center_y = (bounds.minimum.y + bounds.maximum.y) / 2.0;
    const auto radians = rectangle.rotation_degrees * std::numbers::pi / 180.0;
    const auto cosine = std::cos(radians);
    const auto sine = std::sin(radians);

    const auto rotate = [&](double x, double y) {
        const auto translated_x = x - center_x;
        const auto translated_y = y - center_y;
        return polivex::core::Point2D {
            center_x + translated_x * cosine - translated_y * sine,
            center_y + translated_x * sine + translated_y * cosine,
        };
    };

    const std::array corners {
        rotate(bounds.minimum.x, bounds.minimum.y),
        rotate(bounds.minimum.x, bounds.maximum.y),
        rotate(bounds.maximum.x, bounds.minimum.y),
        rotate(bounds.maximum.x, bounds.maximum.y),
    };

    auto frame = polivex::core::Rectangle2D {corners.front(), corners.front()};
    for (const auto& corner : corners) {
        frame.minimum.x = std::min(frame.minimum.x, corner.x);
        frame.minimum.y = std::min(frame.minimum.y, corner.y);
        frame.maximum.x = std::max(frame.maximum.x, corner.x);
        frame.maximum.y = std::max(frame.maximum.y, corner.y);
    }

    return frame;
}

std::vector<RectangleSnapshot> collect_rectangles(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection)
{
    std::vector<RectangleSnapshot> rectangles;
    rectangles.reserve(selection.size());
    for (const auto id : selection) {
        const auto* rectangle = document.rectangle(id);
        if (rectangle == nullptr) {
            return {};
        }
        rectangles.push_back({id, rectangle->bounds, frame_bounds_for(*rectangle)});
    }

    return rectangles;
}

bool align_by_edge(polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection,
    Axis axis, bool use_minimum)
{
    auto rectangles = collect_rectangles(document, selection);
    if (rectangles.size() < 2) {
        return false;
    }

    const auto edge = [axis, use_minimum](const RectangleSnapshot& rectangle) {
        if (axis == Axis::Horizontal) {
            return use_minimum ? rectangle.frame_bounds.minimum.x : rectangle.frame_bounds.maximum.x;
        }
        return use_minimum ? rectangle.frame_bounds.minimum.y : rectangle.frame_bounds.maximum.y;
    };

    const auto iterator = use_minimum
        ? std::min_element(rectangles.begin(), rectangles.end(), [&](const auto& first, const auto& second) {
              return edge(first) < edge(second);
          })
        : std::max_element(rectangles.begin(), rectangles.end(), [&](const auto& first, const auto& second) {
              return edge(first) < edge(second);
          });
    const auto target = edge(*iterator);

    for (const auto& rectangle : rectangles) {
        const auto current = edge(rectangle);
        const auto delta = target - current;
        if (axis == Axis::Horizontal) {
            (void)document.move_rectangle(rectangle.id, {delta, 0.0});
        } else {
            (void)document.move_rectangle(rectangle.id, {0.0, delta});
        }
    }

    return true;
}

bool align_by_center(polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection,
    Axis axis)
{
    auto rectangles = collect_rectangles(document, selection);
    if (rectangles.size() < 2) {
        return false;
    }

    const auto first_edge = [axis](const RectangleSnapshot& rectangle) {
        return axis == Axis::Horizontal ? rectangle.frame_bounds.minimum.x : rectangle.frame_bounds.minimum.y;
    };
    const auto second_edge = [axis](const RectangleSnapshot& rectangle) {
        return axis == Axis::Horizontal ? rectangle.frame_bounds.maximum.x : rectangle.frame_bounds.maximum.y;
    };

    const auto minimum = std::min_element(rectangles.begin(), rectangles.end(), [&](const auto& first, const auto& second) {
        return first_edge(first) < first_edge(second);
    });
    const auto maximum = std::max_element(rectangles.begin(), rectangles.end(), [&](const auto& first, const auto& second) {
        return second_edge(first) < second_edge(second);
    });

    const auto target = (axis == Axis::Horizontal
            ? (minimum->frame_bounds.minimum.x + maximum->frame_bounds.maximum.x)
            : (minimum->frame_bounds.minimum.y + maximum->frame_bounds.maximum.y))
        / 2.0;

    for (const auto& rectangle : rectangles) {
        const auto center = axis == Axis::Horizontal
            ? (rectangle.frame_bounds.minimum.x + rectangle.frame_bounds.maximum.x) / 2.0
            : (rectangle.frame_bounds.minimum.y + rectangle.frame_bounds.maximum.y) / 2.0;
        const auto delta = target - center;
        if (axis == Axis::Horizontal) {
            (void)document.move_rectangle(rectangle.id, {delta, 0.0});
        } else {
            (void)document.move_rectangle(rectangle.id, {0.0, delta});
        }
    }

    return true;
}

bool distribute(polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection, Axis axis)
{
    auto rectangles = collect_rectangles(document, selection);
    if (rectangles.size() < 3) {
        return false;
    }

    std::sort(rectangles.begin(), rectangles.end(), [axis](const auto& first, const auto& second) {
        return axis == Axis::Horizontal ? first.frame_bounds.minimum.x < second.frame_bounds.minimum.x
                                        : first.frame_bounds.minimum.y < second.frame_bounds.minimum.y;
    });

    const auto first_edge = axis == Axis::Horizontal ? rectangles.front().frame_bounds.minimum.x
                                                     : rectangles.front().frame_bounds.minimum.y;
    const auto last_edge = axis == Axis::Horizontal ? rectangles.back().frame_bounds.maximum.x
                                                    : rectangles.back().frame_bounds.maximum.y;
    double total_size = 0.0;
    for (const auto& rectangle : rectangles) {
        total_size += axis == Axis::Horizontal ? rectangle.frame_bounds.maximum.x - rectangle.frame_bounds.minimum.x
                                               : rectangle.frame_bounds.maximum.y - rectangle.frame_bounds.minimum.y;
    }
    const auto gap = (last_edge - first_edge - total_size) / static_cast<double>(rectangles.size() - 1);

    auto current_edge = first_edge;
    for (auto& rectangle : rectangles) {
        const auto delta = current_edge - (axis == Axis::Horizontal ? rectangle.frame_bounds.minimum.x
                                                                    : rectangle.frame_bounds.minimum.y);
        if (axis == Axis::Horizontal) {
            (void)document.move_rectangle(rectangle.id, {delta, 0.0});
            current_edge += (rectangle.frame_bounds.maximum.x - rectangle.frame_bounds.minimum.x) + gap;
        } else {
            (void)document.move_rectangle(rectangle.id, {0.0, delta});
            current_edge += (rectangle.frame_bounds.maximum.y - rectangle.frame_bounds.minimum.y) + gap;
        }
    }

    return true;
}

}  // namespace

bool align_rectangles_left(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_edge(document, selection, Axis::Horizontal, true);
}

bool align_rectangles_right(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_edge(document, selection, Axis::Horizontal, false);
}

bool align_rectangles_horizontal_center(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_center(document, selection, Axis::Horizontal);
}

bool align_rectangles_top(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_edge(document, selection, Axis::Vertical, false);
}

bool align_rectangles_bottom(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_edge(document, selection, Axis::Vertical, true);
}

bool align_rectangles_vertical_middle(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return align_by_center(document, selection, Axis::Vertical);
}

bool distribute_rectangles_horizontally(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return distribute(document, selection, Axis::Horizontal);
}

bool distribute_rectangles_vertically(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept
{
    return distribute(document, selection, Axis::Vertical);
}

}  // namespace polivex::app
