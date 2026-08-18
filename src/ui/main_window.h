#pragma once

#include <QMainWindow>

namespace polivex::app {
class ApplicationSession;
}

class QLabel;

namespace polivex::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(polivex::app::ApplicationSession& session, QWidget* parent = nullptr);

private slots:
    void handle_new_document();

private:
    void create_actions();
    void create_layout();
    void create_status_bar();
    void refresh_window_state();

    polivex::app::ApplicationSession& session_;
    QLabel* document_label_ = nullptr;
};

}  // namespace polivex::ui
