#pragma once

#include <optional>
#include <span>
#include <vector>

#include <QWidget>

#include "app/workspace.h"
#include "core/point_2d.h"
#include "core/rectangle_entity.h"

class QMouseEvent;
class QPaintEvent;
class QPainter;
class QWheelEvent;

namespace polivex::ui {

class ViewportWidget : public QWidget {
    Q_OBJECT

public:
    explicit ViewportWidget(QWidget* parent = nullptr);

    void set_viewport_state(const polivex::app::ViewportState& state);
    void set_rectangles(std::span<const polivex::core::RectangleEntity> rectangles);
    void set_selected_entity_id(std::optional<polivex::core::EntityId> entity_id);

signals:
    void pan_requested(double delta_x, double delta_y);
    void zoom_requested(double factor);
    void camera_preset_requested(polivex::app::CameraPreset preset);
    void rectangle_creation_requested(
        const polivex::core::Point2D& first_corner, const polivex::core::Point2D& second_corner);
    void rectangle_selected(std::optional<polivex::core::EntityId> entity_id);
    void selected_rectangle_move_requested(const polivex::core::Point2D& delta);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    [[nodiscard]] QPointF screen_to_scene(const QPointF& screen_position) const;
    [[nodiscard]] QPointF scene_to_screen(const QPointF& scene_position) const;
    void draw_grid(QPainter& painter) const;
    void draw_axes(QPainter& painter) const;
    void draw_navigation_cube(QPainter& painter) const;
    void draw_rectangles(QPainter& painter) const;
    [[nodiscard]] QRect navigation_cube_rect() const;
    [[nodiscard]] std::optional<polivex::core::EntityId> rectangle_at(const QPointF& screen_position) const;

    polivex::app::ViewportState state_;
    bool is_panning_ = false;
    bool is_creating_rectangle_ = false;
    bool is_moving_selected_rectangle_ = false;
    QPoint last_mouse_position_;
    polivex::core::Point2D rectangle_start_;
    polivex::core::Point2D rectangle_current_;
    polivex::core::Point2D last_scene_position_;
    std::vector<polivex::core::RectangleEntity> rectangles_;
    std::optional<polivex::core::EntityId> selected_entity_id_;
};

}  // namespace polivex::ui
