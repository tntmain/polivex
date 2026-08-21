#pragma once

#include <optional>

#include "core/project_document.h"
#include "app/workspace.h"
#include "app/selection_set.h"

namespace polivex::app {

class ApplicationSession {
public:
    ApplicationSession();

    [[nodiscard]] const polivex::core::ProjectDocument& active_document() const noexcept;
    [[nodiscard]] polivex::core::ProjectDocument& active_document() noexcept;

    void create_new_document();

    [[nodiscard]] const ViewportState& viewport_state() const noexcept;
    [[nodiscard]] ViewportState& viewport_state() noexcept;
    void set_workspace(Workspace workspace) noexcept;
    void set_camera_preset(CameraPreset preset) noexcept;
    void set_active_tool(ActiveTool tool) noexcept;
    [[nodiscard]] bool create_rectangle(const polivex::core::Point2D& first_corner,
        const polivex::core::Point2D& second_corner);
    void select_rectangle(std::optional<polivex::core::EntityId> entity_id) noexcept;
    void set_selected_rectangles(std::vector<polivex::core::EntityId> entity_ids) noexcept;
    void toggle_selected_rectangle(polivex::core::EntityId entity_id) noexcept;
    void clear_selection() noexcept;
    [[nodiscard]] std::optional<polivex::core::EntityId> selected_entity_id() const noexcept;
    [[nodiscard]] const std::vector<polivex::core::EntityId>& selected_entity_ids() const noexcept;
    [[nodiscard]] bool move_selected_rectangle(const polivex::core::Point2D& delta) noexcept;
    [[nodiscard]] bool resize_selected_rectangle(polivex::core::Rectangle2D bounds) noexcept;
    [[nodiscard]] bool set_selected_rectangle_rotation(double rotation_degrees) noexcept;
    [[nodiscard]] bool set_selected_rectangle_corner_radius(double radius) noexcept;
    [[nodiscard]] bool bring_selected_rectangle_to_front() noexcept;
    [[nodiscard]] bool send_selected_rectangle_to_back() noexcept;
    [[nodiscard]] bool move_selected_rectangle_up() noexcept;
    [[nodiscard]] bool move_selected_rectangle_down() noexcept;
    [[nodiscard]] bool set_selected_vector_style(polivex::core::VectorStyle style) noexcept;
    [[nodiscard]] bool align_selected_rectangles_left() noexcept;
    [[nodiscard]] bool align_selected_rectangles_right() noexcept;
    [[nodiscard]] bool align_selected_rectangles_horizontal_center() noexcept;
    [[nodiscard]] bool align_selected_rectangles_top() noexcept;
    [[nodiscard]] bool align_selected_rectangles_bottom() noexcept;
    [[nodiscard]] bool align_selected_rectangles_vertical_middle() noexcept;
    [[nodiscard]] bool distribute_selected_rectangles_horizontally() noexcept;
    [[nodiscard]] bool distribute_selected_rectangles_vertically() noexcept;

private:
    polivex::core::ProjectDocument active_document_;
    ViewportState viewport_state_;
    SelectionSet selection_;
};

}  // namespace polivex::app
