#pragma once

#include <span>

#include "core/project_document.h"

namespace polivex::app {

[[nodiscard]] bool align_rectangles_left(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool align_rectangles_right(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool align_rectangles_horizontal_center(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool align_rectangles_top(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool align_rectangles_bottom(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool align_rectangles_vertical_middle(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool distribute_rectangles_horizontally(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;
[[nodiscard]] bool distribute_rectangles_vertically(
    polivex::core::ProjectDocument& document, std::span<const polivex::core::EntityId> selection) noexcept;

}  // namespace polivex::app
