#include "core/project_document.h"

#include <algorithm>
#include <utility>

namespace polivex::core {

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
    const auto first_corner = bounds.minimum;
    const auto second_corner = bounds.maximum;
    bounds.minimum.x = std::min(first_corner.x, second_corner.x);
    bounds.minimum.y = std::min(first_corner.y, second_corner.y);
    bounds.maximum.x = std::max(first_corner.x, second_corner.x);
    bounds.maximum.y = std::max(first_corner.y, second_corner.y);

    if (kind == RectangleKind::Sketch && !sketch_plane.has_value()) {
        sketch_plane = SketchPlane::XY;
    }

    const auto vector_style = kind == RectangleKind::Vector ? std::optional<VectorStyle>(VectorStyle {}) : std::nullopt;
    rectangles_.push_back({next_entity_id_++, kind, bounds, sketch_plane, vector_style});
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

RectangleEntity* ProjectDocument::rectangle_mutable(EntityId id) noexcept
{
    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [id](const RectangleEntity& rectangle) {
        return rectangle.id == id;
    });
    return iterator == rectangles_.end() ? nullptr : &*iterator;
}

}  // namespace polivex::core
