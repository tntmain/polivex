#include "ui/viewport_placeholder.h"

#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace polivex {
namespace ui {

ViewportPlaceholder::ViewportPlaceholder(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    auto* frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setObjectName("viewportFrame");

    auto* frame_layout = new QHBoxLayout(frame);
    message_ = new QLabel(frame);
    message_->setAlignment(Qt::AlignCenter);
    frame_layout->addWidget(message_);

    layout->addWidget(frame);
    retranslate_ui();
}

void ViewportPlaceholder::retranslate_ui()
{
    message_->setText(QCoreApplication::translate(
        "polivex::ui::ViewportPlaceholder", "Viewport placeholder\n2D sketch and 3D scene will live here."));
}

}  // namespace ui
}  // namespace polivex
