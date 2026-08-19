# Architecture notes

This is the starting shape of Polivex, not a constitution. Change it when evidence says we should, but keep the dependency direction simple while the project is young.

## Product shape

Polivex is a native desktop application for vector graphics, precise 2D sketching, and later 3D modelling. The UI should stay approachable without putting geometry or CAD rules inside buttons and widgets.

Vector objects and CAD sketch entities may share low-level geometry such as points, paths, transforms, and IDs. Their higher-level rules remain separate: fills, strokes, and layers belong to vector graphics; dimensions, constraints, and feature inputs belong to CAD sketches.

## Stack

- C++20
- CMake
- Qt 6 Widgets for the desktop shell
- Rendering choice after a viewport prototype
- Automated tests from the start, beginning with non-UI code

## Modules

### `src/core`

The domain layer. It must not know about Qt widgets, windows, or dock panels.

Examples: documents, shared geometry, vector paths and styles, sketch entities, sketch planes, constraints, feature history, IDs, and shared value types.

The first `SketchPlane` values are `XY`, `XZ`, and `YZ`. A later extension can represent a custom plane or a face of a 3D body without changing the meaning of an existing sketch entity.

### `src/app`

The application layer. It turns user intentions into changes to `core` and coordinates active documents, commands, tools, workspaces, and later undo/redo.

### `src/ui`

The Qt presentation layer: windows, panels, dialogs, toolbars, and viewport widgets. It may use `app`, but it does not own vector, geometry, or modelling rules.

### `tests`

Fast checks for `core` and `app` first. Integration and UI coverage can follow once the workflow is real.

### `resources`

Files packaged with the application. `resources/i18n` contains Qt translation sources; `resources/assets` is for icons, images, sample files, and similar content.

## Dependency rule

```text
ui  ->  app  ->  core
```

`ui` may call `app`; `app` may call `core`. The reverse directions are not allowed. In particular, keep `core` free of UI dependencies and keep complicated design decisions out of event handlers.

## Why keep this boundary

- Vector, sketch, and modelling logic stays easy to test.
- Changing the UI or renderer is less dangerous.
- Contributors can work on a smaller part of the codebase.
- The project does not become one enormous `MainWindow` class.

## Next architectural decisions

1. Add a command system and workspace activation in `app`.
2. Grow the first shared 2D geometry and separate vector/sketch entities beyond rectangles.
3. Define a viewport scene abstraction before committing to a renderer.
4. Add an `io` module when saving and import/export become substantial.

## Questions we have not decided yet

- Geometry kernel: adopt one, wrap one, or build a small layer first?
- Qt Widgets only, or a later Widgets/Quick hybrid?
- First persistence format?
- Whether plugins need a formal boundary at all?
