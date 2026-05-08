# Milestone 0: Spec And Repo Foundation

## Objective

Prepare the FireRed AI Gym Challenge fork so Milestone 1 can start implementation without re-litigating scope, repository ownership, or workflow.

## Delivered

- GitHub fork created at `yourfriendfitz/fire-red-llm`.
- Local repository tracks `origin/master` and has `upstream` pointing at `pret/pokefirered`.
- `spec.md` defines product scope, architecture, milestones, open decisions, and MVP definition of done.
- `docs/development.md` defines the branch-per-milestone workflow.
- Root `Makefile` includes `milestone0-check` for foundation verification.
- `.gitignore` mirrors local planning exclusions from `support-ticket-llm` for `pre-spec.md`, `AGENTS.md`, `docs/decisions.md`, and `.agent/`.

## Acceptance Criteria

- Another engineer can identify the ROM, bridge, controller, and policy boundaries.
- Another engineer can identify the next implementation task.
- The repository is a real fork of `pret/pokefirered`.
- Open decisions are listed explicitly in `spec.md`.
- Copyrighted base ROMs and generated ROM artifacts are not committed.
- Local agent/planning notes are ignored and not part of repository history.
- Project tooling has a named milestone verification entrypoint.

## Verification

Run:

```bash
make milestone0-check
```

This checks:

- project planning docs exist,
- milestone docs exist,
- `origin` and `upstream` remotes are configured,
- the committed PR diff passes `git diff --check origin/master...HEAD`,
- the local working tree diff passes `git diff --check`.

`milestone0-check` must not create ROM build directories or other ignored build artifacts.

## Milestone 1 Follow-Up

Milestone 1 work is tracked in [milestone-1.md](milestone-1.md).
