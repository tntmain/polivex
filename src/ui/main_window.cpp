#include "ui/main_window.h"

#include <QString>
#include <QDockWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

#include "app/application_session.h"
#include "ui/viewport_placeholder.h"

namespace polivex::ui {

MainWindow::MainWindow(polivex::app::ApplicationSession& session, QWidget* parent)
    : QMainWindow(parent)
    , session_(session)
{
    setWindowTitle("Polivex");
    resize(1280, 800);

    create_actions();
    create_layout();
    create_status_bar();
    refresh_window_state();
}

void MainWindow::handle_new_document()
{
    session_.create_new_document();
    refresh_window_state();
}

void MainWindow::create_actions()
{
    auto* file_menu = menuBar()->addMenu("&File");
    auto* new_action = file_menu->addAction("&New");
    new_action->setShortcut(QKeySequence::New);
    connect(new_action, &QAction::triggered, this, &MainWindow::handle_new_document);

    file_menu->addSeparator();
    auto* exit_action = file_menu->addAction("E&xit");
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    auto* file_toolbar = addToolBar("File");
    file_toolbar->addAction(new_action);
}

void MainWindow::create_layout()
{
    setCentralWidget(new ViewportPlaceholder(this));

    auto* browser_dock = new QDockWidget("Browser", this);
    browser_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    document_label_ = new QLabel(browser_dock);
    document_label_->setMargin(12);
    browser_dock->setWidget(document_label_);
    addDockWidget(Qt::LeftDockWidgetArea, browser_dock);

    auto* inspector_dock = new QDockWidget("Inspector", this);
    inspector_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* inspector_label = new QLabel("Tool settings and entity properties will appear here.", inspector_dock);
    inspector_label->setWordWrap(true);
    inspector_label->setMargin(12);
    inspector_dock->setWidget(inspector_label);
    addDockWidget(Qt::RightDockWidgetArea, inspector_dock);
}

void MainWindow::create_status_bar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::refresh_window_state()
{
    const auto& document = session_.active_document();
    const auto document_name = QString::fromStdString(document.name());

    setWindowTitle(QString("Polivex - %1").arg(document_name));
    document_label_->setText(
        QString("Active document: %1\nDirty: %2")
            .arg(document_name, document.is_dirty() ? "yes" : "no"));
}

}  // namespace polivex::ui
