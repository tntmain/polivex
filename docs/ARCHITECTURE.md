# Architecture notes

This is the starting shape of Polivex, not a constitution. Change it when evidence says we should, but keep the dependency direction simple while the project is young.

## Product shape

Polivex is a native desktop CAD application: make a 2D sketch, then build a 3D model from it. The UI should stay approachable without putting CAD rules inside buttons and widgets.

## Stack

- C++20
- CMake
- Qt 6 Widgets for the desktop shell
- Rendering choice after a viewport prototype
- Automated tests from the start, beginning with non-UI code

## Modules

### `src/core`

The domain layer. It must not know about Qt widgets, windows, or dock panels.

Examples: documents, sketch entities, constraints, feature history, IDs, and shared value types.

### `src/app`

The application layer. It turns user intentions into changes to `core` and coordinates active documents, commands, tools, and later undo/redo.

### `src/ui`

The Qt presentation layer: windows, panels, dialogs, toolbars, and viewport widgets. It may use `app`, but it does not own geometry or modelling rules.

### `tests`

Fast checks for `core` and `app` first. Integration and UI coverage can follow once the workflow is real.

### `resources`

Files packaged with the application. `resources/i18n` contains Qt translation sources; `resources/assets` is for icons, images, sample files, and similar content.

## Dependency rule

```text
ui  ->  app  ->  core
```

`ui` may call `app`; `app` may call `core`. The reverse directions are not allowed. In particular, keep `core` free of UI dependencies and keep complicated CAD decisions out of event handlers.

## Why keep this boundary

- Sketch and modelling logic stays easy to test.
- Changing the UI or renderer is less dangerous.
- Contributors can work on a smaller part of the codebase.
- The project does not become one enormous `MainWindow` class.

## Next architectural decisions

1. Add a command system in `app`.
2. Add sketch entities and constraints in `core`.
3. Define a viewport scene abstraction before committing to a renderer.
4. Add an `io` module when saving and import/export become substantial.

## Questions we have not decided yet

- Geometry kernel: adopt one, wrap one, or build a small layer first?
- Qt Widgets only, or a later Widgets/Quick hybrid?
- First persistence format?
- Whether plugins need a formal boundary at all?
