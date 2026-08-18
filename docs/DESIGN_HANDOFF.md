# Design Handoff from Figma

Yes, you can transfer a design from Figma into the project, but not as a one-click full application conversion.
For a desktop CAD app, Figma should be treated as a UI source, not as the source of business logic.

## Recommended Direction for Polivex

For Polivex, the recommended UI direction is `Qt Widgets`.
That is the better fit for a classic desktop CAD application with menus, dock panels, inspectors, and a large central viewport.

Use Figma for visual design and component rules, then implement the result manually in Qt Widgets.

`Qt Quick` remains a valid future option for highly animated or design-heavy surfaces, but it should not be the default path for the main CAD shell right now.

## Best Practical Path

For a C++ desktop application, the cleanest route is:

1. design screens and components in Figma
2. define spacing, typography, colors, states, and assets in Figma
3. translate the design into Qt Widgets components
4. connect those components to real application logic in C++

## Recommended Workflow

### For Qt Widgets

- use Figma as the visual source of truth
- build reusable widgets and view classes in Qt
- export icons, images, and tokens from Figma
- implement behavior manually in C++ and Qt

### For Qt Quick

If the project later adds a QML-based surface, Figma handoff is smoother there than with classic widget-based UI.
That is useful for special screens, but it is not required for the main desktop shell.

## Important Reality Check

You can transfer:

- layout
- spacing
- colors
- typography
- icons and assets
- component states

You cannot reliably transfer:

- CAD logic
- document model
- geometry operations
- undo/redo behavior
- complex desktop interactions without manual engineering

## Suggested Figma Rules

- create a small design system early
- use reusable components
- define hover, pressed, selected, and disabled states
- annotate interactions and shortcuts
- keep desktop scaling in mind
- define minimum window sizes and dock behavior

## Practical Figma to Qt Widgets Pipeline

1. Design the shell in Figma first.
2. Break the shell into implementation regions:
   toolbar, browser, inspector, status bar, viewport chrome.
3. Export shared tokens:
   colors, spacing, font sizes, corner radii, icon sizes.
4. Implement layout structure in Qt first:
   `QMainWindow`, `QDockWidget`, toolbars, status bar, central widget.
5. Implement reusable view classes:
   browser panel, inspector panel, viewport widget, future tool palettes.
6. Apply visual styling last with Qt stylesheets or palette tuning.
7. Connect widgets to `app` and `core` only after the shell matches the design.

## What to Transfer from Figma

- panel sizes and proportions
- toolbar grouping
- iconography
- typography scale
- spacing system
- color tokens
- interaction states

## What to Engineer Separately

- CAD commands
- viewport rendering
- snapping behavior
- undo and redo
- document structure
- tool activation rules

## Mapping Figma to Qt Widgets

- app shell frame -> `QMainWindow`
- left and right side panels -> `QDockWidget`
- top command areas -> `QToolBar` and menus
- central canvas area -> custom `QWidget` or viewport class
- inspector rows -> form layouts or custom property widgets
- reusable buttons and controls -> small custom widget subclasses when needed

## For This Project

If Polivex uses Qt 6, prefer one of these paths:

- manual handoff from Figma into Qt Widgets components
- limited QML prototyping only when a specific screen benefits from it

The goal is to keep the main application shell solid, desktop-oriented, and easy to maintain while still letting design work guide the UI.
