#include "ui/main_window.h"

#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QStatusBar>
#include <QToolBar>

#include "app/application_session.h"
#include "ui/viewport_placeholder.h"

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
    refresh_window_state();
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

    file_toolbar_ = addToolBar(QString());
    file_toolbar_->addAction(new_action_);
}

void MainWindow::create_layout()
{
    viewport_ = new ViewportPlaceholder(this);
    setCentralWidget(viewport_);

    browser_dock_ = new QDockWidget(this);
    browser_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    document_label_ = new QLabel(browser_dock_);
    document_label_->setMargin(12);
    browser_dock_->setWidget(document_label_);
    addDockWidget(Qt::LeftDockWidgetArea, browser_dock_);

    inspector_dock_ = new QDockWidget(this);
    inspector_dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    inspector_label_ = new QLabel(inspector_dock_);
    inspector_label_->setWordWrap(true);
    inspector_label_->setMargin(12);
    inspector_dock_->setWidget(inspector_label_);
    addDockWidget(Qt::RightDockWidgetArea, inspector_dock_);
}

void MainWindow::create_status_bar()
{
    statusBar();
}

void MainWindow::refresh_window_state()
{
    const auto& document = session_.active_document();
    const auto document_name = display_document_name();

    setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Polivex - %1").arg(document_name));
    document_label_->setText(
        QCoreApplication::translate("polivex::ui::MainWindow", "Active document: %1\nModified: %2")
            .arg(document_name,
                document.is_dirty()
                    ? QCoreApplication::translate("polivex::ui::MainWindow", "yes")
                    : QCoreApplication::translate("polivex::ui::MainWindow", "no")));
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
    english_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "English"));
    russian_action_->setText(QCoreApplication::translate("polivex::ui::MainWindow", "Russian"));
    file_toolbar_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "File"));
    browser_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Browser"));
    inspector_dock_->setWindowTitle(QCoreApplication::translate("polivex::ui::MainWindow", "Inspector"));
    inspector_label_->setText(QCoreApplication::translate(
        "polivex::ui::MainWindow", "Tool settings and entity properties will appear here."));
    statusBar()->showMessage(QCoreApplication::translate("polivex::ui::MainWindow", "Ready"));
    viewport_->retranslate_ui();
    refresh_window_state();
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
