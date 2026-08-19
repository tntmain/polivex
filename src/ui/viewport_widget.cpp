#include "ui/viewport_widget.h"

#include <algorithm>
#include <cmath>

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

namespace polivex::ui {

namespace {

constexpr double kBasePixelsPerUnit = 48.0;
constexpr double kMinimumZoom = 0.1;
constexpr double kMaximumZoom = 20.0;

}  // namespace

ViewportWidget::ViewportWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(360, 240);
}

void ViewportWidget::set_viewport_state(const polivex::app::ViewportState& state)
{
    state_ = state;
    update();
}

void ViewportWidget::set_rectangles(std::span<const polivex::core::RectangleEntity> rectangles)
{
    rectangles_.assign(rectangles.begin(), rectangles.end());
    update();
}

void ViewportWidget::set_selected_entity_id(std::optional<polivex::core::EntityId> entity_id)
{
    selected_entity_id_ = entity_id;
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

    if (is_moving_selected_rectangle_) {
        const auto current_position = screen_to_scene(event->position());
        const polivex::core::Point2D delta {
            current_position.x() - last_scene_position_.x,
            current_position.y() - last_scene_position_.y,
        };
        last_scene_position_ = {current_position.x(), current_position.y()};
        emit selected_rectangle_move_requested(delta);
        return;
    }

    if (!is_panning_) {
        return;
    }

    const auto delta = event->pos() - last_mouse_position_;
    last_mouse_position_ = event->pos();
    emit pan_requested(delta.x(), delta.y());
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
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
        const auto selected = rectangle_at(event->position());
        emit rectangle_selected(selected);

        if (selected.has_value()) {
            const auto scene_position = screen_to_scene(event->position());
            last_scene_position_ = {scene_position.x(), scene_position.y()};
            is_moving_selected_rectangle_ = true;
            setCursor(Qt::SizeAllCursor);
        }
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
    if ((event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) && is_panning_) {
        is_panning_ = false;
        unsetCursor();
    }

    if (event->button() == Qt::LeftButton && is_creating_rectangle_) {
        is_creating_rectangle_ = false;
        unsetCursor();

        if (std::abs(rectangle_current_.x - rectangle_start_.x) > 0.001
            && std::abs(rectangle_current_.y - rectangle_start_.y) > 0.001) {
            emit rectangle_creation_requested(rectangle_start_, rectangle_current_);
        }

        update();
    }

    if (event->button() == Qt::LeftButton && is_moving_selected_rectangle_) {
        is_moving_selected_rectangle_ = false;
        unsetCursor();
    }
}

void ViewportWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#20252b"));
    painter.setRenderHint(QPainter::Antialiasing, true);

    draw_grid(painter);
    draw_axes(painter);
    draw_rectangles(painter);
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

void ViewportWidget::draw_grid(QPainter& painter) const
{
    if (state_.camera_preset != polivex::app::CameraPreset::Top) {
        return;
    }

    const auto scale = kBasePixelsPerUnit * state_.zoom;
    const auto spacing = std::max(12.0, scale);
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
    if (state_.camera_preset != polivex::app::CameraPreset::Top) {
        return;
    }

    const auto draw_rectangle = [this, &painter](
                                    const polivex::core::Rectangle2D& bounds, QColor border, QColor fill, bool selected) {
        const auto top_left = scene_to_screen({bounds.minimum.x, bounds.maximum.y});
        const auto bottom_right = scene_to_screen({bounds.maximum.x, bounds.minimum.y});
        painter.setPen(QPen(selected ? QColor("#f3bd5b") : border, selected ? 3.0 : 2.0));
        painter.setBrush(fill);
        painter.drawRect(QRectF(top_left, bottom_right).normalized());
    };

    for (const auto& rectangle : rectangles_) {
        if (rectangle.kind == polivex::core::RectangleKind::Vector) {
            const auto style = rectangle.vector_style.value_or(polivex::core::VectorStyle {});
            draw_rectangle(rectangle.bounds, QColor(style.red, style.green, style.blue),
                QColor(style.red, style.green, style.blue, style.opacity), rectangle.id == selected_entity_id_);
        } else {
            draw_rectangle(rectangle.bounds, QColor("#65c466"), QColor(101, 196, 102, 25), rectangle.id == selected_entity_id_);
        }
    }

    if (is_creating_rectangle_) {
        draw_rectangle({rectangle_start_, rectangle_current_}, QColor("#f3bd5b"), QColor(243, 189, 91, 35), false);
    }
}

std::optional<polivex::core::EntityId> ViewportWidget::rectangle_at(const QPointF& screen_position) const
{
    if (state_.camera_preset != polivex::app::CameraPreset::Top) {
        return std::nullopt;
    }

    const auto scene_position = screen_to_scene(screen_position);
    for (auto iterator = rectangles_.rbegin(); iterator != rectangles_.rend(); ++iterator) {
        const auto& bounds = iterator->bounds;
        if (scene_position.x() >= bounds.minimum.x && scene_position.x() <= bounds.maximum.x
            && scene_position.y() >= bounds.minimum.y && scene_position.y() <= bounds.maximum.y) {
            return iterator->id;
        }
    }

    return std::nullopt;
}

QRect ViewportWidget::navigation_cube_rect() const
{
    constexpr int cube_size = 72;
    constexpr int margin = 18;
    return {width() - cube_size - margin, margin, cube_size, cube_size};
}

}  // namespace polivex::ui
