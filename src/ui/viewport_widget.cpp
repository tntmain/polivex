#include "ui/viewport_widget.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

#include <QLineF>
#include <QApplication>
#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QRectF>
#include <QString>
#include <QTransform>
#include <QWheelEvent>

namespace polivex::ui {

namespace {

constexpr double kBasePixelsPerUnit = 48.0;
constexpr double kMinimumZoom = 0.1;
constexpr double kMaximumZoom = 20.0;
constexpr double kSelectionStrokeWidth = 1.0;
constexpr double kHandleSizePx = 8.0;
constexpr double kEdgeResizeGrabPx = 7.0;
constexpr double kRadiusHandleSizePx = 9.0;
constexpr double kRadiusHandleInsetPx = 6.0;
constexpr double kSnapThresholdPx = 5.0;

enum class AxisTarget {
    Left,
    Center,
    Right,
    Bottom,
    Middle,
    Top,
};

struct AxisCandidate {
    double scene = 0.0;
    double screen = 0.0;
};

struct SnapResult {
    double delta_scene = 0.0;
    double guide_scene = 0.0;
    bool matched = false;
};

struct FrameGeometry {
    QPointF top_left;
    QPointF top_right;
    QPointF bottom_right;
    QPointF bottom_left;
    QPointF top_center;
    QPointF right_center;
    QPointF bottom_center;
    QPointF left_center;
    QPointF center;
};

QPointF to_point(const polivex::core::Point2D& point)
{
    return {point.x, point.y};
}

QPointF midpoint(const QPointF& first, const QPointF& second)
{
    return {(first.x() + second.x()) / 2.0, (first.y() + second.y()) / 2.0};
}

double dot_product(const QPointF& first, const QPointF& second)
{
    return first.x() * second.x() + first.y() * second.y();
}

QPointF rotate_point(const QPointF& point, const QPointF& center, double degrees)
{
    if (degrees == 0.0) {
        return point;
    }

    const auto radians = degrees * std::numbers::pi / 180.0;
    const auto cosine = std::cos(radians);
    const auto sine = std::sin(radians);
    const auto translated_x = point.x() - center.x();
    const auto translated_y = point.y() - center.y();
    return {
        center.x() + translated_x * cosine - translated_y * sine,
        center.y() + translated_x * sine + translated_y * cosine,
    };
}

QPointF inverse_rotate_point(const QPointF& point, const QPointF& center, double degrees)
{
    return rotate_point(point, center, -degrees);
}

FrameGeometry make_frame_geometry(const polivex::core::Rectangle2D& bounds, double rotation_degrees)
{
    const QPointF top_left {bounds.minimum.x, bounds.maximum.y};
    const QPointF top_right {bounds.maximum.x, bounds.maximum.y};
    const QPointF bottom_right {bounds.maximum.x, bounds.minimum.y};
    const QPointF bottom_left {bounds.minimum.x, bounds.minimum.y};
    const auto center = to_point(polivex::core::rectangle_center(bounds));

    return {
        rotate_point(top_left, center, rotation_degrees),
        rotate_point(top_right, center, rotation_degrees),
        rotate_point(bottom_right, center, rotation_degrees),
        rotate_point(bottom_left, center, rotation_degrees),
        rotate_point(midpoint(top_left, top_right), center, rotation_degrees),
        rotate_point(midpoint(top_right, bottom_right), center, rotation_degrees),
        rotate_point(midpoint(bottom_left, bottom_right), center, rotation_degrees),
        rotate_point(midpoint(top_left, bottom_left), center, rotation_degrees),
        center,
    };
}

FrameGeometry make_frame_geometry(const polivex::core::RectangleEntity& rectangle)
{
    return make_frame_geometry(rectangle.bounds, rectangle.rotation_degrees);
}

QPointF radius_handle_position(
    const polivex::core::RectangleEntity& rectangle, ViewportWidget::CornerRadiusHandle handle, double scale)
{
    const auto& bounds = rectangle.bounds;
    const auto inset = kRadiusHandleInsetPx / std::max(scale, 0.0001);
    const auto max_radius = std::min(polivex::core::rectangle_width(bounds), polivex::core::rectangle_height(bounds)) / 2.0;
    const auto offset = std::clamp(rectangle.corner_radius + inset, 0.0, max_radius);
    const QPointF top_left {bounds.minimum.x + offset, bounds.maximum.y - offset};
    const QPointF top_right {bounds.maximum.x - offset, bounds.maximum.y - offset};
    const QPointF bottom_right {bounds.maximum.x - offset, bounds.minimum.y + offset};
    const QPointF bottom_left {bounds.minimum.x + offset, bounds.minimum.y + offset};
    const auto center = to_point(polivex::core::rectangle_center(bounds));

    switch (handle) {
    case ViewportWidget::CornerRadiusHandle::TopLeft:
        return rotate_point(top_left, center, rectangle.rotation_degrees);
    case ViewportWidget::CornerRadiusHandle::TopRight:
        return rotate_point(top_right, center, rectangle.rotation_degrees);
    case ViewportWidget::CornerRadiusHandle::BottomRight:
        return rotate_point(bottom_right, center, rectangle.rotation_degrees);
    case ViewportWidget::CornerRadiusHandle::BottomLeft:
        return rotate_point(bottom_left, center, rectangle.rotation_degrees);
    case ViewportWidget::CornerRadiusHandle::None:
        break;
    }

    return center;
}

Qt::CursorShape cursor_for_resize_handle(ViewportWidget::ResizeHandle handle)
{
    switch (handle) {
    case ViewportWidget::ResizeHandle::TopLeft:
    case ViewportWidget::ResizeHandle::BottomRight:
        return Qt::SizeFDiagCursor;
    case ViewportWidget::ResizeHandle::TopRight:
    case ViewportWidget::ResizeHandle::BottomLeft:
        return Qt::SizeBDiagCursor;
    case ViewportWidget::ResizeHandle::Left:
    case ViewportWidget::ResizeHandle::Right:
        return Qt::SizeHorCursor;
    case ViewportWidget::ResizeHandle::Top:
    case ViewportWidget::ResizeHandle::Bottom:
        return Qt::SizeVerCursor;
    case ViewportWidget::ResizeHandle::None:
        break;
    }

    return Qt::ArrowCursor;
}

std::vector<AxisCandidate> x_candidates_for(const polivex::core::RectangleEntity& rectangle, double scale)
{
    const auto& bounds = rectangle.bounds;
    const auto center_x = (bounds.minimum.x + bounds.maximum.x) / 2.0;
    return {
        {bounds.minimum.x, bounds.minimum.x * scale},
        {center_x, center_x * scale},
        {bounds.maximum.x, bounds.maximum.x * scale},
    };
}

std::vector<AxisCandidate> y_candidates_for(const polivex::core::RectangleEntity& rectangle, double scale)
{
    const auto& bounds = rectangle.bounds;
    const auto center_y = (bounds.minimum.y + bounds.maximum.y) / 2.0;
    return {
        {bounds.minimum.y, -bounds.minimum.y * scale},
        {center_y, -center_y * scale},
        {bounds.maximum.y, -bounds.maximum.y * scale},
    };
}

std::vector<AxisCandidate> x_candidates_for(const polivex::core::Rectangle2D& bounds, double scale, AxisTarget target)
{
    const auto center_x = (bounds.minimum.x + bounds.maximum.x) / 2.0;
    switch (target) {
    case AxisTarget::Left:
        return {{bounds.minimum.x, bounds.minimum.x * scale}};
    case AxisTarget::Center:
        return {{center_x, center_x * scale}};
    case AxisTarget::Right:
        return {{bounds.maximum.x, bounds.maximum.x * scale}};
    case AxisTarget::Bottom:
    case AxisTarget::Middle:
    case AxisTarget::Top:
        break;
    }

    return {};
}

std::vector<AxisCandidate> y_candidates_for(const polivex::core::Rectangle2D& bounds, double scale, AxisTarget target)
{
    const auto center_y = (bounds.minimum.y + bounds.maximum.y) / 2.0;
    switch (target) {
    case AxisTarget::Bottom:
        return {{bounds.minimum.y, -bounds.minimum.y * scale}};
    case AxisTarget::Middle:
        return {{center_y, -center_y * scale}};
    case AxisTarget::Top:
        return {{bounds.maximum.y, -bounds.maximum.y * scale}};
    case AxisTarget::Left:
    case AxisTarget::Center:
    case AxisTarget::Right:
        break;
    }

    return {};
}

SnapResult best_snap(const std::vector<AxisCandidate>& selected, const std::vector<AxisCandidate>& others)
{
    SnapResult result;
    double best_distance = std::numeric_limits<double>::max();

    for (const auto& selected_candidate : selected) {
        for (const auto& other_candidate : others) {
            const auto distance = std::abs(selected_candidate.screen - other_candidate.screen);
            if (distance > kSnapThresholdPx || distance >= best_distance) {
                continue;
            }

            best_distance = distance;
            result.delta_scene = other_candidate.scene - selected_candidate.scene;
            result.guide_scene = other_candidate.scene;
            result.matched = true;
        }
    }

    return result;
}

QRectF handle_rect_at(const QPointF& center, double size_px)
{
    return {center.x() - size_px / 2.0, center.y() - size_px / 2.0, size_px, size_px};
}

bool contains_handle(const QPointF& screen_position, const QPointF& handle_center, double size_px)
{
    return handle_rect_at(handle_center, size_px).contains(screen_position);
}

}  // namespace

ViewportWidget::ViewportWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(360, 240);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void ViewportWidget::set_viewport_state(const polivex::app::ViewportState& state)
{
    state_ = state;
    grid_visible_ = state.grid_style == polivex::app::GridStyle::Visible;
    grid_spacing_ = std::max(0.1, state.grid_spacing);
    background_transparent_ = state.background_style == polivex::app::BackgroundStyle::Transparent;
    background_color_ = QColor(state.background_red, state.background_green, state.background_blue);
    setAttribute(Qt::WA_TranslucentBackground, background_transparent_);
    update();
}

void ViewportWidget::set_rectangles(std::span<const polivex::core::RectangleEntity> rectangles)
{
    rectangles_.assign(rectangles.begin(), rectangles.end());
    update();
}

void ViewportWidget::set_selected_entity_id(std::optional<polivex::core::EntityId> entity_id)
{
    const auto previous = selected_entity_ids_;
    selected_entity_ids_.clear();
    if (entity_id.has_value()) {
        selected_entity_ids_.push_back(*entity_id);
    }
    if (selected_entity_ids_ != previous) {
        selection_handle_mode_ = SelectionHandleMode::Scale;
    }
    update();
}

void ViewportWidget::set_selected_entity_ids(std::span<const polivex::core::EntityId> entity_ids)
{
    const auto previous = selected_entity_ids_;
    selected_entity_ids_.assign(entity_ids.begin(), entity_ids.end());
    if (selected_entity_ids_ != previous) {
        selection_handle_mode_ = SelectionHandleMode::Scale;
    }
    update();
}

void ViewportWidget::set_grid_visible(bool visible)
{
    grid_visible_ = visible;
    update();
}

void ViewportWidget::set_grid_spacing(double spacing)
{
    grid_spacing_ = std::max(0.1, spacing);
    update();
}

void ViewportWidget::set_background_color(const QColor& color)
{
    background_color_ = color;
    background_transparent_ = false;
    setAttribute(Qt::WA_TranslucentBackground, false);
    update();
}

void ViewportWidget::set_background_transparent(bool transparent)
{
    background_transparent_ = transparent;
    setAttribute(Qt::WA_TranslucentBackground, transparent);
    update();
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (is_creating_rectangle_) {
        const auto point = screen_to_scene(event->position());
        rectangle_current_ = {point.x(), point.y()};
        update();
        return;
    }

    if (is_pending_selected_drag_) {
        if ((event->pos() - pending_press_position_).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }

        is_pending_selected_drag_ = false;
        pending_toggle_selection_mode_ = false;
        is_moving_selected_rectangle_ = true;
        interaction_mode_ = InteractionMode::Move;
        setCursor(Qt::SizeAllCursor);
    }

    if (is_moving_selected_rectangle_ || is_resizing_selected_rectangle_ || is_rotating_selected_rectangle_
        || is_adjusting_corner_radius_) {
        const auto selected = selected_rectangle();
        if (!selected.has_value()) {
            return;
        }

        snap_guides_.clear();
        const auto current_scene = screen_to_scene(event->position());
        auto bounds = interaction_initial_bounds_;

        if (is_moving_selected_rectangle_) {
            const polivex::core::Point2D absolute_delta {
                current_scene.x() - interaction_press_scene_.x(),
                current_scene.y() - interaction_press_scene_.y(),
            };
            bounds.minimum.x += absolute_delta.x;
            bounds.maximum.x += absolute_delta.x;
            bounds.minimum.y += absolute_delta.y;
            bounds.maximum.y += absolute_delta.y;

            if (selected->rotation_degrees == 0.0) {
                const auto scale = kBasePixelsPerUnit * state_.zoom;
                const auto center_x = (bounds.minimum.x + bounds.maximum.x) / 2.0;
                const auto center_y = (bounds.minimum.y + bounds.maximum.y) / 2.0;
                const std::vector<AxisCandidate> selected_x {
                    {bounds.minimum.x, bounds.minimum.x * scale},
                    {center_x, center_x * scale},
                    {bounds.maximum.x, bounds.maximum.x * scale},
                };
                const std::vector<AxisCandidate> selected_y {
                    {bounds.minimum.y, -bounds.minimum.y * scale},
                    {center_y, -center_y * scale},
                    {bounds.maximum.y, -bounds.maximum.y * scale},
                };
                std::vector<AxisCandidate> other_x;
                std::vector<AxisCandidate> other_y;
                for (const auto& rectangle : rectangles_) {
                    if (is_selected(rectangle.id)) {
                        continue;
                    }
                    const auto x_set = x_candidates_for(rectangle, scale);
                    const auto y_set = y_candidates_for(rectangle, scale);
                    other_x.insert(other_x.end(), x_set.begin(), x_set.end());
                    other_y.insert(other_y.end(), y_set.begin(), y_set.end());
                }

                const auto snap_x = best_snap(selected_x, other_x);
                const auto snap_y = best_snap(selected_y, other_y);
                if (snap_x.matched) {
                    const auto delta_scene_x = snap_x.delta_scene;
                    bounds.minimum.x += delta_scene_x;
                    bounds.maximum.x += delta_scene_x;
                    const auto guide_x = scene_to_screen(QPointF(snap_x.guide_scene, 0.0)).x();
                    snap_guides_.push_back(QLineF(QPointF(guide_x, 0.0), QPointF(guide_x, height())));
                }
                if (snap_y.matched) {
                    const auto delta_scene_y = snap_y.delta_scene;
                    bounds.minimum.y += delta_scene_y;
                    bounds.maximum.y += delta_scene_y;
                    const auto guide_y = scene_to_screen(QPointF(0.0, snap_y.guide_scene)).y();
                    snap_guides_.push_back(QLineF(QPointF(0.0, guide_y), QPointF(width(), guide_y)));
                }
            }

            const polivex::core::Point2D emitted_delta {
                bounds.minimum.x - interaction_initial_bounds_.minimum.x - last_move_delta_.x,
                bounds.minimum.y - interaction_initial_bounds_.minimum.y - last_move_delta_.y,
            };
            last_move_delta_ = {
                bounds.minimum.x - interaction_initial_bounds_.minimum.x,
                bounds.minimum.y - interaction_initial_bounds_.minimum.y,
            };
            emit selected_rectangle_move_requested(emitted_delta);
            update();
            return;
        }

        if (is_resizing_selected_rectangle_) {
            const auto initial_center = to_point(polivex::core::rectangle_center(interaction_initial_bounds_));
            const auto radians = interaction_initial_rotation_ * std::numbers::pi / 180.0;
            const QPointF axis_x {std::cos(radians), std::sin(radians)};
            const QPointF axis_y {-std::sin(radians), std::cos(radians)};
            const auto initial_frame = make_frame_geometry(interaction_initial_bounds_, interaction_initial_rotation_);

            auto make_bounds = [](const QPointF& center, double half_width, double half_height) {
                return polivex::core::Rectangle2D {
                    {center.x() - half_width, center.y() - half_height},
                    {center.x() + half_width, center.y() + half_height},
                };
            };

            switch (active_resize_handle_) {
            case ResizeHandle::TopLeft:
            case ResizeHandle::TopRight:
            case ResizeHandle::BottomRight:
            case ResizeHandle::BottomLeft: {
                QPointF anchor;
                switch (active_resize_handle_) {
                case ResizeHandle::TopLeft:
                    anchor = initial_frame.bottom_right;
                    break;
                case ResizeHandle::TopRight:
                    anchor = initial_frame.bottom_left;
                    break;
                case ResizeHandle::BottomRight:
                    anchor = initial_frame.top_left;
                    break;
                case ResizeHandle::BottomLeft:
                    anchor = initial_frame.top_right;
                    break;
                case ResizeHandle::Top:
                case ResizeHandle::Right:
                case ResizeHandle::Bottom:
                case ResizeHandle::Left:
                case ResizeHandle::None:
                    break;
                }

                const auto delta = current_scene - anchor;
                const auto width = std::max(std::abs(dot_product(delta, axis_x)), 0.001);
                const auto height = std::max(std::abs(dot_product(delta, axis_y)), 0.001);
                const auto center = anchor + axis_x * (dot_product(delta, axis_x) / 2.0)
                    + axis_y * (dot_product(delta, axis_y) / 2.0);
                bounds = make_bounds(center, width / 2.0, height / 2.0);
                break;
            }
            case ResizeHandle::Left:
            case ResizeHandle::Right: {
                const auto anchor = active_resize_handle_ == ResizeHandle::Left ? initial_frame.right_center : initial_frame.left_center;
                const auto delta = current_scene - anchor;
                const auto width = std::max(std::abs(dot_product(delta, axis_x)), 0.001);
                const auto direction = dot_product(delta, axis_x) >= 0.0 ? 1.0 : -1.0;
                const auto center = anchor + axis_x * (direction * width / 2.0);
                bounds = make_bounds(center, width / 2.0, polivex::core::rectangle_height(interaction_initial_bounds_) / 2.0);
                break;
            }
            case ResizeHandle::Top:
            case ResizeHandle::Bottom: {
                const auto anchor = active_resize_handle_ == ResizeHandle::Top ? initial_frame.bottom_center : initial_frame.top_center;
                const auto delta = current_scene - anchor;
                const auto height = std::max(std::abs(dot_product(delta, axis_y)), 0.001);
                const auto direction = dot_product(delta, axis_y) >= 0.0 ? 1.0 : -1.0;
                const auto center = anchor + axis_y * (direction * height / 2.0);
                bounds = make_bounds(center, polivex::core::rectangle_width(interaction_initial_bounds_) / 2.0, height / 2.0);
                break;
            }
            case ResizeHandle::None:
                break;
            }

            if (selected->rotation_degrees == 0.0) {
                const auto scale = kBasePixelsPerUnit * state_.zoom;
                std::vector<AxisCandidate> selected_x;
                std::vector<AxisCandidate> selected_y;
                std::vector<AxisCandidate> other_x;
                std::vector<AxisCandidate> other_y;

                switch (active_resize_handle_) {
                case ResizeHandle::TopLeft:
                case ResizeHandle::Left:
                case ResizeHandle::BottomLeft:
                    selected_x = x_candidates_for(bounds, scale, AxisTarget::Left);
                    break;
                case ResizeHandle::TopRight:
                case ResizeHandle::Right:
                case ResizeHandle::BottomRight:
                    selected_x = x_candidates_for(bounds, scale, AxisTarget::Right);
                    break;
                case ResizeHandle::Top:
                case ResizeHandle::Bottom:
                case ResizeHandle::None:
                    break;
                }

                switch (active_resize_handle_) {
                case ResizeHandle::TopLeft:
                case ResizeHandle::Top:
                case ResizeHandle::TopRight:
                    selected_y = y_candidates_for(bounds, scale, AxisTarget::Top);
                    break;
                case ResizeHandle::BottomLeft:
                case ResizeHandle::Bottom:
                case ResizeHandle::BottomRight:
                    selected_y = y_candidates_for(bounds, scale, AxisTarget::Bottom);
                    break;
                case ResizeHandle::Left:
                case ResizeHandle::Right:
                case ResizeHandle::None:
                    break;
                }

                for (const auto& rectangle : rectangles_) {
                    if (is_selected(rectangle.id)) {
                        continue;
                    }
                    const auto x_set = x_candidates_for(rectangle, scale);
                    const auto y_set = y_candidates_for(rectangle, scale);
                    other_x.insert(other_x.end(), x_set.begin(), x_set.end());
                    other_y.insert(other_y.end(), y_set.begin(), y_set.end());
                }

                const auto snap_x = !selected_x.empty() ? best_snap(selected_x, other_x) : SnapResult {};
                const auto snap_y = !selected_y.empty() ? best_snap(selected_y, other_y) : SnapResult {};
                if (snap_x.matched) {
                    if (active_resize_handle_ == ResizeHandle::TopLeft || active_resize_handle_ == ResizeHandle::Left
                        || active_resize_handle_ == ResizeHandle::BottomLeft) {
                        bounds.minimum.x += snap_x.delta_scene;
                    } else if (active_resize_handle_ == ResizeHandle::TopRight || active_resize_handle_ == ResizeHandle::Right
                        || active_resize_handle_ == ResizeHandle::BottomRight) {
                        bounds.maximum.x += snap_x.delta_scene;
                    }
                    const auto guide_x = scene_to_screen(QPointF(snap_x.guide_scene, 0.0)).x();
                    snap_guides_.push_back(QLineF(QPointF(guide_x, 0.0), QPointF(guide_x, height())));
                }
                if (snap_y.matched) {
                    if (active_resize_handle_ == ResizeHandle::TopLeft || active_resize_handle_ == ResizeHandle::Top
                        || active_resize_handle_ == ResizeHandle::TopRight) {
                        bounds.maximum.y += snap_y.delta_scene;
                    } else if (active_resize_handle_ == ResizeHandle::BottomLeft || active_resize_handle_ == ResizeHandle::Bottom
                        || active_resize_handle_ == ResizeHandle::BottomRight) {
                        bounds.minimum.y += snap_y.delta_scene;
                    }
                    const auto guide_y = scene_to_screen(QPointF(0.0, snap_y.guide_scene)).y();
                    snap_guides_.push_back(QLineF(QPointF(0.0, guide_y), QPointF(width(), guide_y)));
                }
            }

            emit selected_rectangle_resize_requested(bounds);
            update();
            return;
        }

        if (is_rotating_selected_rectangle_) {
            const auto center = to_point(polivex::core::rectangle_center(interaction_initial_bounds_));
            const auto current_angle = std::atan2(current_scene.y() - center.y(), current_scene.x() - center.x());
            const auto delta_degrees = (current_angle - interaction_initial_mouse_angle_) * 180.0 / std::numbers::pi;
            emit selected_rectangle_rotation_requested(interaction_initial_rotation_ + delta_degrees);
            update();
            return;
        }

        if (is_adjusting_corner_radius_) {
            const auto center = to_point(polivex::core::rectangle_center(interaction_initial_bounds_));
            const auto local_point = inverse_rotate_point(current_scene, center, interaction_initial_rotation_);
            double radius = interaction_initial_corner_radius_;
            switch (active_corner_radius_handle_) {
            case CornerRadiusHandle::TopLeft:
                radius = std::clamp(std::min(local_point.x() - interaction_initial_bounds_.minimum.x,
                                           interaction_initial_bounds_.maximum.y - local_point.y()),
                    0.0, std::min(polivex::core::rectangle_width(interaction_initial_bounds_),
                              polivex::core::rectangle_height(interaction_initial_bounds_)) / 2.0);
                break;
            case CornerRadiusHandle::TopRight:
                radius = std::clamp(std::min(interaction_initial_bounds_.maximum.x - local_point.x(),
                                           interaction_initial_bounds_.maximum.y - local_point.y()),
                    0.0, std::min(polivex::core::rectangle_width(interaction_initial_bounds_),
                              polivex::core::rectangle_height(interaction_initial_bounds_)) / 2.0);
                break;
            case CornerRadiusHandle::BottomRight:
                radius = std::clamp(std::min(interaction_initial_bounds_.maximum.x - local_point.x(),
                                           local_point.y() - interaction_initial_bounds_.minimum.y),
                    0.0, std::min(polivex::core::rectangle_width(interaction_initial_bounds_),
                              polivex::core::rectangle_height(interaction_initial_bounds_)) / 2.0);
                break;
            case CornerRadiusHandle::BottomLeft:
                radius = std::clamp(std::min(local_point.x() - interaction_initial_bounds_.minimum.x,
                                           local_point.y() - interaction_initial_bounds_.minimum.y),
                    0.0, std::min(polivex::core::rectangle_width(interaction_initial_bounds_),
                              polivex::core::rectangle_height(interaction_initial_bounds_)) / 2.0);
                break;
            case CornerRadiusHandle::None:
                break;
            }

            emit selected_rectangle_corner_radius_requested(radius);
            update();
            return;
        }
    }

    if (!is_panning_) {
        apply_hover_cursor(event->position());
        return;
    }

    const auto delta = event->pos() - last_mouse_position_;
    last_mouse_position_ = event->pos();
    emit pan_requested(delta.x(), delta.y());
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        is_panning_ = true;
        last_mouse_position_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() == Qt::LeftButton && navigation_cube_rect().contains(event->pos())) {
        emit camera_preset_requested(polivex::app::CameraPreset::Top);
        return;
    }

    if (event->button() == Qt::LeftButton && state_.active_tool == polivex::app::ActiveTool::Select) {
        const auto multi_select = event->modifiers().testFlag(Qt::ShiftModifier);
        const auto current_selected = selected_rectangle();
        const auto is_selected_entity = [this](polivex::core::EntityId entity_id) {
            return std::find(selected_entity_ids_.begin(), selected_entity_ids_.end(), entity_id) != selected_entity_ids_.end();
        };
        const auto hit = rectangle_at(event->position());
        const auto primary_selected_id = selected_entity_ids_.size() == 1
            ? std::optional<polivex::core::EntityId>(selected_entity_ids_.front())
            : std::nullopt;
        const auto hit_is_primary = hit.has_value() && primary_selected_id.has_value() && *hit == *primary_selected_id;
        const auto hit_is_selected = hit.has_value() && is_selected_entity(*hit);

        auto begin_pending_move = [&](const polivex::core::RectangleEntity& rectangle, bool allow_toggle) {
            interaction_press_scene_ = screen_to_scene(event->position());
            interaction_initial_bounds_ = rectangle.bounds;
            interaction_initial_rotation_ = rectangle.rotation_degrees;
            interaction_initial_corner_radius_ = rectangle.corner_radius;
            last_move_delta_ = {0.0, 0.0};
            pending_press_position_ = event->pos();
            is_pending_selected_drag_ = true;
            pending_toggle_selection_mode_ = allow_toggle;
        };

        auto begin_interaction = [&](const SelectionHit& selection_hit, const polivex::core::RectangleEntity& rectangle) {
            interaction_mode_ = selection_hit.mode;
            interaction_press_scene_ = screen_to_scene(event->position());
            interaction_initial_bounds_ = rectangle.bounds;
            interaction_initial_rotation_ = rectangle.rotation_degrees;
            interaction_initial_corner_radius_ = rectangle.corner_radius;
            last_move_delta_ = {0.0, 0.0};

            switch (selection_hit.mode) {
            case InteractionMode::Move:
                begin_pending_move(rectangle, hit_is_primary);
                return;
            case InteractionMode::Resize:
                is_resizing_selected_rectangle_ = true;
                active_resize_handle_ = selection_hit.resize_handle;
                break;
            case InteractionMode::Rotate: {
                is_rotating_selected_rectangle_ = true;
                const auto center = to_point(polivex::core::rectangle_center(interaction_initial_bounds_));
                interaction_initial_mouse_angle_ = std::atan2(interaction_press_scene_.y() - center.y(),
                    interaction_press_scene_.x() - center.x());
                break;
            }
            case InteractionMode::CornerRadius:
                is_adjusting_corner_radius_ = true;
                active_corner_radius_handle_ = selection_hit.corner_radius_handle;
                break;
            case InteractionMode::None:
                break;
            }

            setCursor(selection_hit.cursor);
        };

        if (!multi_select && selected_entity_ids_.size() > 1 && hit_is_selected) {
            const auto iterator = std::find(selected_entity_ids_.begin(), selected_entity_ids_.end(), *hit);
            if (iterator != selected_entity_ids_.begin()) {
                const auto active_id = *iterator;
                selected_entity_ids_.erase(iterator);
                selected_entity_ids_.insert(selected_entity_ids_.begin(), active_id);
                emit selection_changed(selected_entity_ids_);
            }
            emit rectangle_selected(*hit);
            update();
            apply_hover_cursor(event->position());
            return;
        }

        if (!multi_select && current_selected.has_value()) {
            const auto selection_hit = hit_test_selection(*current_selected, event->position());
            if (selection_hit.matched) {
                begin_interaction(selection_hit, *current_selected);
                return;
            }
        }

        if (multi_select && hit.has_value()) {
            const auto iterator = std::find(selected_entity_ids_.begin(), selected_entity_ids_.end(), *hit);
            if (iterator != selected_entity_ids_.end()) {
                selected_entity_ids_.erase(iterator);
            } else {
                selected_entity_ids_.insert(selected_entity_ids_.begin(), *hit);
            }
            selection_handle_mode_ = SelectionHandleMode::Scale;
        } else {
            selected_entity_ids_.clear();
            if (hit.has_value()) {
                selected_entity_ids_.push_back(*hit);
            }
            if (!hit_is_primary) {
                selection_handle_mode_ = SelectionHandleMode::Scale;
            }
        }
        emit rectangle_selected(hit);
        emit selection_changed(selected_entity_ids_);

        if (!hit.has_value()) {
            update();
            apply_hover_cursor(event->position());
            return;
        }

        const auto selected = selected_rectangle();
        if (!selected.has_value()) {
            return;
        }

        begin_pending_move(*selected, false);
        return;
    }

    if (event->button() == Qt::LeftButton && state_.active_tool == polivex::app::ActiveTool::Rectangle
        && state_.workspace != polivex::app::Workspace::Model) {
        const auto point = screen_to_scene(event->position());
        rectangle_start_ = {point.x(), point.y()};
        rectangle_current_ = rectangle_start_;
        is_creating_rectangle_ = true;
        setCursor(Qt::CrossCursor);
    }
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton && is_panning_) {
        is_panning_ = false;
        unsetCursor();
    }

    if (event->button() == Qt::LeftButton && is_pending_selected_drag_) {
        is_pending_selected_drag_ = false;
        const auto should_toggle_mode = pending_toggle_selection_mode_
            && (event->pos() - pending_press_position_).manhattanLength() < QApplication::startDragDistance();
        pending_toggle_selection_mode_ = false;
        interaction_mode_ = InteractionMode::None;
        last_move_delta_ = {0.0, 0.0};
        snap_guides_.clear();
        if (should_toggle_mode) {
            selection_handle_mode_ = selection_handle_mode_ == SelectionHandleMode::Scale
                ? SelectionHandleMode::Transform
                : SelectionHandleMode::Scale;
        }
        apply_hover_cursor(event->position());
        update();
        return;
    }

    if (event->button() == Qt::LeftButton && is_creating_rectangle_) {
        is_creating_rectangle_ = false;
        apply_hover_cursor(event->position());

        if (std::abs(rectangle_current_.x - rectangle_start_.x) > 0.001
            && std::abs(rectangle_current_.y - rectangle_start_.y) > 0.001) {
            emit rectangle_creation_requested(rectangle_start_, rectangle_current_);
        }

        update();
    }

    if (event->button() == Qt::LeftButton && (is_moving_selected_rectangle_ || is_resizing_selected_rectangle_
                                                 || is_rotating_selected_rectangle_
                                                 || is_adjusting_corner_radius_)) {
        pending_toggle_selection_mode_ = false;
        is_moving_selected_rectangle_ = false;
        is_resizing_selected_rectangle_ = false;
        is_rotating_selected_rectangle_ = false;
        is_adjusting_corner_radius_ = false;
        interaction_mode_ = InteractionMode::None;
        active_resize_handle_ = ResizeHandle::None;
        active_corner_radius_handle_ = CornerRadiusHandle::None;
        last_move_delta_ = {0.0, 0.0};
        snap_guides_.clear();
        apply_hover_cursor(event->position());
        update();
    }
}

void ViewportWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (selected_entity_ids_.empty()) {
        event->ignore();
        return;
    }

    emit scene_context_menu_requested(event->globalPos());
    event->accept();
}

void ViewportWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (background_transparent_) {
        painter.fillRect(rect(), Qt::transparent);
    } else {
        painter.fillRect(rect(), background_color_);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    draw_grid(painter);
    draw_axes(painter);
    draw_rectangles(painter);
    draw_selection_overlay(painter);
    draw_snap_guides(painter);
    draw_navigation_cube(painter);
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    const auto zoom_factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    emit zoom_requested(zoom_factor);
    event->accept();
}

QPointF ViewportWidget::screen_to_scene(const QPointF& screen_position) const
{
    const auto scale = kBasePixelsPerUnit * state_.zoom;
    return {
        (screen_position.x() - width() / 2.0 - state_.pan_x) / scale,
        -(screen_position.y() - height() / 2.0 - state_.pan_y) / scale,
    };
}

QPointF ViewportWidget::scene_to_screen(const QPointF& scene_position) const
{
    const auto scale = kBasePixelsPerUnit * state_.zoom;
    return {
        width() / 2.0 + state_.pan_x + scene_position.x() * scale,
        height() / 2.0 + state_.pan_y - scene_position.y() * scale,
    };
}

ViewportWidget::SelectionHit ViewportWidget::hit_test_selection(
    const polivex::core::RectangleEntity& rectangle, const QPointF& screen_position) const
{
    SelectionHit hit;
    const auto scale = kBasePixelsPerUnit * state_.zoom;
    const auto frame = make_frame_geometry(rectangle);
    const auto to_screen = [this](const QPointF& point) { return scene_to_screen(point); };
    const auto corner_handles = std::array {
        std::pair {ResizeHandle::TopLeft, to_screen(frame.top_left)},
        std::pair {ResizeHandle::TopRight, to_screen(frame.top_right)},
        std::pair {ResizeHandle::BottomRight, to_screen(frame.bottom_right)},
        std::pair {ResizeHandle::BottomLeft, to_screen(frame.bottom_left)},
    };
    const auto edge_handles = std::array {
        std::pair {ResizeHandle::Top, to_screen(frame.top_center)},
        std::pair {ResizeHandle::Right, to_screen(frame.right_center)},
        std::pair {ResizeHandle::Bottom, to_screen(frame.bottom_center)},
        std::pair {ResizeHandle::Left, to_screen(frame.left_center)},
    };

    if (selection_handle_mode_ == SelectionHandleMode::Scale) {
        const auto radius_handles = std::array {
            std::pair {CornerRadiusHandle::TopLeft, to_screen(radius_handle_position(rectangle, CornerRadiusHandle::TopLeft, scale))},
            std::pair {CornerRadiusHandle::TopRight, to_screen(radius_handle_position(rectangle, CornerRadiusHandle::TopRight, scale))},
            std::pair {CornerRadiusHandle::BottomRight, to_screen(radius_handle_position(rectangle, CornerRadiusHandle::BottomRight, scale))},
            std::pair {CornerRadiusHandle::BottomLeft, to_screen(radius_handle_position(rectangle, CornerRadiusHandle::BottomLeft, scale))},
        };
        for (const auto& [handle, position] : radius_handles) {
            if (contains_handle(screen_position, position, kRadiusHandleSizePx)) {
                hit.mode = InteractionMode::CornerRadius;
                hit.corner_radius_handle = handle;
                hit.cursor = Qt::CrossCursor;
                hit.matched = true;
                return hit;
            }
        }

        for (const auto& [handle, position] : corner_handles) {
            if (contains_handle(screen_position, position, kHandleSizePx)) {
                hit.mode = InteractionMode::Resize;
                hit.resize_handle = handle;
                hit.cursor = cursor_for_resize_handle(handle);
                hit.matched = true;
                return hit;
            }
        }

        for (const auto& [handle, position] : edge_handles) {
            if (contains_handle(screen_position, position, kHandleSizePx)) {
                hit.mode = InteractionMode::Resize;
                hit.resize_handle = handle;
                hit.cursor = cursor_for_resize_handle(handle);
                hit.matched = true;
                return hit;
            }
        }
    } else {
        for (const auto& [handle, position] : corner_handles) {
            if (contains_handle(screen_position, position, kHandleSizePx)) {
                hit.mode = InteractionMode::Rotate;
                hit.resize_handle = handle;
                hit.cursor = Qt::CrossCursor;
                hit.matched = true;
                return hit;
            }
        }

        for (const auto& [handle, position] : edge_handles) {
            if (contains_handle(screen_position, position, kHandleSizePx)) {
                hit.mode = InteractionMode::Rotate;
                hit.resize_handle = handle;
                hit.cursor = Qt::CrossCursor;
                hit.matched = true;
                return hit;
            }
        }
    }

    const auto center = to_point(polivex::core::rectangle_center(rectangle.bounds));
    const auto local_point = inverse_rotate_point(screen_to_scene(screen_position), center, rectangle.rotation_degrees);
    const auto grab = kEdgeResizeGrabPx / std::max(scale, 0.0001);
    const auto corner_guard = std::max(grab * 2.0, kHandleSizePx / std::max(scale, 0.0001));
    const auto& bounds = rectangle.bounds;

    const auto within_vertical_span = [bounds, corner_guard](double y) {
        return y > bounds.minimum.y + corner_guard && y < bounds.maximum.y - corner_guard;
    };
    const auto within_horizontal_span = [bounds, corner_guard](double x) {
        return x > bounds.minimum.x + corner_guard && x < bounds.maximum.x - corner_guard;
    };

    if (std::abs(local_point.x() - bounds.minimum.x) <= grab && within_vertical_span(local_point.y())) {
        hit.mode = InteractionMode::Resize;
        hit.resize_handle = ResizeHandle::Left;
        hit.cursor = cursor_for_resize_handle(hit.resize_handle);
        hit.matched = true;
        return hit;
    }
    if (std::abs(local_point.x() - bounds.maximum.x) <= grab && within_vertical_span(local_point.y())) {
        hit.mode = InteractionMode::Resize;
        hit.resize_handle = ResizeHandle::Right;
        hit.cursor = cursor_for_resize_handle(hit.resize_handle);
        hit.matched = true;
        return hit;
    }
    if (std::abs(local_point.y() - bounds.maximum.y) <= grab && within_horizontal_span(local_point.x())) {
        hit.mode = InteractionMode::Resize;
        hit.resize_handle = ResizeHandle::Top;
        hit.cursor = cursor_for_resize_handle(hit.resize_handle);
        hit.matched = true;
        return hit;
    }
    if (std::abs(local_point.y() - bounds.minimum.y) <= grab && within_horizontal_span(local_point.x())) {
        hit.mode = InteractionMode::Resize;
        hit.resize_handle = ResizeHandle::Bottom;
        hit.cursor = cursor_for_resize_handle(hit.resize_handle);
        hit.matched = true;
        return hit;
    }

    if (polivex::core::rectangle_contains(bounds, {local_point.x(), local_point.y()})) {
        hit.mode = InteractionMode::Move;
        hit.cursor = Qt::SizeAllCursor;
        hit.matched = true;
    }

    return hit;
}

void ViewportWidget::apply_hover_cursor(const QPointF& screen_position)
{
    if (state_.active_tool == polivex::app::ActiveTool::Rectangle
        && state_.workspace != polivex::app::Workspace::Model) {
        setCursor(Qt::CrossCursor);
        return;
    }

    const auto selected = selected_rectangle();
    if (state_.active_tool == polivex::app::ActiveTool::Select && selected.has_value()) {
        const auto hit = hit_test_selection(*selected, screen_position);
        if (hit.matched) {
            setCursor(hit.cursor);
            return;
        }
    }

    unsetCursor();
}

void ViewportWidget::draw_grid(QPainter& painter) const
{
    if (!grid_visible_ || state_.camera_preset != polivex::app::CameraPreset::Top) {
        return;
    }

    const auto scale = kBasePixelsPerUnit * state_.zoom;
    const auto spacing = std::max(12.0, scale * grid_spacing_);
    const auto origin = scene_to_screen({0.0, 0.0});

    painter.setPen(QPen(QColor("#303842"), 1.0));
    for (auto x = std::fmod(origin.x(), spacing); x < width(); x += spacing) {
        painter.drawLine(QPointF(x, 0.0), QPointF(x, height()));
    }
    for (auto y = std::fmod(origin.y(), spacing); y < height(); y += spacing) {
        painter.drawLine(QPointF(0.0, y), QPointF(width(), y));
    }
}

void ViewportWidget::draw_axes(QPainter& painter) const
{
    const auto origin = scene_to_screen({0.0, 0.0});

    painter.setPen(QPen(QColor("#e85d75"), 2.0));
    painter.drawLine(QPointF(0.0, origin.y()), QPointF(width(), origin.y()));
    painter.setPen(QPen(QColor("#65c466"), 2.0));
    painter.drawLine(QPointF(origin.x(), 0.0), QPointF(origin.x(), height()));

    painter.setPen(QColor("#f2f5f7"));
    painter.drawText(QPointF(width() - 24.0, origin.y() - 8.0), "X");
    painter.drawText(QPointF(origin.x() + 8.0, 18.0), "Y");
}

void ViewportWidget::draw_navigation_cube(QPainter& painter) const
{
    const auto cube = navigation_cube_rect();
    const auto is_top_view = state_.camera_preset == polivex::app::CameraPreset::Top;

    painter.setBrush(QColor("#2e3742"));
    painter.setPen(QPen(QColor("#8996a6"), 1.0));
    painter.drawRoundedRect(cube, 8.0, 8.0);

    const auto face = cube.adjusted(10, 10, -10, -10);
    painter.setBrush(is_top_view ? QColor("#4c8bf5") : QColor("#475361"));
    painter.drawRoundedRect(face, 4.0, 4.0);
    painter.setPen(Qt::white);
    QString label;
    switch (state_.camera_preset) {
    case polivex::app::CameraPreset::Top:
        label = "TOP";
        break;
    case polivex::app::CameraPreset::Front:
        label = "FRONT";
        break;
    case polivex::app::CameraPreset::Right:
        label = "RIGHT";
        break;
    case polivex::app::CameraPreset::Isometric:
        label = "ISO";
        break;
    }
    painter.drawText(face, Qt::AlignCenter, label);
}

void ViewportWidget::draw_rectangles(QPainter& painter) const
{
    painter.save();
    const auto scale = kBasePixelsPerUnit * state_.zoom;
    painter.translate(width() / 2.0 + state_.pan_x, height() / 2.0 + state_.pan_y);
    painter.scale(scale, -scale);

    const auto draw_rectangle = [this, &painter, scale](const polivex::core::RectangleEntity& rectangle) {
        const auto style = rectangle.kind == polivex::core::RectangleKind::Vector
            ? rectangle.vector_style.value_or(polivex::core::VectorStyle {})
            : polivex::core::VectorStyle {101, 196, 102, 25};
        const auto border = rectangle.kind == polivex::core::RectangleKind::Vector ? QColor(style.red, style.green, style.blue)
                                                                                    : QColor("#65c466");
        const auto fill = rectangle.kind == polivex::core::RectangleKind::Vector
            ? QColor(style.red, style.green, style.blue, style.opacity)
            : QColor(101, 196, 102, 25);

        painter.save();
        const auto center = to_point(polivex::core::rectangle_center(rectangle.bounds));
        painter.translate(center);
        painter.rotate(rectangle.rotation_degrees);
        painter.translate(-center);
        const auto selected = is_selected(rectangle.id);
        painter.setPen(QPen(selected ? QColor("#f3bd5b") : border, (selected ? 2.0 : 2.0) / scale));
        painter.setBrush(fill);
        const QRectF local_rect {
            QPointF(rectangle.bounds.minimum.x, rectangle.bounds.minimum.y),
            QPointF(rectangle.bounds.maximum.x, rectangle.bounds.maximum.y),
        };
        painter.drawRoundedRect(local_rect.normalized(), rectangle.corner_radius, rectangle.corner_radius);
        painter.restore();
    };

    for (const auto& rectangle : rectangles_) {
        draw_rectangle(rectangle);
    }

    if (is_creating_rectangle_) {
        const QRectF local_rect {QPointF(rectangle_start_.x, rectangle_start_.y), QPointF(rectangle_current_.x, rectangle_current_.y)};
        painter.setPen(QPen(QColor("#f3bd5b"), 2.0 / scale));
        painter.setBrush(QColor(243, 189, 91, 35));
        painter.drawRect(local_rect.normalized());
    }

    painter.restore();
}

void ViewportWidget::draw_selection_overlay(QPainter& painter) const
{
    const auto selected = selected_rectangle();
    if (!selected.has_value()) {
        return;
    }

    const auto frame = make_frame_geometry(*selected);
    const auto to_screen = [this](const QPointF& point) { return scene_to_screen(point); };

    painter.save();
    painter.setPen(QPen(QColor("#f3bd5b"), kSelectionStrokeWidth, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);

    QPolygonF polygon;
    polygon << to_screen(frame.top_left) << to_screen(frame.top_right) << to_screen(frame.bottom_right)
            << to_screen(frame.bottom_left);
    painter.drawPolygon(polygon);

    const auto draw_square_handle = [&painter](const QPointF& point, double size, const QColor& color) {
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 1.0));
        painter.drawRect(QRectF(point.x() - size / 2.0, point.y() - size / 2.0, size, size));
    };
    const auto draw_round_handle = [&painter](const QPointF& point, double size, const QColor& color) {
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 1.0));
        painter.drawEllipse(QRectF(point.x() - size / 2.0, point.y() - size / 2.0, size, size));
    };
    const auto draw_radius_handle = [&painter](const QPointF& point, double size, const QColor& color) {
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 1.0));
        painter.drawEllipse(QRectF(point.x() - size / 2.0, point.y() - size / 2.0, size, size));
    };

    const auto handle_color = QColor("#f3bd5b");
    const auto transform_color = QColor("#67d4ff");
    const auto radius_color = QColor("#ff7ab3");
    if (selection_handle_mode_ == SelectionHandleMode::Scale) {
        draw_square_handle(to_screen(frame.top_left), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.top_center), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.top_right), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.right_center), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.bottom_right), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.bottom_center), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.bottom_left), kHandleSizePx, handle_color);
        draw_square_handle(to_screen(frame.left_center), kHandleSizePx, handle_color);

        const auto scale = kBasePixelsPerUnit * state_.zoom;
        draw_radius_handle(
            to_screen(radius_handle_position(*selected, CornerRadiusHandle::TopLeft, scale)), kRadiusHandleSizePx, radius_color);
        draw_radius_handle(
            to_screen(radius_handle_position(*selected, CornerRadiusHandle::TopRight, scale)), kRadiusHandleSizePx, radius_color);
        draw_radius_handle(
            to_screen(radius_handle_position(*selected, CornerRadiusHandle::BottomRight, scale)), kRadiusHandleSizePx, radius_color);
        draw_radius_handle(
            to_screen(radius_handle_position(*selected, CornerRadiusHandle::BottomLeft, scale)), kRadiusHandleSizePx, radius_color);
    } else {
        draw_round_handle(to_screen(frame.top_left), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.top_center), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.top_right), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.right_center), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.bottom_right), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.bottom_center), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.bottom_left), kHandleSizePx, transform_color);
        draw_round_handle(to_screen(frame.left_center), kHandleSizePx, transform_color);
        painter.setPen(QPen(transform_color, 1.5));
        painter.drawLine(to_screen(frame.center) + QPointF(-6.0, 0.0), to_screen(frame.center) + QPointF(6.0, 0.0));
        painter.drawLine(to_screen(frame.center) + QPointF(0.0, -6.0), to_screen(frame.center) + QPointF(0.0, 6.0));
    }
    draw_selection_badge(painter, *selected);
    painter.restore();
}

void ViewportWidget::draw_selection_badge(QPainter& painter, const polivex::core::RectangleEntity& rectangle) const
{
    const auto frame = make_frame_geometry(rectangle);
    const auto to_screen = [this](const QPointF& point) { return scene_to_screen(point); };
    QPolygonF polygon;
    polygon << to_screen(frame.top_left) << to_screen(frame.top_right) << to_screen(frame.bottom_right)
            << to_screen(frame.bottom_left);
    const auto selection_bounds = polygon.boundingRect();
    const auto text = selection_badge_text(rectangle);
    const QFontMetrics metrics(painter.font());
    const auto text_width = metrics.horizontalAdvance(text);
    const auto badge_width = text_width + 20.0;
    const auto badge_height = metrics.height() + 10.0;
    const auto badge_margin = 8.0;
    const auto badge_x = std::clamp(
        selection_bounds.center().x() - badge_width / 2.0,
        badge_margin,
        std::max(badge_margin, width() - badge_width - badge_margin));
    const auto badge_y = std::clamp(
        selection_bounds.bottom() + 12.0,
        badge_margin,
        std::max(badge_margin, height() - badge_height - badge_margin));
    const auto badge_rect = QRectF(badge_x, badge_y, badge_width, badge_height);

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(25, 30, 36, 225));
    painter.drawRoundedRect(badge_rect, 8.0, 8.0);
    painter.setPen(QColor("#f5f8fb"));
    painter.drawText(badge_rect, Qt::AlignCenter, text);
    painter.restore();
}

void ViewportWidget::draw_snap_guides(QPainter& painter) const
{
    if (snap_guides_.empty()) {
        return;
    }

    painter.save();
    painter.setPen(QPen(QColor("#ff4fd8"), 1.0));
    for (const auto& guide : snap_guides_) {
        painter.drawLine(guide);
    }
    painter.restore();
}

std::optional<polivex::core::EntityId> ViewportWidget::rectangle_at(const QPointF& screen_position) const
{
    const auto scene_position = screen_to_scene(screen_position);
    for (auto iterator = rectangles_.rbegin(); iterator != rectangles_.rend(); ++iterator) {
        const auto center = to_point(polivex::core::rectangle_center(iterator->bounds));
        const auto local_point = inverse_rotate_point(scene_position, center, iterator->rotation_degrees);
        if (polivex::core::rectangle_contains(iterator->bounds, {local_point.x(), local_point.y()})) {
            return iterator->id;
        }
    }

    return std::nullopt;
}

std::optional<polivex::core::RectangleEntity> ViewportWidget::selected_rectangle() const
{
    if (selected_entity_ids_.empty()) {
        return std::nullopt;
    }

    const auto iterator = std::find_if(rectangles_.begin(), rectangles_.end(), [this](const auto& rectangle) {
        return !selected_entity_ids_.empty() && rectangle.id == selected_entity_ids_.front();
    });
    if (iterator == rectangles_.end()) {
        return std::nullopt;
    }

    return *iterator;
}

bool ViewportWidget::is_selected(polivex::core::EntityId entity_id) const
{
    return std::find(selected_entity_ids_.begin(), selected_entity_ids_.end(), entity_id) != selected_entity_ids_.end();
}

QString ViewportWidget::selection_badge_text(const polivex::core::RectangleEntity& rectangle) const
{
    const auto frame = make_frame_geometry(rectangle);
    const auto to_screen = [this](const QPointF& point) { return scene_to_screen(point); };
    QPolygonF polygon;
    polygon << to_screen(frame.top_left) << to_screen(frame.top_right) << to_screen(frame.bottom_right)
            << to_screen(frame.bottom_left);
    const auto screen_bounds = polygon.boundingRect();
    const auto width_units = polivex::core::rectangle_width(rectangle.bounds);
    const auto height_units = polivex::core::rectangle_height(rectangle.bounds);

    return QString("%1 x %2 px  |  %3 x %4 u")
        .arg(screen_bounds.width(), 0, 'f', 0)
        .arg(screen_bounds.height(), 0, 'f', 0)
        .arg(width_units, 0, 'f', 2)
        .arg(height_units, 0, 'f', 2);
}

QRect ViewportWidget::navigation_cube_rect() const
{
    constexpr int cube_size = 72;
    constexpr int margin = 18;
    return {width() - cube_size - margin, margin, cube_size, cube_size};
}

}  // namespace polivex::ui
