#include <cstdlib>
#include <cmath>
#include <array>
#include <iostream>

#include "app/application_session.h"
#include "app/rectangle_batch_ops.h"
#include "core/rectangle_entity.h"

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

    if (!session.active_document().set_rectangle_corner_radius(vector_id, 10.0)) {
        std::cerr << "expected corner radius update to succeed\n";
        return EXIT_FAILURE;
    }

    vector_rectangle = session.active_document().rectangle(vector_id);
    if (vector_rectangle == nullptr || vector_rectangle->corner_radius != 0.5) {
        std::cerr << "expected corner radius to clamp to half of the shorter side\n";
        return EXIT_FAILURE;
    }

    const auto& second_rectangle =
        session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{4.0, 0.0}, {5.0, 1.0}});
    const auto second_id = second_rectangle.id;
    if (!session.active_document().bring_rectangle_to_front(vector_id)) {
        std::cerr << "expected bring to front to succeed\n";
        return EXIT_FAILURE;
    }
    if (session.active_document().rectangles().back().id != vector_id) {
        std::cerr << "expected selected rectangle to be last after bring to front\n";
        return EXIT_FAILURE;
    }
    if (!session.active_document().send_rectangle_to_back(vector_id)) {
        std::cerr << "expected send to back to succeed\n";
        return EXIT_FAILURE;
    }
    if (session.active_document().rectangles().front().id != vector_id || session.active_document().rectangles().back().id != second_id) {
        std::cerr << "expected z-order moves to reorder rectangles\n";
        return EXIT_FAILURE;
    }

    session.create_new_document();
    session.set_workspace(polivex::app::Workspace::Vector);
    const auto first_id = session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{0.0, 0.0}, {2.0, 1.0}}).id;
    const auto rotated_id = session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{10.0, 0.0}, {12.0, 1.0}}).id;
    if (!session.active_document().set_rectangle_rotation(rotated_id, 90.0)) {
        std::cerr << "expected rotation update to succeed\n";
        return EXIT_FAILURE;
    }

    const std::array selected_ids {first_id, rotated_id};
    if (!polivex::app::align_rectangles_left(session.active_document(), selected_ids)) {
        std::cerr << "expected align left to succeed for rotated rectangles\n";
        return EXIT_FAILURE;
    }

    const auto* aligned_first = session.active_document().rectangle(first_id);
    const auto* aligned_second = session.active_document().rectangle(rotated_id);
    if (aligned_first == nullptr || aligned_second == nullptr) {
        std::cerr << "expected aligned rectangles to exist\n";
        return EXIT_FAILURE;
    }

    const auto first_frame = polivex::core::rotated_frame_bounds(*aligned_first);
    const auto second_frame = polivex::core::rotated_frame_bounds(*aligned_second);
    if (std::abs(aligned_first->bounds.minimum.x - 0.0) > 1e-9 || std::abs(aligned_first->bounds.maximum.x - 2.0) > 1e-9) {
        std::cerr << "expected the anchor rectangle to stay in place during alignment\n";
        return EXIT_FAILURE;
    }
    if (std::abs(first_frame.minimum.x - second_frame.minimum.x) > 1e-9) {
        std::cerr << "expected rotated rectangles to share the same left edge after alignment\n";
        return EXIT_FAILURE;
    }

    session.create_new_document();
    session.set_workspace(polivex::app::Workspace::Vector);
    const auto a_id = session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{0.0, 0.0}, {1.0, 1.0}}).id;
    const auto b_id = session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{1.0, 0.0}, {2.0, 1.0}}).id;
    const auto c_id = session.active_document().add_rectangle(polivex::core::RectangleKind::Vector, {{2.0, 0.0}, {3.0, 1.0}}).id;

    if (!session.active_document().reorder_rectangles(std::array {c_id, a_id, b_id})) {
        std::cerr << "expected layer reorder to succeed\n";
        return EXIT_FAILURE;
    }

    const auto reordered = session.active_document().rectangles();
    if (reordered.size() != 3 || reordered[0].id != c_id || reordered[1].id != a_id || reordered[2].id != b_id) {
        std::cerr << "expected rectangle order to match the requested stack order\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
