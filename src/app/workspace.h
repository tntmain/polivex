#pragma once

#include <cstdint>

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

enum class GridStyle {
    Hidden,
    Visible,
};

enum class BackgroundStyle {
    Solid,
    Checkerboard,
    Transparent,
};

struct ViewportState {
    Workspace workspace = Workspace::Vector;
    CameraPreset camera_preset = CameraPreset::Top;
    double zoom = 1.0;
    double pan_x = 0.0;
    double pan_y = 0.0;
    ActiveTool active_tool = ActiveTool::Select;
    GridStyle grid_style = GridStyle::Visible;
    BackgroundStyle background_style = BackgroundStyle::Solid;
    double grid_spacing = 1.0;
    double snap_threshold = 5.0;
    std::uint8_t background_red = 32;
    std::uint8_t background_green = 37;
    std::uint8_t background_blue = 43;
};

}  // namespace polivex::app
