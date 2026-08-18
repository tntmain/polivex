# Architecture Notes

This document defines the recommended starting direction for Polivex.
It is meant to guide early implementation, not freeze the design forever.

## Product Shape

Polivex should be a native desktop CAD application with:

- a 2D sketching workflow
- a 3D modeling workflow derived from sketches
- a UI simple enough for beginners

## Recommended Stack

- language: C++20
- build system: CMake
- UI: Qt 6
- rendering: decide after viewport prototype
- tests: unit tests from the start

## High-Level Modules

### `app`

Application startup, configuration, logging, and document lifecycle.

### `ui`

Main window, panels, toolbars, dialogs, and command wiring.

### `sketch`

2D geometry, constraints, dimensions, snapping, and editing tools.

### `model`

3D feature generation, model history, and regeneration logic.

### `core`

Shared data structures, identifiers, events, settings, and utilities.

### `io`

Project file format, import/export, and persistence.

## Suggested Early Priorities

1. Build a desktop shell with a viewport and tool system.
2. Define a document model that can host sketches and later 3D features.
3. Implement a minimal sketch engine before attempting full 3D workflows.
4. Keep UI code separate from geometric logic from day one.

## Architectural Principles

- keep the geometry and UI layers separate
- prefer explicit data flow
- isolate platform-specific code
- avoid over-engineering in the MVP
- design for future undo/redo and history tracking

## Open Questions

- which geometry kernel to adopt or build
- whether to use Qt Widgets, Qt Quick, or a hybrid UI
- which persistence format should be used first
- what the first plugin boundary should be
