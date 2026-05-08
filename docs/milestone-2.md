# Milestone 2: Playable Pewter Gym Challenge

## Objective

Make the ROM itself launch into a deterministic Pewter Gym challenge after New Game, with enough rule enforcement to test the core scenario by hand before any emulator bridge or LLM controller exists.

## Delivered Items

- New Game now initializes the challenge and warps directly to Pewter Gym.
- New Game skips the vanilla Oak speech controls guide and intro sequence.
- The player receives two seeded, random level-14 Pokemon from a curated non-legendary pool.
- Generated Pokemon receive seeded passive held items.
- The title-screen Help System is disabled so smoke tests cannot get trapped before New Game.
- The start menu hides Bag and Save while the challenge is active.
- Gym exits are blocked by map coordinate scripts.
- Winning against Camper Liam heals the party and clamps levels back to Brock's level cap.
- Defeating Brock marks the run complete and shows a custom completed-run message.
- Losing a battle records a loss state, heals the party, and returns to the gym with a loss message.
- `make milestone2-check` builds the hacked ROM without using upstream byte-for-byte compare.

## Acceptance Criteria

- A tester can launch the ROM, select New Game, and observe the challenge setup in Pewter Gym.
- The player cannot leave Pewter Gym through the exit tiles.
- Party generation is reproducible from seed `F17E`.
- The active challenge party cannot remain above level 14 after trainer wins.
- Brock victory reaches a clear completed-run ending.
- Battle loss reaches a clear loss state.

## Verification

```bash
make milestone2-check
git diff --check origin/master...HEAD
git diff --check
```

## Follow-Up

- Add emulator-assisted smoke tests once the emulator bridge exists.
- Expose challenge state through the future controller memory/API surface.
- Broaden automated validation around seeded party contents once a ROM-state reader is available.
