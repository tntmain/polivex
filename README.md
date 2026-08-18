# Polivex

Polivex is an open-source CAD application for Linux and Windows.
The goal is to keep the product approachable for beginners while still being capable enough for real sketching and modeling workflows.

## Vision

Polivex aims to make CAD feel simpler, cleaner, and easier to learn.
The product direction is inspired by beginner-friendly tools such as Tinkercad, but with a native desktop workflow and a path toward more advanced sketch-based modeling.

## Project Status

This repository is in the early prototyping stage.
The initial desktop shell, modular structure, and contributor workflow are in place.

## Current Technical Direction

- C++20
- CMake
- Qt 6 Widgets for the first desktop shell
- layered architecture to keep domain logic away from UI code

## Repository Guide

- [Contributing Guide](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Security Policy](SECURITY.md)
- [Support](SUPPORT.md)
- [Roadmap](docs/ROADMAP.md)
- [Branching Model](docs/BRANCHING.md)
- [Architecture Notes](docs/ARCHITECTURE.md)
- [Figma Handoff Guide](docs/DESIGN_HANDOFF.md)

## Project Layout

- `src/core`: pure domain and shared project logic
- `src/app`: application session and orchestration layer
- `src/ui`: Qt windows, panels, tools, and presentation
- `tests`: lightweight test executables and future test suites

Current dependency direction:

- `ui -> app -> core`

`core` should stay free from Qt UI concerns.

## Build

### Windows with MSYS2 UCRT64

```powershell
cmake --preset msys2-ucrt64
cmake --build --preset msys2-ucrt64
ctest --preset msys2-ucrt64
```

### Generic local configure

If Qt 6 is installed in a custom location, point CMake at it with `CMAKE_PREFIX_PATH`.

```powershell
cmake -S . -B build/default -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64
cmake --build build/default
ctest --test-dir build/default --output-on-failure
```

## What Exists Today

- a Qt desktop shell with a main window and placeholder viewport
- a minimal `ProjectDocument` domain object
- an `ApplicationSession` layer between UI and core
- a first core test executable
- GitHub Actions CI for Linux and Windows

## What Comes Next

1. Define the sketch document model in `core`.
2. Add command and tool abstractions in `app`.
3. Replace the viewport placeholder with a real 2D sketch scene.
4. Introduce constraints, dimensions, and feature generation incrementally.

## License

This project is licensed under the Mozilla Public License 2.0.
See [LICENSE](LICENSE) for details.
