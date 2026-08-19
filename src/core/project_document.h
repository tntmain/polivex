#pragma once

#include <span>
#include <string>
#include <vector>

#include "core/rectangle_entity.h"

namespace polivex::core {

class ProjectDocument {
public:
    explicit ProjectDocument(std::string name = "Untitled");

    [[nodiscard]] const std::string& name() const noexcept;
    void rename(std::string new_name);

    [[nodiscard]] bool is_dirty() const noexcept;
    void mark_dirty(bool dirty = true) noexcept;

    const RectangleEntity& add_rectangle(
        RectangleKind kind, Rectangle2D bounds, std::optional<SketchPlane> sketch_plane = std::nullopt);
    [[nodiscard]] std::span<const RectangleEntity> rectangles() const noexcept;
    [[nodiscard]] const RectangleEntity* rectangle(EntityId id) const noexcept;
    [[nodiscard]] bool move_rectangle(EntityId id, Point2D delta) noexcept;
    [[nodiscard]] bool set_vector_rectangle_style(EntityId id, VectorStyle style) noexcept;

private:
    [[nodiscard]] RectangleEntity* rectangle_mutable(EntityId id) noexcept;

    std::string name_;
    bool dirty_ = false;
    EntityId next_entity_id_ = 1;
    std::vector<RectangleEntity> rectangles_;
};

}  // namespace polivex::core
