#include <cstdlib>
#include <iostream>

#include "app/application_session.h"

int main()
{
    polivex::app::ApplicationSession session;

    if (session.active_document().name() != "Untitled") {
        std::cerr << "expected new session document to be Untitled\n";
        return EXIT_FAILURE;
    }

    session.active_document().rename("Bracket");

    if (!session.active_document().is_dirty()) {
        std::cerr << "expected renamed document to be dirty\n";
        return EXIT_FAILURE;
    }

    session.create_new_document();

    if (session.active_document().name() != "Untitled") {
        std::cerr << "expected reset document to be Untitled\n";
        return EXIT_FAILURE;
    }

    if (session.active_document().is_dirty()) {
        std::cerr << "expected new document to be clean\n";
        return EXIT_FAILURE;
    }

    session.set_workspace(polivex::app::Workspace::Model);
    session.set_camera_preset(polivex::app::CameraPreset::Isometric);

    if (session.viewport_state().camera_preset != polivex::app::CameraPreset::Isometric) {
        std::cerr << "expected model workspace to support an isometric camera\n";
        return EXIT_FAILURE;
    }

    session.set_workspace(polivex::app::Workspace::Sketch);

    if (session.viewport_state().camera_preset != polivex::app::CameraPreset::Top) {
        std::cerr << "expected sketch workspace to reset the camera to top\n";
        return EXIT_FAILURE;
    }

    if (!session.create_rectangle({3.0, 4.0}, {1.0, 2.0})) {
        std::cerr << "expected sketch workspace to create a rectangle\n";
        return EXIT_FAILURE;
    }

    const auto rectangles = session.active_document().rectangles();
    if (rectangles.size() != 1 || rectangles.front().kind != polivex::core::RectangleKind::Sketch) {
        std::cerr << "expected one sketch rectangle\n";
        return EXIT_FAILURE;
    }

    if (rectangles.front().sketch_plane != polivex::core::SketchPlane::XY) {
        std::cerr << "expected a first sketch rectangle to be on the XY plane\n";
        return EXIT_FAILURE;
    }

    if (rectangles.front().bounds.minimum.x != 1.0 || rectangles.front().bounds.minimum.y != 2.0
        || rectangles.front().bounds.maximum.x != 3.0 || rectangles.front().bounds.maximum.y != 4.0) {
        std::cerr << "expected rectangle bounds to be normalised\n";
        return EXIT_FAILURE;
    }

    session.create_new_document();
    session.set_workspace(polivex::app::Workspace::Vector);
    if (!session.create_rectangle({0.0, 0.0}, {2.0, 1.0})) {
        std::cerr << "expected vector workspace to create a rectangle\n";
        return EXIT_FAILURE;
    }
    const auto vector_id = session.active_document().rectangles().front().id;
    session.select_rectangle(vector_id);

    if (!session.move_selected_rectangle({3.0, -2.0})) {
        std::cerr << "expected selected vector rectangle to move\n";
        return EXIT_FAILURE;
    }

    const polivex::core::VectorStyle style {255, 128, 0, 90};
    if (!session.set_selected_vector_style(style)) {
        std::cerr << "expected selected vector rectangle style to change\n";
        return EXIT_FAILURE;
    }

    const auto* vector_rectangle = session.active_document().rectangle(vector_id);
    if (vector_rectangle == nullptr || vector_rectangle->bounds.minimum.x != 3.0
        || !vector_rectangle->vector_style.has_value() || vector_rectangle->vector_style->opacity != 90) {
        std::cerr << "expected vector rectangle move and style to persist\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
