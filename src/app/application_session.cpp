#include "app/application_session.h"

namespace polivex::app {

ApplicationSession::ApplicationSession()
    : active_document_("Untitled")
{
}

const polivex::core::ProjectDocument& ApplicationSession::active_document() const noexcept
{
    return active_document_;
}

polivex::core::ProjectDocument& ApplicationSession::active_document() noexcept
{
    return active_document_;
}

void ApplicationSession::create_new_document()
{
    active_document_ = polivex::core::ProjectDocument("Untitled");
}

const ViewportState& ApplicationSession::viewport_state() const noexcept
{
    return viewport_state_;
}

ViewportState& ApplicationSession::viewport_state() noexcept
{
    return viewport_state_;
}

void ApplicationSession::set_workspace(Workspace workspace) noexcept
{
    viewport_state_.workspace = workspace;

    if (workspace != Workspace::Model) {
        viewport_state_.camera_preset = CameraPreset::Top;
    }
}

void ApplicationSession::set_camera_preset(CameraPreset preset) noexcept
{
    viewport_state_.camera_preset = preset;
}

void ApplicationSession::set_active_tool(ActiveTool tool) noexcept
{
    viewport_state_.active_tool = tool;
}

bool ApplicationSession::create_rectangle(
    const polivex::core::Point2D& first_corner, const polivex::core::Point2D& second_corner)
{
    const polivex::core::Rectangle2D bounds {first_corner, second_corner};

    switch (viewport_state_.workspace) {
    case Workspace::Vector:
        active_document_.add_rectangle(polivex::core::RectangleKind::Vector, bounds);
        return true;
    case Workspace::Sketch:
        active_document_.add_rectangle(
            polivex::core::RectangleKind::Sketch, bounds, polivex::core::SketchPlane::XY);
        return true;
    case Workspace::Model:
        return false;
    }

    return false;
}

void ApplicationSession::select_rectangle(std::optional<polivex::core::EntityId> entity_id) noexcept
{
    selected_entity_id_ = entity_id;
}

std::optional<polivex::core::EntityId> ApplicationSession::selected_entity_id() const noexcept
{
    return selected_entity_id_;
}

bool ApplicationSession::move_selected_rectangle(const polivex::core::Point2D& delta) noexcept
{
    return selected_entity_id_.has_value() && active_document_.move_rectangle(*selected_entity_id_, delta);
}

bool ApplicationSession::set_selected_vector_style(polivex::core::VectorStyle style) noexcept
{
    return selected_entity_id_.has_value()
        && active_document_.set_vector_rectangle_style(*selected_entity_id_, style);
}

}  // namespace polivex::app
