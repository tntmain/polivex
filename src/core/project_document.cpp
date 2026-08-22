#include "core/project_document.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <numbers>
#include <utility>

namespace polivex::core {

namespace {

Point2D rotate_point(const Point2D& point, const Point2D& center, double degrees)
{
    if (degrees == 0.0) {
        return point;
    }

    const auto radians = degrees * std::numbers::pi / 180.0;
    const auto cosine = std::cos(radians);
    const auto sine = std::sin(radians);
    const auto translated_x = point.x - center.x;
    const auto translated_y = point.y - center.y;
    return {
        center.x + translated_x * cosine - translated_y * sine,
        center.y + translated_x * sine + translated_y * cosine,
    };
}

double remap_value(double value, double source_minimum, double source_maximum, double target_minimum, double target_maximum)
{
    const auto source_size = source_maximum - source_minimum;
    if (std::abs(source_size) <= 1e-9) {
        return (target_minimum + target_maximum) / 2.0;
    }

    const auto t = (value - source_minimum) / source_size;
    return target_minimum + (target_maximum - target_minimum) * t;
}

double distance_between(const Point2D& first, const Point2D& second)
{
    return std::hypot(second.x - first.x, second.y - first.y);
}

double maximum_corner_radius_for(const std::array<Point2D, 4>& vertices, std::size_t corner_index)
{
    const auto previous_index = (corner_index + vertices.size() - 1) % vertices.size();
    const auto next_index = (corner_index + 1) % vertices.size();
    return std::min(
        distance_between(vertices[corner_index], vertices[previous_index]),
        distance_between(vertices[corner_index], vertices[next_index]))
        / 2.0;
}

std::string default_rectangle_name(RectangleKind kind, const std::vector<RectangleEntity>& rectangles)
{
    const auto count = std::count_if(rectangles.begin(), rectangles.end(), [kind](const RectangleEntity& rectangle) {
        return rectangle.kind == kind;
    });
    return std::string(kind == RectangleKind::Vector ? "Vector " : "Sketch ") + std::to_string(count + 1);
}

}  // namespace

ProjectDocument::ProjectDocument(std::string name)
    : name_(std::move(name))
{
}

const std::string& ProjectDocument::name() const noexcept
{
    return name_;
}

void ProjectDocument::rename(std::string new_name)
{
    name_ = std::move(new_name);
    dirty_ = true;
}

bool ProjectDocument::is_dirty() const noexcept
{
    return dirty_;
}

void ProjectDocument::mark_dirty(bool dirty) noexcept
{
    dirty_ = dirty;
}

const RectangleEntity& ProjectDocument::add_rectangle(
    RectangleKind kind, Rectangle2D bounds, std::optional<SketchPlane> sketch_plane)
{
    bounds = normalized_rectangle(bounds);

    if (kind == RectangleKind::Sketch && !sketch_plane.has_value()) {
        sketch_plane = SketchPlane::XY;
    }

    const auto vector_style = kind == RectangleKind::Vector ? std::optional<VectorStyle>(VectorStyle {}) : std::nullopt;
    rectangles_.push_back({
        next_entity_id_++,
        kind,
        default_rectangle_name(kind, rectangles_),
        bounds,
        sketch_plane,
        vector_style,
        rectangle_center(bounds),
        rectangle_vertices(bounds),
    });
    mark_dirty();
    return rectangles_.back();
}

std::span<const RectangleEntity> ProjectDocument::rectangles() const noexcept
{
    return rectangles_;
}

const RectangleEntity* ProjectDocument::rectangle(EntityId id) const noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    return iterator == rectangles_.end() ? nullptr : &*iterator;
}

bool ProjectDocument::rename_rectangle(EntityId id, std::string name) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    if (name.empty()) {
        name = rectangle->kind == RectangleKind::Vector ? "Vector" : "Sketch";
    }

    rectangle->name = std::move(name);
    mark_dirty();
    return true;
}

bool ProjectDocument::move_rectangle(EntityId id, Point2D delta) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    rectangle->bounds.minimum.x += delta.x;
    rectangle->bounds.minimum.y += delta.y;
    rectangle->bounds.maximum.x += delta.x;
    rectangle->bounds.maximum.y += delta.y;
    rectangle->pivot.x += delta.x;
    rectangle->pivot.y += delta.y;
    for (auto& vertex : rectangle->vertices) {
        vertex.x += delta.x;
        vertex.y += delta.y;
    }
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_bounds(EntityId id, Rectangle2D bounds) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    const auto previous_bounds = rectangle->bounds;
    bounds = normalized_rectangle(bounds);

    const auto new_pivot = Point2D {
        remap_value(rectangle->pivot.x, previous_bounds.minimum.x, previous_bounds.maximum.x, bounds.minimum.x, bounds.maximum.x),
        remap_value(rectangle->pivot.y, previous_bounds.minimum.y, previous_bounds.maximum.y, bounds.minimum.y, bounds.maximum.y),
    };

    std::array<Point2D, 4> scaled_vertices {};
    for (std::size_t index = 0; index < rectangle->vertices.size(); ++index) {
        scaled_vertices[index] = {
            remap_value(
                rectangle->vertices[index].x, previous_bounds.minimum.x, previous_bounds.maximum.x,
                bounds.minimum.x, bounds.maximum.x),
            remap_value(
                rectangle->vertices[index].y, previous_bounds.minimum.y, previous_bounds.maximum.y,
                bounds.minimum.y, bounds.maximum.y),
        };
    }

    rectangle->bounds = bounds;
    rectangle->pivot = new_pivot;
    rectangle->vertices = scaled_vertices;
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_rotation(EntityId id, double rotation_degrees) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    const auto delta = rotation_degrees - rectangle->rotation_degrees;
    for (auto& vertex : rectangle->vertices) {
        vertex = rotate_point(vertex, rectangle->pivot, delta);
    }
    rectangle->rotation_degrees = rotation_degrees;
    rectangle->bounds = rectangle_bounds_from_vertices(rectangle->vertices);
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_shape(
    EntityId id, const std::array<Point2D, 4>& vertices, Point2D pivot, double rotation_degrees,
    bool has_custom_vertices) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    rectangle->vertices = vertices;
    rectangle->pivot = pivot;
    rectangle->rotation_degrees = rotation_degrees;
    rectangle->bounds = rectangle_bounds_from_vertices(vertices);
    rectangle->has_custom_vertices = has_custom_vertices;
    if (has_custom_vertices) {
        rectangle->corner_radius = 0.0;
        for (std::size_t index = 0; index < rectangle->corner_radii.size(); ++index) {
            rectangle->corner_radii[index] =
                std::clamp(rectangle->corner_radii[index], 0.0, maximum_corner_radius_for(vertices, index));
        }
    } else {
        rectangle->corner_radii.fill(0.0);
    }
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_corner_radius(EntityId id, double radius) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    const auto maximum_radius = std::min(rectangle_width(rectangle->bounds), rectangle_height(rectangle->bounds)) / 2.0;
    rectangle->corner_radius = std::clamp(radius, 0.0, maximum_radius);
    rectangle->corner_radii.fill(0.0);
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_corner_radius(EntityId id, std::size_t corner_index, double radius) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr || corner_index >= rectangle->corner_radii.size()) {
        return false;
    }

    rectangle->has_custom_vertices = true;
    rectangle->corner_radius = 0.0;
    rectangle->corner_radii[corner_index] = std::clamp(
        radius, 0.0, maximum_corner_radius_for(rectangle->vertices, corner_index));
    mark_dirty();
    return true;
}

bool ProjectDocument::set_vector_rectangle_style(EntityId id, VectorStyle style) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr || rectangle->kind != RectangleKind::Vector) {
        return false;
    }

    rectangle->vector_style = style;
    mark_dirty();
    return true;
}

bool ProjectDocument::bring_rectangle_to_front(EntityId id) noexcept
{
    if (rectangles_.empty()) {
        return false;
    }

    return reorder_rectangle(id, rectangles_.size() - 1);
}

bool ProjectDocument::send_rectangle_to_back(EntityId id) noexcept
{
    return reorder_rectangle(id, 0);
}

bool ProjectDocument::move_rectangle_up(EntityId id) noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    if (iterator == rectangles_.end() || std::next(iterator) == rectangles_.end()) {
        return iterator != rectangles_.end();
    }

    return reorder_rectangle(id, static_cast<std::size_t>(std::distance(rectangles_.begin(), iterator)) + 1);
}

bool ProjectDocument::move_rectangle_down(EntityId id) noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    if (iterator == rectangles_.end() || iterator == rectangles_.begin()) {
        return iterator != rectangles_.end();
    }

    return reorder_rectangle(id, static_cast<std::size_t>(std::distance(rectangles_.begin(), iterator)) - 1);
}

bool ProjectDocument::reorder_rectangles(std::span<const EntityId> ordered_ids) noexcept
{
    if (ordered_ids.size() != rectangles_.size()) {
        return false;
    }

    std::vector<RectangleEntity> reordered;
    reordered.reserve(rectangles_.size());

    for (const auto id : ordered_ids) {
        if (std::find_if(reordered.begin(), reordered.end(), [id](const RectangleEntity& rectangle) {
                return rectangle.id == id;
            }) != reordered.end()) {
            return false;
        }

        const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
            return rectangle.id == id;
        });
        if (iterator == rectangles_.end()) {
            return false;
        }

        reordered.push_back(*iterator);
    }

    rectangles_ = std::move(reordered);
    mark_dirty();
    return true;
}

RectangleEntity* ProjectDocument::rectangle_mutable(EntityId id) noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    return iterator == rectangles_.end() ? nullptr : &*iterator;
}

bool ProjectDocument::reorder_rectangle(EntityId id, std::size_t target_index) noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    if (iterator == rectangles_.end()) {
        return false;
    }

    auto rectangle = *iterator;
    rectangles_.erase(iterator);
    if (target_index > rectangles_.size()) {
        target_index = rectangles_.size();
    }
    rectangles_.insert(rectangles_.begin() + static_cast<std::ptrdiff_t>(target_index), rectangle);
    mark_dirty();
    return true;
}

}  // namespace polivex::core
