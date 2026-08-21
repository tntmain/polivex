#pragma once

#include <QMainWindow>
#include <QString>

namespace polivex::app {
class ApplicationSession;
}

namespace polivex::core {
struct Point2D;
struct Rectangle2D;
}

class QLabel;
class QAction;
class QDockWidget;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QToolBar;
class QActionGroup;
class QPushButton;
class QSlider;
class QDoubleSpinBox;

namespace polivex::ui {
class ViewportWidget;
}

namespace polivex::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(polivex::app::ApplicationSession& session, QWidget* parent = nullptr);
    void set_current_language(const QString& locale);

signals:
    void language_requested(const QString& locale);

private slots:
    void handle_new_document();
    void handle_workspace_selected(QAction* action);
    void handle_camera_preset_selected(QAction* action);
    void handle_rectangle_tool_selected();
    void handle_selected_color_change();
    void handle_selected_opacity_change(int opacity);
    void handle_selected_geometry_change();
    void handle_selected_move_requested(const polivex::core::Point2D& delta);
    void handle_selected_resize_requested(const polivex::core::Rectangle2D& bounds);
    void handle_selected_rotation_requested(double rotation_degrees);
    void handle_selected_corner_radius_requested(double radius);
    void handle_selected_bring_to_front();
    void handle_selected_send_to_back();
    void handle_selected_move_up();
    void handle_selected_move_down();
    void handle_selected_align_left();
    void handle_selected_align_right();
    void handle_selected_align_horizontal_center();
    void handle_selected_align_top();
    void handle_selected_align_bottom();
    void handle_selected_align_vertical_middle();
    void handle_selected_distribute_horizontally();
    void handle_selected_distribute_vertically();
    void handle_layers_item_changed(QListWidgetItem* item);
    void handle_layers_selection_changed();
    void handle_grid_visibility_toggled(bool visible);
    void handle_grid_spacing_change();
    void handle_background_transparency_toggled(bool transparent);
    void handle_background_color_change();

private:
    void changeEvent(QEvent* event) override;
    void create_actions();
    void create_layout();
    void create_status_bar();
    void refresh_window_state();
    void retranslate_ui();
    void refresh_viewport();
    void refresh_inspector();
    void refresh_layers();
    [[nodiscard]] QString display_document_name() const;

    polivex::app::ApplicationSession& session_;
    QLabel* document_label_ = nullptr;
    QLabel* layers_label_ = nullptr;
    QLabel* inspector_label_ = nullptr;
    QLabel* geometry_label_ = nullptr;
    QListWidget* layers_list_ = nullptr;
    QPushButton* layer_top_button_ = nullptr;
    QPushButton* layer_up_button_ = nullptr;
    QPushButton* layer_down_button_ = nullptr;
    QPushButton* layer_bottom_button_ = nullptr;
    QDoubleSpinBox* x_spinbox_ = nullptr;
    QDoubleSpinBox* y_spinbox_ = nullptr;
    QDoubleSpinBox* width_spinbox_ = nullptr;
    QDoubleSpinBox* height_spinbox_ = nullptr;
    QDoubleSpinBox* rotation_spinbox_ = nullptr;
    QDoubleSpinBox* corner_radius_spinbox_ = nullptr;
    QPushButton* color_button_ = nullptr;
    QSlider* opacity_slider_ = nullptr;
    QMenu* file_menu_ = nullptr;
    QMenu* view_menu_ = nullptr;
    QMenu* layer_menu_ = nullptr;
    QMenu* language_menu_ = nullptr;
    QMenu* workspace_menu_ = nullptr;
    QMenu* camera_menu_ = nullptr;
    QToolBar* file_toolbar_ = nullptr;
    QAction* new_action_ = nullptr;
    QAction* exit_action_ = nullptr;
    QAction* english_action_ = nullptr;
    QAction* russian_action_ = nullptr;
    QAction* vector_workspace_action_ = nullptr;
    QAction* sketch_workspace_action_ = nullptr;
    QAction* model_workspace_action_ = nullptr;
    QAction* top_camera_action_ = nullptr;
    QAction* front_camera_action_ = nullptr;
    QAction* right_camera_action_ = nullptr;
    QAction* isometric_camera_action_ = nullptr;
    QAction* rectangle_tool_action_ = nullptr;
    QAction* grid_visible_action_ = nullptr;
    QAction* grid_spacing_action_ = nullptr;
    QAction* background_transparent_action_ = nullptr;
    QAction* background_color_action_ = nullptr;
    QAction* bring_to_front_action_ = nullptr;
    QAction* send_to_back_action_ = nullptr;
    QAction* move_up_action_ = nullptr;
    QAction* move_down_action_ = nullptr;
    QAction* align_left_action_ = nullptr;
    QAction* align_right_action_ = nullptr;
    QAction* align_horizontal_center_action_ = nullptr;
    QAction* align_top_action_ = nullptr;
    QAction* align_bottom_action_ = nullptr;
    QAction* align_vertical_middle_action_ = nullptr;
    QAction* distribute_horizontally_action_ = nullptr;
    QAction* distribute_vertically_action_ = nullptr;
    QDockWidget* browser_dock_ = nullptr;
    QDockWidget* layers_dock_ = nullptr;
    QDockWidget* inspector_dock_ = nullptr;
    ViewportWidget* viewport_ = nullptr;
};

}  // namespace polivex::ui
