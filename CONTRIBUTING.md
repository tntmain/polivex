# Contributing to Polivex

Thank you for your interest in contributing.
Polivex is intended to grow as a welcoming open-source CAD project, so clarity, respect, and maintainability matter as much as shipping features.

## Before You Start

- read the [roadmap](docs/ROADMAP.md)
- read the [branching model](docs/BRANCHING.md)
- check whether a similar issue or proposal already exists
- open an issue before starting large work

## Good First Contributions

Early contributions are especially welcome in these areas:

- repository structure and tooling
- build system setup
- documentation improvements
- UI prototypes
- sketch engine experiments
- tests and CI

## Workflow

1. Create or pick an issue.
2. Create a branch from `main`.
3. Make a focused change.
4. Add or update tests when possible.
5. Open a pull request using the repository template.

## Branch Naming

Use one of these prefixes:

- `feat/<short-name>`
- `fix/<short-name>`
- `docs/<short-name>`
- `refactor/<short-name>`
- `build/<short-name>`

Examples:

- `feat/sketch-toolbar`
- `fix/windows-build`
- `docs/roadmap-update`

## Pull Request Rules

- keep PRs small and focused
- explain the problem, solution, and tradeoffs
- link the relevant issue
- include screenshots or short recordings for UI work
- avoid mixing refactors with feature work unless necessary

## Coding Expectations

- prefer readable code over clever code
- keep platform-specific code isolated
- document non-obvious architectural decisions
- avoid introducing dependencies without discussion
- preserve a beginner-friendly user experience

## Comment Style

Comments should explain intent, constraints, or tradeoffs, not restate obvious code.

- write comments in English
- prefer short `//` comments near the relevant code
- use `TODO:` for planned follow-up work
- use `FIXME:` for known broken or unsafe behavior
- use `NOTE:` for important context another contributor should not miss
- use `///` only for API-level documentation when a type or function needs it
- avoid block comments unless a longer explanation is genuinely necessary
- do not leave commented-out code in the repository

Good:

- `// Keep UI state out of the geometry layer.`
- `// TODO: Replace placeholder viewport with sketch scene.`

Avoid:

- `// increment i`
- `// button click`

Closing namespace comments such as `// namespace polivex::ui` are welcome in implementation files when they improve readability.

## Commit Messages

Prefer short, descriptive commit messages such as:

- `Add sketch document model skeleton`
- `Fix CMake preset for Clang on Linux`
- `Document branch protection rules`

## Large Changes

Open a design discussion before starting work on:

- geometry kernel changes
- file format decisions
- rendering architecture changes
- dependency changes
- major UX redesigns

## Review Philosophy

Reviews should improve the patch and help contributors succeed.
Feedback should be specific, respectful, and actionable.
