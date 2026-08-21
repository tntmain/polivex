#include "app/application_session.h"

#include <cmath>

#include "app/rectangle_batch_ops.h"
#include "app/selection_set.h"

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
    selection_.set_single(entity_id);
}

void ApplicationSession::set_selected_rectangles(std::vector<polivex::core::EntityId> entity_ids) noexcept
{
    selection_.set_many(entity_ids);
}

void ApplicationSession::toggle_selected_rectangle(polivex::core::EntityId entity_id) noexcept
{
    selection_.toggle(entity_id);
}

void ApplicationSession::clear_selection() noexcept
{
    selection_.clear();
}

std::optional<polivex::core::EntityId> ApplicationSession::selected_entity_id() const noexcept
{
    return selection_.primary();
}

const std::vector<polivex::core::EntityId>& ApplicationSession::selected_entity_ids() const noexcept
{
    return selection_.ids();
}

bool ApplicationSession::move_selected_rectangle(const polivex::core::Point2D& delta) noexcept
{
    return selected_entity_id().has_value() && active_document_.move_rectangle(*selected_entity_id(), delta);
}

bool ApplicationSession::resize_selected_rectangle(polivex::core::Rectangle2D bounds) noexcept
{
    return selected_entity_id().has_value() && active_document_.set_rectangle_bounds(*selected_entity_id(), bounds);
}

bool ApplicationSession::set_selected_rectangle_rotation(double rotation_degrees) noexcept
{
    return selected_entity_id().has_value()
        && active_document_.set_rectangle_rotation(*selected_entity_id(), rotation_degrees);
}

bool ApplicationSession::set_selected_rectangle_corner_radius(double radius) noexcept
{
    return selected_entity_id().has_value()
        && active_document_.set_rectangle_corner_radius(*selected_entity_id(), radius);
}

bool ApplicationSession::bring_selected_rectangle_to_front() noexcept
{
    return selected_entity_id().has_value() && active_document_.bring_rectangle_to_front(*selected_entity_id());
}

bool ApplicationSession::send_selected_rectangle_to_back() noexcept
{
    return selected_entity_id().has_value() && active_document_.send_rectangle_to_back(*selected_entity_id());
}

bool ApplicationSession::move_selected_rectangle_up() noexcept
{
    return selected_entity_id().has_value() && active_document_.move_rectangle_up(*selected_entity_id());
}

bool ApplicationSession::move_selected_rectangle_down() noexcept
{
    return selected_entity_id().has_value() && active_document_.move_rectangle_down(*selected_entity_id());
}

bool ApplicationSession::set_selected_vector_style(polivex::core::VectorStyle style) noexcept
{
    return selected_entity_id().has_value()
        && active_document_.set_vector_rectangle_style(*selected_entity_id(), style);
}

bool ApplicationSession::align_selected_rectangles_left() noexcept
{
    return align_rectangles_left(active_document_, selection_.ids());
}

bool ApplicationSession::align_selected_rectangles_right() noexcept
{
    return align_rectangles_right(active_document_, selection_.ids());
}

bool ApplicationSession::align_selected_rectangles_horizontal_center() noexcept
{
    return align_rectangles_horizontal_center(active_document_, selection_.ids());
}

bool ApplicationSession::align_selected_rectangles_top() noexcept
{
    return align_rectangles_top(active_document_, selection_.ids());
}

bool ApplicationSession::align_selected_rectangles_bottom() noexcept
{
    return align_rectangles_bottom(active_document_, selection_.ids());
}

bool ApplicationSession::align_selected_rectangles_vertical_middle() noexcept
{
    return align_rectangles_vertical_middle(active_document_, selection_.ids());
}

bool ApplicationSession::distribute_selected_rectangles_horizontally() noexcept
{
    return distribute_rectangles_horizontally(active_document_, selection_.ids());
}

bool ApplicationSession::distribute_selected_rectangles_vertically() noexcept
{
    return distribute_rectangles_vertically(active_document_, selection_.ids());
}

}  // namespace polivex::app
