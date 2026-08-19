#pragma once

#include <optional>

#include "core/project_document.h"
#include "app/workspace.h"

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
    [[nodiscard]] std::optional<polivex::core::EntityId> selected_entity_id() const noexcept;
    [[nodiscard]] bool move_selected_rectangle(const polivex::core::Point2D& delta) noexcept;
    [[nodiscard]] bool set_selected_vector_style(polivex::core::VectorStyle style) noexcept;

private:
    polivex::core::ProjectDocument active_document_;
    ViewportState viewport_state_;
    std::optional<polivex::core::EntityId> selected_entity_id_;
};

}  // namespace polivex::app
