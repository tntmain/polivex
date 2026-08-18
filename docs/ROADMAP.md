# Roadmap

This is a direction, not a promise of dates. We will build the foundation first, then make sketching useful, then turn sketches into models. A CAD program is a large machine; skipping straight to the shiny buttons is how it becomes a large broken machine.

## 0. Foundation — in progress

- [x] Repository, licence, contribution rules, and issue templates
- [x] CMake build and CI for Linux and Windows
- [x] Qt desktop shell
- [x] Initial `ui -> app -> core` structure
- [ ] Formatting and static-analysis rules
- [ ] Logging and diagnostics

## 1. Desktop shell

Goal: a dependable cross-platform application shell.

- Document lifecycle: create, open, save, close
- Main window layout, docks, menus, and toolbars
- Input and shortcut handling
- Viewport abstraction
- Basic undo/redo plumbing

## 2. 2D sketching MVP

Goal: make and edit a useful sketch.

- Lines, rectangles, circles, and arcs
- Selection, move, and delete
- Snapping
- Dimensions and basic constraints
- Save and load sketch documents

## 3. Parametric modelling MVP

Goal: make simple 3D geometry from a closed sketch.

- Extrude and cut
- One additional feature: revolve or sweep
- Feature tree
- Rebuild model history after an edit

## 4. Make it pleasant to use

Goal: beginners should be able to discover the workflow without a tutorial open on a second monitor.

- Clearer tool grouping and states
- Keyboard shortcuts
- Sample files and short onboarding flows
- Useful error messages
- Sensible defaults

## 5. Grow the project

Goal: make it easy for more people to participate safely.

- Good-first-issue labels and contributor onboarding
- Release process and changelog
- Documentation expansion
- Decide whether extensions need a plugin boundary

## Not part of the first release

- Advanced simulation
- Cloud collaboration
- Enterprise PLM
- Specialised manufacturing workflows
