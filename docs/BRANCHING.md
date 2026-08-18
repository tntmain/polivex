# Branching model

Keep Git boring. `main` is the shared integration branch; everything else is a short-lived branch that comes back through a pull request.

## Branches

- `main` contains the current working version of Polivex.
- `release/0.2`-style branches may be added later when releases need maintenance.

There is no `develop` branch. Add one only if the team reaches a point where it solves a real problem.

Create work branches from `main` using `feat/`, `fix/`, `docs/`, `refactor/`, or `build/`:

```text
feat/sketch-toolbar
fix/windows-build
docs/roadmap-update
```

## Merging

- Open a pull request into `main`.
- Let CI pass before merging.
- Ask for a review when another active maintainer is available.
- Prefer squash merges for focused changes.
- Do not push directly to `main` or force-push it.

## GitHub protection for `main`

Enable required pull requests, required status checks, and protection against force pushes and branch deletion. At the moment, the CI workflow is the check to require.

## Releases

When releases begin, use semantic version tags such as `v0.1.0`, `v0.2.0`, and `v1.0.0`.
