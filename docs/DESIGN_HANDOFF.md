# Design Handoff from Figma

Yes, you can transfer a design from Figma into the project, but not as a one-click full application conversion.
For a desktop CAD app, Figma should be treated as a UI source, not as the source of business logic.

## Best Practical Path

For a C++ desktop application, the cleanest route is:

1. design screens and components in Figma
2. define spacing, typography, colors, states, and assets in Figma
3. translate the design into Qt UI components
4. connect those components to real application logic in C++

## Recommended Workflow

### If you use Qt

- use Figma as the visual source of truth
- build reusable components in Qt
- export icons, images, and tokens from Figma
- implement behavior manually in C++ and Qt

If you use Qt Quick, Figma handoff is usually smoother than with classic widget-based UI.

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

## For This Project

If Polivex uses Qt 6, prefer one of these paths:

- manual handoff from Figma into Qt components
- Figma-to-Qt conversion for a starter UI, then manual cleanup

The second path can speed up prototyping, but the produced UI still needs engineering review and refinement.
