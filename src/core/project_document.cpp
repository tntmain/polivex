#include "core/project_document.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace polivex::core {

namespace {

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
    rectangles_.push_back({next_entity_id_++, kind, default_rectangle_name(kind, rectangles_), bounds, sketch_plane, vector_style});
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
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_bounds(EntityId id, Rectangle2D bounds) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    rectangle->bounds = normalized_rectangle(bounds);
    mark_dirty();
    return true;
}

bool ProjectDocument::set_rectangle_rotation(EntityId id, double rotation_degrees) noexcept
{
    auto* rectangle = rectangle_mutable(id);
    if (rectangle == nullptr) {
        return false;
    }

    rectangle->rotation_degrees = rotation_degrees;
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
