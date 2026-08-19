#pragma once

#include <QMainWindow>
#include <QString>

namespace polivex::app {
class ApplicationSession;
}

class QLabel;
class QAction;
class QDockWidget;
class QMenu;
class QToolBar;
class QActionGroup;
class QPushButton;
class QSlider;

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

private:
    void changeEvent(QEvent* event) override;
    void create_actions();
    void create_layout();
    void create_status_bar();
    void refresh_window_state();
    void retranslate_ui();
    void refresh_viewport();
    void refresh_inspector();
    [[nodiscard]] QString display_document_name() const;

    polivex::app::ApplicationSession& session_;
    QLabel* document_label_ = nullptr;
    QLabel* inspector_label_ = nullptr;
    QPushButton* color_button_ = nullptr;
    QSlider* opacity_slider_ = nullptr;
    QMenu* file_menu_ = nullptr;
    QMenu* view_menu_ = nullptr;
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
    QDockWidget* browser_dock_ = nullptr;
    QDockWidget* inspector_dock_ = nullptr;
    ViewportWidget* viewport_ = nullptr;
};

}  // namespace polivex::ui
