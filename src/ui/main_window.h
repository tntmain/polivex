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

namespace polivex::ui {
class ViewportPlaceholder;
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

private:
    void changeEvent(QEvent* event) override;
    void create_actions();
    void create_layout();
    void create_status_bar();
    void refresh_window_state();
    void retranslate_ui();
    [[nodiscard]] QString display_document_name() const;

    polivex::app::ApplicationSession& session_;
    QLabel* document_label_ = nullptr;
    QLabel* inspector_label_ = nullptr;
    QMenu* file_menu_ = nullptr;
    QMenu* view_menu_ = nullptr;
    QMenu* language_menu_ = nullptr;
    QToolBar* file_toolbar_ = nullptr;
    QAction* new_action_ = nullptr;
    QAction* exit_action_ = nullptr;
    QAction* english_action_ = nullptr;
    QAction* russian_action_ = nullptr;
    QDockWidget* browser_dock_ = nullptr;
    QDockWidget* inspector_dock_ = nullptr;
    ViewportPlaceholder* viewport_ = nullptr;
};

}  // namespace polivex::ui
