# Contributing to Polivex

Thanks for taking an interest in Polivex. This is an early project, so small improvements, sharp questions, tests, and honest feedback are all useful contributions.

## Before writing code

- Read the [roadmap](docs/ROADMAP.md) and [architecture notes](docs/ARCHITECTURE.md).
- Search existing issues and pull requests first.
- For a large feature or a change to the geometry, file format, renderer, or dependencies, open an issue before investing time in code.

## A normal contribution

1. Pick or create an issue.
2. Create a short-lived branch from `main`.
3. Keep the change focused.
4. Build and run relevant tests locally.
5. Open a pull request and explain what changed.

## Branch names

Use a prefix that describes the work:

- `feat/sketch-toolbar`
- `fix/windows-build`
- `docs/roadmap-update`
- `refactor/document-api`
- `build/qt-version-check`

The fuller branch policy is in [docs/BRANCHING.md](docs/BRANCHING.md).

## Pull requests

Please keep one PR about one problem. Link its issue when there is one, mention trade-offs, and attach a screenshot or short recording for visible UI changes. Do not mix a broad refactor into a feature PR unless it is required for the feature.

Before requesting review:

- [ ] The project builds on your machine.
- [ ] Relevant tests pass, or you explain why no test applies.
- [ ] Documentation is updated when behaviour, setup, or workflow changed.
- [ ] UI changes include a screenshot or recording.

## Code and comments

Prefer clear code over clever code. Keep platform-specific code in a small, obvious place. Do not add a dependency casually: discuss it first if it affects the build, licensing, or architecture.

Comments are for intent, constraints, and trade-offs—not a translation of the line below them.

- Write comments in English.
- Use short `//` comments near the code they explain.
- Use `TODO:` for planned work, `FIXME:` for known incorrect or unsafe behaviour, and `NOTE:` for context that must not be missed.
- Use `///` only for public API documentation.
- Remove commented-out code instead of leaving it in the repository.
- A closing comment such as `// namespace polivex::ui` is fine when it makes a long file easier to scan.

Good:

```cpp
// Keep UI state out of the geometry layer.
// TODO: Replace the placeholder viewport with a sketch scene.
```

Not useful:

```cpp
// Increment i.
++i;
```

## Commit messages

Use a short imperative summary. For example:

- `Add sketch document skeleton`
- `Fix MSYS2 build preset`
- `Document branch protection rules`

## Review culture

Review the patch, not the person. Be direct, specific, and kind. The [Code of Conduct](CODE_OF_CONDUCT.md) applies everywhere the project is discussed.
