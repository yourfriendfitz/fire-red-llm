# Development Workflow

## Current Phase

The repository is in Milestone 0. The GitHub fork exists, the product spec is written, and the next implementation milestone is a containerized vanilla ROM build.

Current workflow goals:

- Keep upstream `pret/pokefirered` build behavior intact.
- Keep project-specific planning, AI, emulator, and tooling work visible in this fork.
- Use branch-per-milestone work and PR summaries with explicit verification.
- Keep all build and validation tooling container-first once Milestone 1 is implemented.
- Never commit copyrighted ROMs, generated ROM outputs, model artifacts, tokens, or local emulator state.

## Milestone Workflow

Use the same working shape as `yourfriendfitz/support-ticket-llm`:

- Create a branch per milestone, named like `milestone-1-containerized-rom-build`.
- Keep each milestone PR focused on the deliverables in `spec.md`.
- Add or update `docs/milestone-N.md` with objective, delivered items, acceptance criteria, verification, and follow-up.
- Add a root `Makefile` target named `milestoneN-check` once that milestone has runnable verification.
- PR descriptions should use:

```markdown
## Summary
- ...

## Verification
- make milestoneN-check
- git diff --check origin/master...HEAD
- git diff --check
```

## Local Branches And Remotes

Expected remotes:

- `origin`: `https://github.com/yourfriendfitz/fire-red-llm.git`
- `upstream`: `https://github.com/pret/pokefirered.git`

Expected default branch:

- `master`

## Planned Project Layout

The upstream decompilation layout stays intact. Project-specific additions should use clear directories that do not obscure upstream source ownership.

Planned additions:

```text
docs/              Milestone notes, architecture notes, and development workflow
docker/            Container build files for ROM/tooling workflows
scripts/           Project utility scripts that are not upstream ROM tools
third_party/       Pinned external source dependencies such as agbcc
ai/                Future controller, bridge, policy, and evaluation code
```

`tools/` is already used by upstream `pokefirered`; avoid placing unrelated project scripts there unless they are ROM build tools.

## Container-First Policy

Do not install host system packages for this project unless the user explicitly asks.

Milestone 1 should provide a container workflow for:

- building or installing `agbcc`,
- running the vanilla ROM build,
- emitting predictable build artifacts,
- failing clearly when the user-provided base ROM is missing.

Until Milestone 1 is complete, the only checked target is:

```bash
make milestone0-check
```

Milestone checks should avoid touching ROM build artifacts unless that milestone explicitly verifies a ROM build.

## Base ROM Handling

This repository must not contain copyrighted base ROM files.

Expected behavior for Milestone 1:

- The user supplies a local FireRed base ROM.
- The file location is documented and ignored by git.
- Missing or invalid base ROM input produces a clear error.
- Generated `.gba`, `.elf`, `.map`, and `.sym` outputs remain ignored.

The upstream `.gitignore` already ignores `*.gba` except data assets that belong to source.
