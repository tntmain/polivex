# Polivex

Polivex is an open-source CAD application for Linux and Windows.
The goal is to keep the product approachable for beginners while still being capable enough for real sketching and modeling workflows.

## Vision

Polivex aims to make CAD feel simpler, cleaner, and easier to learn.
The product direction is inspired by beginner-friendly tools such as Tinkercad, but with a native desktop workflow and a path toward more advanced sketch-based modeling.

## Project Status

This repository is in the early planning stage.
The community, architecture, and roadmap are being defined before core implementation starts.

## Initial Scope

- 2D sketching workspace for simple constrained drawings
- turning sketches into 3D geometry
- beginner-friendly UI and onboarding
- cross-platform desktop support for Linux and Windows
- open development with contributor-friendly workflows

## Repository Guide

- [Contributing Guide](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Security Policy](SECURITY.md)
- [Support](SUPPORT.md)
- [Roadmap](docs/ROADMAP.md)
- [Branching Model](docs/BRANCHING.md)
- [Architecture Notes](docs/ARCHITECTURE.md)
- [Figma Handoff Guide](docs/DESIGN_HANDOFF.md)

## Recommended Technical Direction

The current recommended baseline is:

- C++20
- CMake
- Qt 6 for cross-platform desktop UI
- a modular core so geometry, sketching, and UI can evolve independently

This is a proposal, not a final lock-in.

## What Comes Next

1. Define the application architecture and core domain model.
2. Create the app shell and rendering prototype.
3. Build a minimal sketch workflow.
4. Add sketch constraints and simple feature generation.
5. Open the first contributor-friendly milestones.

## License

This project is licensed under the Mozilla Public License 2.0.
See [LICENSE](LICENSE) for details.
