#include "ui/viewport_placeholder.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace polivex::ui {

ViewportPlaceholder::ViewportPlaceholder(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    auto* frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setObjectName("viewportFrame");

    auto* frame_layout = new QHBoxLayout(frame);
    auto* message = new QLabel("Viewport placeholder\n2D sketch and 3D scene will live here.", frame);
    message->setAlignment(Qt::AlignCenter);
    frame_layout->addWidget(message);

    layout->addWidget(frame);
}

}  // namespace polivex::ui
