# Branching Model

Polivex uses a simple branch model designed for a small open-source team.

## Main Branches

- `main`: always the latest stable integration branch
- release branches: optional later, for example `release/0.2`

At the current stage, `main` is enough.
Do not add a long-lived `develop` branch unless the team grows and the workflow actually needs it.

## Working Branches

Create short-lived branches from `main`:

- `feat/...` for features
- `fix/...` for bug fixes
- `docs/...` for documentation
- `refactor/...` for internal cleanup
- `build/...` for tooling and CI

## Merge Rules

- merge through pull requests
- no direct pushes to `main`
- require at least one review when the project has active maintainers
- require passing CI checks once CI exists
- prefer squash merge for small focused changes

## Branch Protection

Protect `main` with:

- pull request required
- force push disabled
- branch deletion disabled
- status checks required

## Release Tags

Use semantic version tags once releases start:

- `v0.1.0`
- `v0.2.0`
- `v1.0.0`

## Why This Model

This model is easier for new contributors than Git Flow and usually works better for modern open-source projects with pull-request based development.
