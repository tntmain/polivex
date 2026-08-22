#pragma once

#include <optional>
#include <span>
#include <vector>

#include <QColor>
#include <QPoint>
#include <QWidget>

#include "app/workspace.h"
#include "core/point_2d.h"
#include "core/rectangle_entity.h"

class QMouseEvent;
class QPaintEvent;
class QPainter;
class QContextMenuEvent;
class QWheelEvent;
class QString;

namespace polivex::ui {

class ViewportWidget : public QWidget {
    Q_OBJECT

public:
    explicit ViewportWidget(QWidget* parent = nullptr);

    void set_viewport_state(const polivex::app::ViewportState& state);
    void set_rectangles(std::span<const polivex::core::RectangleEntity> rectangles);
    void set_selected_entity_id(std::optional<polivex::core::EntityId> entity_id);
    void set_selected_entity_ids(std::span<const polivex::core::EntityId> entity_ids);
    void set_grid_visible(bool visible);
    void set_grid_spacing(double spacing);
    void set_background_color(const QColor& color);
    void set_background_transparent(bool transparent);

signals:
    void pan_requested(double delta_x, double delta_y);
    void zoom_requested(double factor);
    void camera_preset_requested(polivex::app::CameraPreset preset);
    void rectangle_creation_requested(
        const polivex::core::Point2D& first_corner, const polivex::core::Point2D& second_corner);
    void rectangle_selected(std::optional<polivex::core::EntityId> entity_id);
    void selection_changed(std::vector<polivex::core::EntityId> entity_ids);
    void selected_rectangle_move_requested(const polivex::core::Point2D& delta);
    void selected_rectangle_resize_requested(const polivex::core::Rectangle2D& bounds);
    void selected_rectangle_rotation_requested(double rotation_degrees);
    void selected_rectangle_corner_radius_requested(double radius);
    void selected_rectangle_bring_to_front_requested();
    void selected_rectangle_send_to_back_requested();
    void selected_rectangle_move_up_requested();
    void selected_rectangle_move_down_requested();
    void scene_context_menu_requested(const QPoint& global_position);

public:
    enum class InteractionMode {
        None,
        Move,
        Resize,
        Rotate,
        CornerRadius,
    };

    enum class SelectionHandleMode {
        Scale,
        Transform,
    };

    enum class ResizeHandle {
        None,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
    };

    enum class CornerRadiusHandle {
        None,
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,
    };

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct SelectionHit {
        InteractionMode mode = InteractionMode::None;
        ResizeHandle resize_handle = ResizeHandle::None;
        CornerRadiusHandle corner_radius_handle = CornerRadiusHandle::None;
        Qt::CursorShape cursor = Qt::ArrowCursor;
        bool matched = false;
    };

    [[nodiscard]] QPointF screen_to_scene(const QPointF& screen_position) const;
    [[nodiscard]] QPointF scene_to_screen(const QPointF& scene_position) const;
    [[nodiscard]] SelectionHit hit_test_selection(
        const polivex::core::RectangleEntity& rectangle, const QPointF& screen_position) const;
    void apply_hover_cursor(const QPointF& screen_position);
    void draw_grid(QPainter& painter) const;
    void draw_axes(QPainter& painter) const;
    void draw_navigation_cube(QPainter& painter) const;
    void draw_rectangles(QPainter& painter) const;
    void draw_selection_overlay(QPainter& painter) const;
    void draw_selection_badge(QPainter& painter, const polivex::core::RectangleEntity& rectangle) const;
    void draw_snap_guides(QPainter& painter) const;
    [[nodiscard]] QRect navigation_cube_rect() const;
    [[nodiscard]] std::optional<polivex::core::EntityId> rectangle_at(const QPointF& screen_position) const;
    [[nodiscard]] std::optional<polivex::core::RectangleEntity> selected_rectangle() const;
    [[nodiscard]] bool is_selected(polivex::core::EntityId entity_id) const;
    [[nodiscard]] QString selection_badge_text(const polivex::core::RectangleEntity& rectangle) const;

    polivex::app::ViewportState state_;
    bool is_panning_ = false;
    bool is_creating_rectangle_ = false;
    bool is_pending_selected_drag_ = false;
    bool pending_toggle_selection_mode_ = false;
    bool is_moving_selected_rectangle_ = false;
    bool is_resizing_selected_rectangle_ = false;
    bool is_rotating_selected_rectangle_ = false;
    bool is_adjusting_corner_radius_ = false;
    InteractionMode interaction_mode_ = InteractionMode::None;
    SelectionHandleMode selection_handle_mode_ = SelectionHandleMode::Scale;
    ResizeHandle active_resize_handle_ = ResizeHandle::None;
    CornerRadiusHandle active_corner_radius_handle_ = CornerRadiusHandle::None;
    QPoint last_mouse_position_;
    QPoint pending_press_position_;
    polivex::core::Point2D rectangle_start_;
    polivex::core::Point2D rectangle_current_;
    polivex::core::Point2D last_move_delta_;
    QPointF interaction_press_scene_;
    polivex::core::Rectangle2D interaction_initial_bounds_;
    double interaction_initial_rotation_ = 0.0;
    double interaction_initial_corner_radius_ = 0.0;
    double interaction_initial_mouse_angle_ = 0.0;
    std::vector<polivex::core::RectangleEntity> rectangles_;
    std::vector<polivex::core::EntityId> selected_entity_ids_;
    QColor background_color_ {"#20252b"};
    bool background_transparent_ = false;
    bool grid_visible_ = true;
    double grid_spacing_ = 1.0;
    std::vector<QLineF> snap_guides_;
};

}  // namespace polivex::ui
