#pragma once

#include <QWidget>

class QLabel;

namespace polivex::ui {

class ViewportPlaceholder : public QWidget {
    Q_OBJECT

public:
    explicit ViewportPlaceholder(QWidget* parent = nullptr);
    void retranslate_ui();

private:
    QLabel* message_ = nullptr;
};

}  // namespace polivex::ui
