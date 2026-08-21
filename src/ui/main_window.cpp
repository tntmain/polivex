#include "ui/main_window.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include <QAction>
#include <QActionGroup>
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QColorDialog>
#include <QDockWidget>
#include <QEvent>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSignalBlocker>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

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
    session_.clear_selection();
    refresh_window_state();
    refresh_viewport();
    refresh_inspector();
    refresh_layers();
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
    layer_menu_ = view_menu_->addMenu(QString());

    grid_visible_action_ = view_menu_->addAction(QString());
    grid_visible_action_->setCheckable(true);
    grid_visible_action_->setChecked(true);
    connect(grid_visible_action_, &QAction::toggled, this, &MainWindow::handle_grid_visibility_toggled);

    grid_spacing_action_ = view_menu_->addAction(QString());
    connect(grid_spacing_action_, &QAction::triggered, this, &MainWindow::handle_grid_spacing_change);

    background_transparent_action_ = view_menu_->addAction(QString());
    background_transparent_action_->setCheckable(true);
    connect(background_transparent_action_, &QAction::toggled, this, &MainWindow::handle_background_transparency_toggled);

    background_color_action_ = view_menu_->addAction(QString());
    connect(background_color_action_, &QAction::triggered, this, &MainWindow::handle_background_color_change);

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

    bring_to_front_action_ = layer_menu_->addAction(QString());
    connect(bring_to_front_action_, &QAction::triggered, this, &MainWindow::handle_selected_bring_to_front);
    send_to_back_action_ = layer_menu_->addAction(QString());
    connect(send_to_back_action_, &QAction::triggered, this, &MainWindow::handle_selected_send_to_back);
    move_up_action_ = layer_menu_->addAction(QString());
    connect(move_up_action_, &QAction::triggered, this, &MainWindow::handle_selected_move_up);
    move_down_action_ = layer_menu_->addAction(QString());
    connect(move_down_action_, &QAction::triggered, this, &MainWindow::handle_selected_move_down);
    layer_menu_->addSeparator();
    align_left_action_ = layer_menu_->addAction(QString());
    connect(align_left_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_left);
    align_right_action_ = layer_menu_->addAction(QString());
    connect(align_right_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_right);
    align_horizontal_center_action_ = layer_menu_->addAction(QString());
    connect(align_horizontal_center_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_horizontal_center);
    align_top_action_ = layer_menu_->addAction(QString());
    connect(align_top_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_top);
    align_bottom_action_ = layer_menu_->addAction(QString());
    connect(align_bottom_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_bottom);
    align_vertical_middle_action_ = layer_menu_->addAction(QString());
    connect(align_vertical_middle_action_, &QAction::triggered, this, &MainWindow::handle_selected_align_vertical_middle);
    distribute_horizontally_action_ = layer_menu_->addAction(QString());
    connect(distribute_horizontally_action_, &QAction::triggered, this, &MainWindow::handle_selected_distribute_horizontally);
    distribute_vertically_action_ = layer_menu_->addAction(QString());
    connect(distribute_vertically_action_, &QAction::triggered, this, &MainWindow::handle_selected_distribute_vertically);

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
                refresh_layers();
            }
        });
    connect(viewport_, &ViewportWidget::rectangle_selected, this, [](std::optional<polivex::core::EntityId>) {});
    connect(viewport_, &ViewportWidget::selection_changed, this,
        [this](const std::vector<polivex::core::EntityId>& entity_ids) {
            session_.set_selected_rectangles(entity_ids);
            refresh_inspector();
            refresh_viewport();
            refresh_layers();
        });
    connect(viewport_, &ViewportWidget::selected_rectangle_move_requested, this, &MainWindow::handle_selected_move_requested);
    connect(viewport_, &ViewportWidget::selected_rectangle_resize_requested, this, &MainWindow::handle_selected_resize_requested);
    connect(viewport_, &ViewportWidget::selected_rectangle_rotation_requested, this, &MainWindow::handle_selected_rotation_requested);
    connect(viewport_, &ViewportWidget::selected_rectangle_corner_radius_requested, this,
        &MainWindow::handle_selected_corner_radius_requested);

    browser_dock_ = new QDockWidget(this);
    browser_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    document_label_ = new QLabel(browser_dock_);
    document_label_->setMargin(12);
    browser_dock_->setWidget(document_label_);
    addDockWidget(Qt::LeftDockWidgetArea, browser_dock_);

    layers_dock_ = new QDockWidget(this);
    layers_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* layers_content = new QWidget(layers_dock_);
    auto* layers_layout = new QVBoxLayout(layers_content);
    layers_layout->setContentsMargins(12, 12, 12, 12);
    layers_layout->setSpacing(8);
    layers_label_ = new QLabel(layers_content);
    layers_label_->setWordWrap(true);
    layers_list_ = new QListWidget(layers_content);
    layers_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layers_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layers_list_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    layers_list_->setAlternatingRowColors(true);
    auto* layer_buttons = new QHBoxLayout();
    layer_top_button_ = new QPushButton(layers_content);
    layer_up_button_ = new QPushButton(layers_content);
    layer_down_button_ = new QPushButton(layers_content);
    layer_bottom_button_ = new QPushButton(layers_content);
    layer_buttons->addWidget(layer_top_button_);
    layer_buttons->addWidget(layer_up_button_);
    layer_buttons->addWidget(layer_down_button_);
    layer_buttons->addWidget(layer_bottom_button_);
    layers_layout->addWidget(layers_label_);
    layers_layout->addWidget(layers_list_);
    layers_layout->addLayout(layer_buttons);
    layers_dock_->setWidget(layers_content);
    layers_dock_->setMinimumWidth(280);
    connect(layers_list_, &QListWidget::itemChanged, this, &MainWindow::handle_layers_item_changed);
    connect(layers_list_, &QListWidget::itemSelectionChanged, this, &MainWindow::handle_layers_selection_changed);
    connect(layer_top_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_bring_to_front);
    connect(layer_up_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_move_up);
    connect(layer_down_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_move_down);
    connect(layer_bottom_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_send_to_back);
    addDockWidget(Qt::LeftDockWidgetArea, layers_dock_);
    splitDockWidget(browser_dock_, layers_dock_, Qt::Vertical);

    inspector_dock_ = new QDockWidget(this);
    inspector_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* inspector_content = new QWidget(inspector_dock_);
    auto* inspector_layout = new QVBoxLayout(inspector_content);
    inspector_layout->setContentsMargins(12, 12, 12, 12);
    inspector_layout->setSpacing(8);
    inspector_content->setMinimumWidth(340);
    inspector_dock_->setMinimumWidth(360);

    inspector_label_ = new QLabel(inspector_content);
    inspector_label_->setWordWrap(true);
    geometry_label_ = new QLabel(inspector_content);
    geometry_label_->setText(QString());
    auto* form_layout = new QFormLayout();
    form_layout->setLabelAlignment(Qt::AlignLeft);
    form_layout->setFormAlignment(Qt::AlignTop);
    form_layout->setHorizontalSpacing(10);
    form_layout->setVerticalSpacing(6);

    auto create_spinbox = [inspector_content]() {
        auto* box = new QDoubleSpinBox(inspector_content);
        box->setRange(-100000.0, 100000.0);
        box->setDecimals(2);
        box->setSingleStep(0.5);
        box->setKeyboardTracking(false);
        return box;
    };

    x_spinbox_ = create_spinbox();
    y_spinbox_ = create_spinbox();
    width_spinbox_ = create_spinbox();
    width_spinbox_->setMinimum(0.01);
    height_spinbox_ = create_spinbox();
    height_spinbox_->setMinimum(0.01);
    rotation_spinbox_ = create_spinbox();
    rotation_spinbox_->setRange(-3600.0, 3600.0);
    corner_radius_spinbox_ = create_spinbox();
    corner_radius_spinbox_->setMinimum(0.0);
    color_button_ = new QPushButton(inspector_content);
    opacity_slider_ = new QSlider(Qt::Horizontal, inspector_content);
    opacity_slider_->setRange(0, 255);

    form_layout->addRow(QStringLiteral("X"), x_spinbox_);
    form_layout->addRow(QStringLiteral("Y"), y_spinbox_);
    form_layout->addRow(QStringLiteral("Width"), width_spinbox_);
    form_layout->addRow(QStringLiteral("Height"), height_spinbox_);
    form_layout->addRow(QStringLiteral("Rotation"), rotation_spinbox_);
    form_layout->addRow(QStringLiteral("Corner radius"), corner_radius_spinbox_);
    form_layout->addRow(QStringLiteral("Fill color"), color_button_);
    form_layout->addRow(QStringLiteral("Opacity"), opacity_slider_);
    inspector_layout->addWidget(inspector_label_);
    inspector_layout->addWidget(geometry_label_);
    inspector_layout->addLayout(form_layout);
    inspector_layout->addStretch();
    inspector_dock_->setWidget(inspector_content);
    inspector_dock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    connect(color_button_, &QPushButton::clicked, this, &MainWindow::handle_selected_color_change);
    connect(opacity_slider_, &QSlider::valueChanged, this, &MainWindow::handle_selected_opacity_change);
    connect(x_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_geometry_change);
    connect(y_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_geometry_change);
    connect(width_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_geometry_change);
    connect(height_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_geometry_change);
    connect(rotation_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_rotation_requested);
    connect(corner_radius_spinbox_, &QDoubleSpinBox::valueChanged, this, &MainWindow::handle_selected_corner_radius_requested);
    addDockWidget(Qt::RightDockWidgetArea, inspector_dock_);
    splitDockWidget(layers_dock_, inspector_dock_, Qt::Vertical);

    refresh_layers();
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
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_geometry_change()
{
    const auto entity_id = session_.selected_entity_id();
    if (!entity_id.has_value()) {
        return;
    }

    const auto* rectangle = session_.active_document().rectangle(*entity_id);
    if (rectangle == nullptr) {
        return;
    }

    const polivex::core::Rectangle2D bounds {
        {x_spinbox_->value(), y_spinbox_->value()},
        {x_spinbox_->value() + width_spinbox_->value(), y_spinbox_->value() + height_spinbox_->value()},
    };

    if (session_.resize_selected_rectangle(bounds)) {
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_move_requested(const polivex::core::Point2D& delta)
{
    if (session_.move_selected_rectangle(delta)) {
        refresh_window_state();
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_resize_requested(const polivex::core::Rectangle2D& bounds)
{
    if (session_.resize_selected_rectangle(bounds)) {
        refresh_window_state();
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_rotation_requested(double rotation_degrees)
{
    if (session_.set_selected_rectangle_rotation(rotation_degrees)) {
        refresh_window_state();
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_corner_radius_requested(double radius)
{
    if (session_.set_selected_rectangle_corner_radius(radius)) {
        refresh_window_state();
        refresh_inspector();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_bring_to_front()
{
    if (session_.bring_selected_rectangle_to_front()) {
        refresh_window_state();
        refresh_viewport();
        refresh_layers();
    }
}

void MainWindow::handle_selected_send_to_back()
{
    if (session_.send_selected_rectangle_to_back()) {
        refresh_window_state();
        refresh_viewport();
        refresh_layers();
    }
}

void MainWindow::handle_selected_move_up()
{
    if (session_.move_selected_rectangle_up()) {
        refresh_window_state();
        refresh_viewport();
        refresh_layers();
    }
}

void MainWindow::handle_selected_move_down()
{
    if (session_.move_selected_rectangle_down()) {
        refresh_window_state();
        refresh_viewport();
        refresh_layers();
    }
}

void MainWindow::handle_selected_align_left()
{
    if (session_.align_selected_rectangles_left()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_align_right()
{
    if (session_.align_selected_rectangles_right()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_align_horizontal_center()
{
    if (session_.align_selected_rectangles_horizontal_center()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_align_top()
{
    if (session_.align_selected_rectangles_top()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_align_bottom()
{
    if (session_.align_selected_rectangles_bottom()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_align_vertical_middle()
{
    if (session_.align_selected_rectangles_vertical_middle()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_distribute_horizontally()
{
    if (session_.distribute_selected_rectangles_horizontally()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_selected_distribute_vertically()
{
    if (session_.distribute_selected_rectangles_vertically()) {
        refresh_window_state();
        refresh_viewport();
    }
}

void MainWindow::handle_layers_item_changed(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }

    const auto entity_id = static_cast<polivex::core::EntityId>(item->data(Qt::UserRole).toULongLong());
    const auto* rectangle = session_.active_document().rectangle(entity_id);
    if (rectangle == nullptr) {
        return;
    }

    const auto name = item->text().trimmed();
    const auto final_name = name.isEmpty() ? QString::fromStdString(rectangle->name) : name;
    if (final_name != item->text()) {
        const QSignalBlocker blocker(layers_list_);
        item->setText(final_name);
    }

    if (session_.active_document().rename_rectangle(entity_id, final_name.toStdString())) {
        refresh_window_state();
        refresh_inspector();
        refresh_viewport();
        refresh_layers();
    }
}

void MainWindow::handle_layers_selection_changed()
{
    if (layers_list_ == nullptr) {
        return;
    }

    std::vector<polivex::core::EntityId> selected_ids;
    selected_ids.reserve(static_cast<std::size_t>(layers_list_->selectedItems().size()));
    for (int row = 0; row < layers_list_->count(); ++row) {
        const auto* item = layers_list_->item(row);
        if (item != nullptr && item->isSelected()) {
            selected_ids.push_back(static_cast<polivex::core::EntityId>(item->data(Qt::UserRole).toULongLong()));
        }
    }

    session_.set_selected_rectangles(std::move(selected_ids));
    refresh_inspector();
    refresh_viewport();
}

void MainWindow::handle_grid_visibility_toggled(bool visible)
{
    auto& state = session_.viewport_state();
    state.grid_style = visible ? polivex::app::GridStyle::Visible : polivex::app::GridStyle::Hidden;
    refresh_viewport();
}

void MainWindow::handle_grid_spacing_change()
{
    auto& state = session_.viewport_state();
    bool ok = false;
    const auto spacing = QInputDialog::getDouble(this,
        QCoreApplication::translate("polivex::ui::MainWindow", "Grid spacing"),
        QCoreApplication::translate("polivex::ui::MainWindow", "Scene units per grid step:"), state.grid_spacing, 0.1,
        1000.0, 2, &ok);
    if (!ok) {
        return;
    }

    state.grid_spacing = spacing;
    refresh_viewport();
}

void MainWindow::handle_background_transparency_toggled(bool transparent)
{
    auto& state = session_.viewport_state();
    state.background_style = transparent ? polivex::app::BackgroundStyle::Transparent
                                         : polivex::app::BackgroundStyle::Solid;
    refresh_viewport();
}

void MainWindow::handle_background_color_change()
{
    auto& state = session_.viewport_state();
    const auto color = QColorDialog::getColor(QColor(state.background_red, state.background_green, state.background_blue), this);
    if (!color.isValid()) {
        return;
    }

    state.background_red = static_cast<std::uint8_t>(color.red());
    state.background_green = static_cast<std::uint8_t>(color.green());
    state.background_blue = static_cast<std::uint8_t>(color.blue());
    state.background_style = polivex::app::BackgroundStyle::Solid;
    refresh_viewport();
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

void MainWindow::refresh_layers()
{
    if (layers_list_ == nullptr) {
        return;
    }

    const auto selected_ids = session_.selected_entity_ids();
    const auto primary_id = session_.selected_entity_id();
    const auto& rectangles = session_.active_document().rectangles();

    const QSignalBlocker blocker(layers_list_);
    layers_list_->clear();

    for (auto iterator = rectangles.rbegin(); iterator != rectangles.rend(); ++iterator) {
        auto* item = new QListWidgetItem(QString::fromStdString(iterator->name), layers_list_);
        item->setData(Qt::UserRole, static_cast<qulonglong>(iterator->id));
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setToolTip(iterator->kind == polivex::core::RectangleKind::Vector
                ? QCoreApplication::translate("polivex::ui::MainWindow", "Vector layer")
                : QCoreApplication::translate("polivex::ui::MainWindow", "Sketch layer"));
        if (std::find(selected_ids.begin(), selected_ids.end(), iterator->id) != selected_ids.end()) {
            item->setSelected(true);
        }
        if (primary_id.has_value() && *primary_id == iterator->id) {
            layers_list_->setCurrentItem(item);
        }
    }

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
    grid_visible_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Show Grid"));
    grid_spacing_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Grid Spacing..."));
    background_transparent_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Transparent Background"));
    background_color_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Background Color..."));
    bring_to_front_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Bring to Front"));
    send_to_back_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Send to Back"));
    move_up_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Move Up"));
    move_down_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Move Down"));
    align_left_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Align Left"));
    align_right_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Align Right"));
    align_horizontal_center_action_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Align Horizontal Center"));
    align_top_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Align Top"));
    align_bottom_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Align Bottom"));
    align_vertical_middle_action_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Align Vertical Middle"));
    distribute_horizontally_action_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Distribute Horizontally"));
    distribute_vertically_action_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Distribute Vertically"));
    file_toolbar_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "File"));
    browser_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Structure"));
    layers_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Layers"));
    inspector_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Inspector"));
    if (layers_label_ != nullptr) {
        layers_label_->setText(QCoreApplication::translate(
            "polivex::ui::MainWindow", "Double-click a layer to rename it. Use the buttons to change order."));
    }
    layer_top_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Top"));
    layer_up_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Up"));
    layer_down_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Down"));
    layer_bottom_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Bottom"));
    statusBar()->showMessage(QCoreApplication::translate("polivex::ui::MainWindow", "Ready"));
    refresh_window_state();
    refresh_viewport();
    refresh_inspector();
    refresh_layers();
}

void MainWindow::refresh_viewport()
{
    const auto& state = session_.viewport_state();
    viewport_->set_viewport_state(state);
    viewport_->set_rectangles(session_.active_document().rectangles());
    viewport_->set_selected_entity_ids(session_.selected_entity_ids());

    vector_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Vector);
    sketch_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Sketch);
    model_workspace_action_->setChecked(state.workspace == polivex::app::Workspace::Model);
    top_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Top);
    front_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Front);
    right_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Right);
    isometric_camera_action_->setChecked(state.camera_preset == polivex::app::CameraPreset::Isometric);
    rectangle_tool_action_->setChecked(state.active_tool == polivex::app::ActiveTool::Rectangle);
    grid_visible_action_->setChecked(state.grid_style == polivex::app::GridStyle::Visible);
    background_transparent_action_->setChecked(state.background_style == polivex::app::BackgroundStyle::Transparent);
    background_color_action_->setEnabled(state.background_style != polivex::app::BackgroundStyle::Transparent);
    const auto has_selection = session_.selected_entity_id().has_value();
    bring_to_front_action_->setEnabled(has_selection);
    send_to_back_action_->setEnabled(has_selection);
    move_up_action_->setEnabled(has_selection);
    move_down_action_->setEnabled(has_selection);
    layer_top_button_->setEnabled(has_selection);
    layer_up_button_->setEnabled(has_selection);
    layer_down_button_->setEnabled(has_selection);
    layer_bottom_button_->setEnabled(has_selection);
    const auto multi_selection = session_.selected_entity_ids().size() > 1;
    align_left_action_->setEnabled(multi_selection);
    align_right_action_->setEnabled(multi_selection);
    align_horizontal_center_action_->setEnabled(multi_selection);
    align_top_action_->setEnabled(multi_selection);
    align_bottom_action_->setEnabled(multi_selection);
    align_vertical_middle_action_->setEnabled(multi_selection);
    distribute_horizontally_action_->setEnabled(session_.selected_entity_ids().size() > 2);
    distribute_vertically_action_->setEnabled(session_.selected_entity_ids().size() > 2);
}

void MainWindow::refresh_inspector()
{
    const auto selection_count = session_.selected_entity_ids().size();
    const auto entity_id = session_.selected_entity_id();
    const auto* rectangle = entity_id.has_value() ? session_.active_document().rectangle(*entity_id) : nullptr;

    if (selection_count != 1 || rectangle == nullptr) {
        inspector_label_->setText(selection_count > 1
                ? QCoreApplication::translate("polivex::ui::MainWindow", "%1 objects selected").arg(selection_count)
                : QCoreApplication::translate("polivex::ui::MainWindow",
                    "Choose Rectangle, then drag on the canvas to create an object."));
        geometry_label_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Properties"));
        {
            const QSignalBlocker blocker(x_spinbox_);
            x_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(y_spinbox_);
            y_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(width_spinbox_);
            width_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(height_spinbox_);
            height_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(rotation_spinbox_);
            rotation_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(corner_radius_spinbox_);
            corner_radius_spinbox_->setValue(0.0);
        }
        {
            const QSignalBlocker blocker(opacity_slider_);
            opacity_slider_->setValue(0);
        }
        color_button_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Fill color"));
        color_button_->setStyleSheet(QString());
        const auto disable = [](auto* widget) { widget->setEnabled(false); };
        disable(x_spinbox_);
        disable(y_spinbox_);
        disable(width_spinbox_);
        disable(height_spinbox_);
        disable(rotation_spinbox_);
        disable(corner_radius_spinbox_);
        disable(color_button_);
        disable(opacity_slider_);
        return;
    }

    const auto name = QString::fromStdString(rectangle->name);
    inspector_label_->setText(rectangle->kind == polivex::core::RectangleKind::Vector
            ? QCoreApplication::translate("polivex::ui::MainWindow", "Vector: %1").arg(name)
            : QCoreApplication::translate("polivex::ui::MainWindow", "Sketch: %1").arg(name));
    geometry_label_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Properties"));

    const auto bounds = rectangle->bounds;
    const auto style = rectangle->vector_style.value_or(polivex::core::VectorStyle {});
    {
        const QSignalBlocker blocker(x_spinbox_);
        x_spinbox_->setValue(bounds.minimum.x);
    }
    {
        const QSignalBlocker blocker(y_spinbox_);
        y_spinbox_->setValue(bounds.minimum.y);
    }
    {
        const QSignalBlocker blocker(width_spinbox_);
        width_spinbox_->setValue(polivex::core::rectangle_width(bounds));
    }
    {
        const QSignalBlocker blocker(height_spinbox_);
        height_spinbox_->setValue(polivex::core::rectangle_height(bounds));
    }
    {
        const QSignalBlocker blocker(rotation_spinbox_);
        rotation_spinbox_->setValue(rectangle->rotation_degrees);
    }
    {
        const QSignalBlocker blocker(corner_radius_spinbox_);
        corner_radius_spinbox_->setValue(rectangle->corner_radius);
    }
    {
        const QSignalBlocker blocker(opacity_slider_);
        opacity_slider_->setValue(style.opacity);
    }

    color_button_->setText(rectangle->kind == polivex::core::RectangleKind::Vector
            ? QCoreApplication::translate("polivex::ui::MainWindow", "Fill color")
            : QCoreApplication::translate("polivex::ui::MainWindow", "No fill"));
    color_button_->setStyleSheet(rectangle->kind == polivex::core::RectangleKind::Vector
            ? QString("background-color: rgb(%1, %2, %3);").arg(style.red).arg(style.green).arg(style.blue)
            : QString());

    x_spinbox_->setEnabled(true);
    y_spinbox_->setEnabled(true);
    width_spinbox_->setEnabled(true);
    height_spinbox_->setEnabled(true);
    rotation_spinbox_->setEnabled(true);
    corner_radius_spinbox_->setEnabled(true);
    color_button_->setEnabled(rectangle->kind == polivex::core::RectangleKind::Vector);
    opacity_slider_->setEnabled(rectangle->kind == polivex::core::RectangleKind::Vector);
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
