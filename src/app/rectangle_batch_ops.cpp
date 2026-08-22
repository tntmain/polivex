#include "app/rectangle_batch_ops.h"

#include <algorithm>
#include <vector>

namespace polivex::app {

namespace {

struct RectangleSnapshot {
    polivex::core::EntityId id = 0;
    polivex::core::Rectangle2D bounds;
    polivex::core::Rectangle2D frame_bounds;
};

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
        rectangles.push_back({id, rectangle->bounds, polivex::core::rotated_frame_bounds(*rectangle)});
    }

    return rectangles;
}

enum class Axis {
    Horizontal,
    Vertical,
};

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

    const auto anchor = std::find_if(rectangles.begin(), rectangles.end(), [&](const RectangleSnapshot& rectangle) {
        return rectangle.id == selection.front();
    });
    if (anchor == rectangles.end()) {
        return false;
    }

    const auto target = edge(*anchor);

    for (const auto& rectangle : rectangles) {
        if (rectangle.id == anchor->id) {
            continue;
        }

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
    const auto anchor = std::find_if(rectangles.begin(), rectangles.end(), [&](const RectangleSnapshot& rectangle) {
        return rectangle.id == selection.front();
    });
    if (anchor == rectangles.end()) {
        return false;
    }

    const auto target = axis == Axis::Horizontal
        ? (anchor->frame_bounds.minimum.x + anchor->frame_bounds.maximum.x) / 2.0
        : (anchor->frame_bounds.minimum.y + anchor->frame_bounds.maximum.y) / 2.0;

    for (const auto& rectangle : rectangles) {
        if (rectangle.id == anchor->id) {
            continue;
        }

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
