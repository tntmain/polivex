#include "ui/main_window.h"

#include <algorithm>

#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QColorDialog>

#include "app/application_session.h"
#include "ui/viewport_widget.h"

namespace polivex {
namespace ui {

MainWindow::MainWindow(polivex::app::ApplicationSession& session, QWidget* parent)
    : QMainWindow(parent)
    , session_(session)
{
    resize(1280, 800);

    create_actions();
    create_layout();
    create_status_bar();
    retranslate_ui();
}

void MainWindow::handle_new_document()
{
    session_.create_new_document();
    session_.select_rectangle(std::nullopt);
    refresh_window_state();
    refresh_viewport();
    refresh_inspector();
}

void MainWindow::create_actions()
{
    file_menu_ = menuBar()->addMenu(QString());
    new_action_ = file_menu_->addAction(QString());
    new_action_->setShortcut(QKeySequence::New);
    connect(new_action_, &QAction::triggered, this, &MainWindow::handle_new_document);

    file_menu_->addSeparator();
    exit_action_ = file_menu_->addAction(QString());
    exit_action_->setShortcut(QKeySequence::Quit);
    connect(exit_action_, &QAction::triggered, this, &QWidget::close);

    view_menu_ = menuBar()->addMenu(QString());
    language_menu_ = view_menu_->addMenu(QString());
    workspace_menu_ = view_menu_->addMenu(QString());
    camera_menu_ = view_menu_->addMenu(QString());

    auto* language_group = new QActionGroup(this);
    language_group->setExclusive(true);

    english_action_ = language_menu_->addAction(QString());
    english_action_->setCheckable(true);
    english_action_->setData("en");
    language_group->addAction(english_action_);

    russian_action_ = language_menu_->addAction(QString());
    russian_action_->setCheckable(true);
    russian_action_->setData("ru");
    language_group->addAction(russian_action_);

    connect(language_group, &QActionGroup::triggered, this, [this](QAction* action) {
        emit language_requested(action->data().toString());
    });

    auto* workspace_group = new QActionGroup(this);
    workspace_group->setExclusive(true);
    vector_workspace_action_ = workspace_menu_->addAction(QString());
    vector_workspace_action_->setCheckable(true);
    vector_workspace_action_->setData(static_cast<int>(polivex::app::Workspace::Vector));
    workspace_group->addAction(vector_workspace_action_);
    sketch_workspace_action_ = workspace_menu_->addAction(QString());
    sketch_workspace_action_->setCheckable(true);
    sketch_workspace_action_->setData(static_cast<int>(polivex::app::Workspace::Sketch));
    workspace_group->addAction(sketch_workspace_action_);
    model_workspace_action_ = workspace_menu_->addAction(QString());
    model_workspace_action_->setCheckable(true);
    model_workspace_action_->setData(static_cast<int>(polivex::app::Workspace::Model));
    workspace_group->addAction(model_workspace_action_);
    connect(workspace_group, &QActionGroup::triggered, this, &MainWindow::handle_workspace_selected);

    auto* camera_group = new QActionGroup(this);
    camera_group->setExclusive(true);
    top_camera_action_ = camera_menu_->addAction(QString());
    top_camera_action_->setCheckable(true);
    top_camera_action_->setData(static_cast<int>(polivex::app::CameraPreset::Top));
    camera_group->addAction(top_camera_action_);
    front_camera_action_ = camera_menu_->addAction(QString());
    front_camera_action_->setCheckable(true);
    front_camera_action_->setData(static_cast<int>(polivex::app::CameraPreset::Front));
    camera_group->addAction(front_camera_action_);
    right_camera_action_ = camera_menu_->addAction(QString());
    right_camera_action_->setCheckable(true);
    right_camera_action_->setData(static_cast<int>(polivex::app::CameraPreset::Right));
    camera_group->addAction(right_camera_action_);
    isometric_camera_action_ = camera_menu_->addAction(QString());
    isometric_camera_action_->setCheckable(true);
    isometric_camera_action_->setData(static_cast<int>(polivex::app::CameraPreset::Isometric));
    camera_group->addAction(isometric_camera_action_);
    connect(camera_group, &QActionGroup::triggered, this, &MainWindow::handle_camera_preset_selected);

    file_toolbar_ = addToolBar(QString());
    file_toolbar_->addAction(new_action_);
    rectangle_tool_action_ = file_toolbar_->addAction(QString());
    rectangle_tool_action_->setCheckable(true);
    connect(rectangle_tool_action_, &QAction::triggered, this, &MainWindow::handle_rectangle_tool_selected);
}

void MainWindow::create_layout()
{
    viewport_ = new ViewportWidget(this);
    setCentralWidget(viewport_);
    connect(viewport_, &ViewportWidget::pan_requested, this, [this](double delta_x, double delta_y) {
        auto& state = session_.viewport_state();
        state.pan_x += delta_x;
        state.pan_y += delta_y;
        refresh_viewport();
    });
    connect(viewport_, &ViewportWidget::zoom_requested, this, [this](double factor) {
        auto& state = session_.viewport_state();
        state.zoom = std::clamp(state.zoom * factor, 0.1, 20.0);
        refresh_viewport();
    });
    connect(viewport_, &ViewportWidget::camera_preset_requested, this, [this](polivex::app::CameraPreset preset) {
        session_.set_camera_preset(preset);
        refresh_viewport();
    });
    connect(viewport_, &ViewportWidget::rectangle_creation_requested, this,
        [this](const polivex::core::Point2D& first_corner, const polivex::core::Point2D& second_corner) {
            if (session_.create_rectangle(first_corner, second_corner)) {
                refresh_window_state();
                refresh_viewport();
            }
        });
    connect(viewport_, &ViewportWidget::rectangle_selected, this,
        [this](std::optional<polivex::core::EntityId> entity_id) {
            session_.select_rectangle(entity_id);
            refresh_inspector();
            refresh_viewport();
        });
    connect(viewport_, &ViewportWidget::selected_rectangle_move_requested, this,
        [this](const polivex::core::Point2D& delta) {
            if (session_.move_selected_rectangle(delta)) {
                refresh_window_state();
                refresh_viewport();
            }
        });

    browser_dock_ = new QDockWidget(this);
    browser_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    document_label_ = new QLabel(browser_dock_);
    document_label_->setMargin(12);
    browser_dock_->setWidget(document_label_);
    addDockWidget(Qt::LeftDockWidgetArea, browser_dock_);

    inspector_dock_ = new QDockWidget(this);
    inspector_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* inspector_content = new QWidget(inspector_dock_);
    auto* inspector_layout = new QVBoxLayout(inspector_content);
    inspector_layout->setContentsMargins(12, 12, 12, 12);
    inspector_layout->setSpacing(8);

    inspector_label_ = new QLabel(inspector_content);
    inspector_label_->setWordWrap(true);
    color_button_ = new QPushButton(inspector_content);
    opacity_slider_ = new QSlider(Qt::Horizontal, inspector_content);
    opacity_slider_->setRange(0, 255);
    inspector_layout->addWidget(inspector_label_);
    inspector_layout->addWidget(color_button_);
    inspector_layout->addWidget(opacity_slider_);
    inspector_layout->addStretch();
    inspector_dock_->setWidget(inspector_content);
    connect(color_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_color_change);
    connect(opacity_slider_, &QSlider::valueChanged, this, &MainWindow::handle_selected_opacity_change);
    addDockWidget(Qt::RightDockWidgetArea, inspector_dock_);
}

void MainWindow::create_status_bar()
{
    statusBar();
}

void MainWindow::handle_workspace_selected(QAction* action)
{
    session_.set_workspace(static_cast<polivex::app::Workspace>(action->data().toInt()));
    refresh_viewport();
}

void MainWindow::handle_camera_preset_selected(QAction* action)
{
    session_.set_camera_preset(static_cast<polivex::app::CameraPreset>(action->data().toInt()));
    refresh_viewport();
}

void MainWindow::handle_rectangle_tool_selected()
{
    const auto next_tool = rectangle_tool_action_->isChecked()
        ? polivex::app::ActiveTool::Rectangle
        : polivex::app::ActiveTool::Select;
    session_.set_active_tool(next_tool);
    refresh_viewport();
}

void MainWindow::handle_selected_color_change()
{
    const auto entity_id = session_.selected_entity_id();
    if (!entity_id.has_value()) {
        return;
    }

    const auto* rectangle = session_.active_document().rectangle(*entity_id);
    if (rectangle == nullptr || !rectangle->vector_style.has_value()) {
        return;
    }

    const auto style = *rectangle->vector_style;
    const auto color = QColorDialog::getColor(QColor(style.red, style.green, style.blue), this);
    if (!color.isValid()) {
        return;
    }

    auto updated_style = style;
    updated_style.red = static_cast<std::uint8_t>(color.red());
    updated_style.green = static_cast<std::uint8_t>(color.green());
    updated_style.blue = static_cast<std::uint8_t>(color.blue());
    if (session_.set_selected_vector_style(updated_style)) {
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_opacity_change(int opacity)
{
    const auto entity_id = session_.selected_entity_id();
    if (!entity_id.has_value()) {
        return;
    }

    const auto* rectangle = session_.active_document().rectangle(*entity_id);
    if (rectangle == nullptr || !rectangle->vector_style.has_value()) {
        return;
    }

    auto style = *rectangle->vector_style;
    style.opacity = static_cast<std::uint8_t>(opacity);
    if (session_.set_selected_vector_style(style)) {
        refresh_viewport();
    }
}

void MainWindow::refresh_window_state()
{
    const auto& document = session_.active_document();
    const auto document_name = display_document_name();
    const auto rectangles = document.rectangles();
    const auto vector_count = std::count_if(rectangles.begin(), rectangles.end(), [](const auto& rectangle) {
        return rectangle.kind == polivex::core::RectangleKind::Vector;
    });
    const auto sketch_count = std::count_if(rectangles.begin(), rectangles.end(), [](const auto& rectangle) {
        return rectangle.kind == polivex::core::RectangleKind::Sketch;
    });

    setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Polivex - %1").arg(document_name));
    document_label_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Active document: %1\nModified: %2\n\nVector rectangles: %3\nSketch rectangles: %4")
            .arg(document_name,
                document.is_dirty()
                    ? QCoreApplication::translate("polivex::ui::MainWindow", "yes")
                    : QCoreApplication::translate("polivex::ui::MainWindow", "no"))
            .arg(vector_count)
            .arg(sketch_count));
}

void MainWindow::set_current_language(const QString& locale)
{
    english_action_->setChecked(locale == "en");
    russian_action_->setChecked(locale == "ru");
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void MainWindow::retranslate_ui()
{
    file_menu_->setTitle(QCoreApplication::translate("polivex::ui::MainWindow", "&File"));
    new_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "&New"));
    exit_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "E&xit"));
    view_menu_->setTitle(QCoreApplication::translate("polivex::ui::MainWindow", "&View"));
    language_menu_->setTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Language"));
    workspace_menu_->setTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Workspace"));
    camera_menu_->setTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Camera"));
    english_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "English"));
    russian_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Russian"));
    vector_workspace_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Vector"));
    sketch_workspace_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Sketch"));
    model_workspace_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Model"));
    top_camera_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Top"));
    front_camera_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Front"));
    right_camera_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Right"));
    isometric_camera_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Isometric"));
    rectangle_tool_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Rectangle"));
    file_toolbar_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "File"));
    browser_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Browser"));
    inspector_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Inspector"));
    statusBar()->showMessage(QCoreApplication::translate("polivex::ui::MainWindow", "Ready"));
    refresh_window_state();
    refresh_viewport();
    refresh_inspector();
}

void MainWindow::refresh_viewport()
{
    const auto& state = session_.viewport_state();
    viewport_->set_viewport_state(state);
    viewport_->set_rectangles(session_.active_document().rectangles());
    viewport_->set_selected_entity_id(session_.selected_entity_id());

    vector_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Vector);
    sketch_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Sketch);
    model_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Model);
    top_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Top);
    front_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Front);
    right_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Right);
    isometric_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Isometric);
    rectangle_tool_action_->setChecked(state.active_tool == polivex::app::ActiveTool::Rectangle);
}

void MainWindow::refresh_inspector()
{
    const auto entity_id = session_.selected_entity_id();
    const auto* rectangle = entity_id.has_value() ? session_.active_document().rectangle(*entity_id) : nullptr;

    if (rectangle == nullptr) {
        inspector_label_->setText(QCoreApplication::translate(
            "polivex::ui::MainWindow", "Choose Rectangle, then drag on the canvas to create an object."));
        color_button_->setVisible(false);
        opacity_slider_->setVisible(false);
        return;
    }

    if (rectangle->kind == polivex::core::RectangleKind::Sketch) {
        inspector_label_->setText(QCoreApplication::translate(
            "polivex::ui::MainWindow", "Sketch rectangle on the XY plane. Dimensions and snapping come next."));
        color_button_->setVisible(false);
        opacity_slider_->setVisible(false);
        return;
    }

    const auto style = rectangle->vector_style.value_or(polivex::core::VectorStyle {});
    inspector_label_->setText(QCoreApplication::translate(
        "polivex::ui::MainWindow", "Selected vector rectangle"));
    color_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Fill color"));
    color_button_->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                     .arg(style.red)
                                     .arg(style.green)
                                     .arg(style.blue));
    opacity_slider_->blockSignals(true);
    opacity_slider_->setValue(style.opacity);
    opacity_slider_->blockSignals(false);
    color_button_->setVisible(true);
    opacity_slider_->setVisible(true);
}

QString MainWindow::display_document_name() const
{
    const auto& document_name = session_.active_document().name();

    if (document_name == "Untitled") {
        return QCoreApplication::translate("polivex::ui::MainWindow", "Untitled");
    }

    return QString::fromStdString(document_name);
}

}  // namespace ui
}  // namespace polivex
