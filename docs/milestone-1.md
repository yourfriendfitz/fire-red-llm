# Milestone 1: Containerized ROM Build

## Objective

Create a repeatable container-first workflow that can build and compare the unmodified FireRed ROM from this fork.

The milestone should prove the upstream foundation before any gameplay hack work starts.

## Deliverables

- Docker build image for the FireRed toolchain.
- Pinned `pret/agbcc` setup under `third_party/agbcc` or an equivalent reproducible container-only path.
- Make target or script to build/install `agbcc` inside the container.
- Make target or script to run the vanilla ROM build inside the container.
- `make compare` execution through the container.
- Documentation for build commands and expected artifacts.
- Root `Makefile` target named `milestone1-check`.

## Acceptance Criteria

- `make milestone1-check` runs the full containerized Milestone 1 verification.
- The vanilla ROM build runs through one documented container command.
- Build artifacts are emitted to predictable ignored paths.
- No copyrighted ROMs or generated ROM outputs are committed.
- Upstream `make` behavior remains usable for normal decompilation targets.

## Verification

Expected final verification:

```bash
make milestone1-check
git diff --check origin/master...HEAD
git diff --check
```

Milestone 1 verifies the generated FireRed ROM against the upstream expected hash via `make compare`.

## Build Commands

```bash
make rom-build
```

Runs the containerized FireRed build workflow. It fetches pinned `pret/agbcc` source under `third_party/agbcc`, installs the compiler into the upstream `tools/agbcc` path, and runs `make compare`.

The Makefile passes the current host UID/GID into Compose by default so generated bind-mounted files remain writable by the local user.

```bash
make rom-shell
```

Opens an interactive shell in the ROM build container for manual upstream build debugging.

Generated outputs such as `build/`, `pokefirered.gba`, `pokefirered.elf`, `pokefirered.map`, `tools/agbcc/`, and `third_party/agbcc/` stay ignored and local.

## First Tasks

1. Add `docker/` build files for the ROM build environment.
2. Decide the exact `agbcc` pin and installation path.
3. Add containerized `make compare` wrapper.
4. Add `milestone1-check`.
5. Update README and this milestone doc with final commands.
