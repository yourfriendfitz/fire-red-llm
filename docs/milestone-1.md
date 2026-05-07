# Milestone 1: Containerized ROM Build

## Objective

Create a repeatable container-first workflow that can build the unmodified FireRed ROM from this fork using a user-provided base ROM.

The milestone should prove the upstream foundation before any gameplay hack work starts.

## Deliverables

- Docker build image for the FireRed toolchain.
- Pinned `pret/agbcc` setup under `third_party/agbcc` or an equivalent reproducible container-only path.
- Make target or script to build/install `agbcc` inside the container.
- Make target or script to run the vanilla ROM build inside the container.
- Documented ignored location for the user-provided base ROM.
- Friendly failure when the base ROM is missing.
- Documentation for build commands and expected artifacts.
- Root `Makefile` target named `milestone1-check`.

## Acceptance Criteria

- `make milestone1-check` runs the full containerized Milestone 1 verification.
- The vanilla ROM build runs through one documented container command.
- Build artifacts are emitted to predictable ignored paths.
- Missing base ROM input produces a clear, actionable error.
- No copyrighted ROMs or generated ROM outputs are committed.
- Upstream `make` behavior remains usable for normal decompilation targets.

## Verification

Expected final verification:

```bash
make milestone1-check
git diff --check
```

If the correct base ROM is present, Milestone 1 should also verify the generated FireRed ROM against the upstream expected hash.

## First Tasks

1. Add `docker/` build files for the ROM build environment.
2. Decide the exact `agbcc` pin and installation path.
3. Add a local base ROM path convention and error check.
4. Add containerized `make firered` or equivalent wrapper.
5. Add `milestone1-check`.
6. Update README and this milestone doc with final commands.
