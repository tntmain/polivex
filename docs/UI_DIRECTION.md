# UI direction

Polivex should feel like one application, even though it supports different kinds of work. The answer is not one giant toolbar. The answer is a shared shell with focused workspaces.

## Three workspaces

### Vector

For visual design: paths, shapes, fills, strokes, layers, grouping, and SVG. The inspector shows appearance properties first: colour, opacity, stroke width, and alignment.

### Sketch

For precise drawings and geometry: units, snapping, dimensions, and constraints. The inspector shows numerical values and constraint state first. A sketch can later become the input for a 3D feature.

### Model

For building solids from sketches: extrude, cut, revolve, and the feature tree. This workspace appears only after the basic sketch workflow is reliable.

## Layout

```text
┌─────────────────────────────────────────────────────────────┐
│ File  Edit  View       [ Vector ] [ Sketch ] [ Model ]       │
├─────────────────────────────────────────────────────────────┤
│ Tool palette │                 Canvas              │ Inspector│
│              │                                     │          │
│ Layers /     │         selected workspace          │ Context  │
│ document     │                                     │ settings │
├──────────────┴─────────────────────────────────────┴──────────┤
│ Status: units · cursor position · snap / constraint state     │
└─────────────────────────────────────────────────────────────┘
```

The top workspace switcher changes the visible tool palette and the inspector, not the whole application. The canvas, document tree, menus, shortcuts, and selection behaviour remain familiar.

## Rules that keep it simple

- Show tools for the active workspace only.
- Use one shared selection tool and the same navigation controls everywhere.
- Keep vector layers and CAD feature history distinct in the document panel.
- Make units and snapping visible in Sketch, but do not show them in Vector unless needed.
- Let a vector path be converted to a sketch only through an explicit command. Do not silently treat decorative artwork as engineering geometry.
- Keep the first version small: Vector and Sketch arrive before Model.

## First interface milestone

The first version now has the shared canvas, workspace switching, navigation, and a rectangle tool. A Vector rectangle and a Sketch rectangle are stored as different object types, even though both can be drawn on the XY view. The next step is selection and an inspector that exposes their different properties: fill and stroke for Vector; dimensions and constraints for Sketch.
