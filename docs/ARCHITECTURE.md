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
- UI: Qt 6 Widgets for the first shell
- rendering: decide after viewport prototype
- tests: lightweight automated tests from the start

## Module Layout

### `src/core`

Pure domain logic and shared project state.
This layer should stay independent from Qt Widgets and desktop-specific UI concepts.

Examples:

- document model
- sketch entities
- parametric constraints
- feature history state
- shared IDs and utility types

### `src/app`

Application orchestration between UI and domain logic.
This layer coordinates active documents, commands, tools, and future undo/redo behavior.

Examples:

- application session
- command dispatch
- tool activation
- document lifecycle

### `src/ui`

Desktop presentation layer built with Qt.
This layer is allowed to depend on `app`, but should not absorb domain logic that belongs in `core`.

Examples:

- main window
- dock panels
- viewport widgets
- dialogs
- toolbars and menus

### `tests`

Fast automated checks for the non-UI parts of the project first, then broader integration coverage later.

## Dependency Rule

Keep dependencies flowing in one direction:

- `ui -> app -> core`

Allowed:

- `ui` can call `app`
- `app` can call `core`

Avoid:

- `core` depending on `ui`
- `core` knowing about widgets or windows
- `ui` owning complex CAD rules directly

## Why This Split Matters

- sketch and modeling logic become easier to test
- UI rewrites become less risky
- future rendering changes stay more isolated
- contributors can work in smaller areas without touching everything

## Suggested Near-Term Additions

1. Add a command system in `src/app`.
2. Introduce sketch entities and constraints in `src/core`.
3. Add a viewport scene abstraction before full rendering work.
4. Keep file IO and import/export in a future dedicated `io` module.

## Open Questions

- which geometry kernel to adopt or build
- whether to stay on Qt Widgets or move to a hybrid Widgets and Quick approach
- which persistence format should be used first
- what the first plugin boundary should be
