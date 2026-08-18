# Figma handoff

Figma can give Polivex its visual language, but it cannot generate a finished CAD application. Treat it as the source of layout, components, and interaction rules; write the behaviour in C++.

## Our choice: Qt Widgets

The main application shell uses Qt Widgets. It suits a desktop CAD app with menus, toolbars, dock panels, inspectors, and a large central viewport.

Qt Quick/QML is still an option later for a highly animated or isolated screen. It is not the default for the CAD shell.

## What comes from Figma

- Layout and panel proportions
- Spacing, colours, typography, and corner radii
- Icons and image assets
- Component states: normal, hover, pressed, selected, disabled
- Toolbar grouping and interaction notes

## What must be engineered

- CAD commands and tool activation
- Document and geometry models
- Viewport rendering and snapping
- Undo/redo
- Saving, loading, and error handling

## Figma-to-Qt workflow

1. Design the window shell and its common components in Figma.
2. Make a small design system: named colours, spacing scale, font sizes, icon sizes, and component states.
3. Mark keyboard shortcuts, minimum window sizes, and dock behaviour in the design.
4. Build the structure in Qt: `QMainWindow`, `QDockWidget`, `QToolBar`, status bar, and a central viewport widget.
5. Build reusable widgets for repeated pieces such as inspector rows and panel headers.
6. Apply visual styling with Qt palettes or stylesheets.
7. Connect the widgets to `app` only after the UI structure is solid.

## Useful mapping

| Figma element | Qt Widgets implementation |
| --- | --- |
| App shell | `QMainWindow` |
| Side panels | `QDockWidget` |
| Command area | `QToolBar` and menus |
| Main canvas | Custom `QWidget` / viewport class |
| Inspector fields | Form layout or custom property widgets |

Export icons and raster assets from Figma, but do not try to export the entire screen as code. A careful manual handoff will produce a UI that behaves like a desktop application instead of a screenshot with buttons.
