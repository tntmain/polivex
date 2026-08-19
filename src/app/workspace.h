#pragma once

namespace polivex::app {

enum class Workspace {
    Vector,
    Sketch,
    Model,
};

enum class CameraPreset {
    Top,
    Front,
    Right,
    Isometric,
};

enum class ActiveTool {
    Select,
    Rectangle,
};

struct ViewportState {
    Workspace workspace = Workspace::Vector;
    CameraPreset camera_preset = CameraPreset::Top;
    double zoom = 1.0;
    double pan_x = 0.0;
    double pan_y = 0.0;
    ActiveTool active_tool = ActiveTool::Select;
};

}  // namespace polivex::app
