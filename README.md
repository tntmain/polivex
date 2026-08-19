<p align="center">
  <img src="resources/assets/polivex_icon.svg" alt="Polivex logo" width="160">
</p>

<h1 align="center">Polivex</h1>

<p align="center">
  <strong>Precise design and vector graphics in one desktop workspace.</strong><br>
  An open-source application for Windows and Linux.
</p>

<p align="center">
  <a href="https://github.com/tntmain/polivex/actions/workflows/ci.yml"><img src="https://github.com/tntmain/polivex/actions/workflows/ci.yml/badge.svg" alt="CI status"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MPL--2.0-brightgreen" alt="MPL 2.0 license"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B" alt="C++20">
  <img src="https://img.shields.io/badge/Qt-6-41CD52?logo=qt" alt="Qt 6">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-4c8bf5" alt="Windows and Linux">
</p>

> Polivex is at the very beginning. It opens a window today; the part where it makes excellent designs is what we are building next.

## What Polivex is for

Polivex brings two kinds of 2D work into one application: expressive vector graphics and precise CAD sketches. A document can begin as a clean vector composition or a dimensioned sketch, then grow into a 3D model when needed.

The aim is a clear beginner workflow with room to grow into a serious vector and sketch-based modelling tool.

## Workspaces

- **Vector** — paths, shapes, fills, strokes, layers, and SVG workflows.
- **Sketch** — exact geometry, snapping, dimensions, and constraints.
- **Model** — 3D features created from closed sketches.

These workspaces share one document foundation but do not confuse their jobs: visual styling belongs to Vector, engineering rules belong to Sketch, and solid features belong to Model.

## Current state

The repository contains a working Qt desktop shell, a small application/core split, a test target, translations, and CI for Linux and Windows. The viewport is still a placeholder and there are no editing tools yet.

## Technology

- C++20
- CMake
- Qt 6 Widgets
- Windows and Linux

The code is split deliberately:

```text
ui  ->  app  ->  core
```

`core` owns document, vector, and CAD rules. `app` coordinates tools and documents. `ui` draws windows, panels, and controls. More details are in the [architecture notes](docs/ARCHITECTURE.md).

## Build it

You need CMake 3.26+, a C++20 compiler, Ninja, and Qt 6.4 or newer with the Widgets and LinguistTools modules.

### Windows: MSYS2 UCRT64

Open the **MSYS2 UCRT64** terminal, install the required packages, then configure and run the project:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-qt6-tools

cmake --preset msys2-ucrt64
cmake --build --preset msys2-ucrt64
ctest --preset msys2-ucrt64
```

### Linux

Install Qt 6 development packages, CMake, Ninja, and a C++ compiler using your distribution's package manager. On Ubuntu or Debian:

```bash
sudo apt install cmake ninja-build g++ qt6-base-dev qt6-tools-dev
cmake --preset default
cmake --build --preset default
ctest --preset default
```

### Qt installed elsewhere

Tell CMake where Qt lives:

```powershell
cmake -S . -B build/local -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64
cmake --build build/local
ctest --test-dir build/local --output-on-failure
```

## Where to look next

- [Roadmap](docs/ROADMAP.md) — what we plan to build and in what order
- [Architecture notes](docs/ARCHITECTURE.md) — how the project is split into modules
- [UI direction](docs/UI_DIRECTION.md) — how the Vector, Sketch, and Model workspaces fit together
- [Contributing guide](CONTRIBUTING.md) — how to make a change or open a pull request
- [Branching model](docs/BRANCHING.md) — how branches and releases work
- [Translations and resources](resources/README.md) — how interface languages and application assets are organised
- [Security policy](SECURITY.md) and [support](SUPPORT.md)

## License

Polivex is distributed under the [Mozilla Public License 2.0](LICENSE).
