# Pokemon FireRed AI Gym Challenge Spec

Last updated: 2026-05-07

## 1. Purpose

Build a small, focused Pokemon FireRed ROM hack where the player can start a new game and watch an AI attempt to clear Pewter City Gym.

The MVP should be narrow enough to finish and verify: one gym, two random Pokemon, no broad adventure loop, no save/menu complexity, and no item decisions. The ROM owns the constrained game scenario. An external AI controller watches emulator/game state and sends validated button inputs.

## 2. Product Summary

The user launches the ROM, selects New Game, and then watches the run. After game initialization, the player character starts the Pewter Gym challenge with two random Pokemon at the Pewter Gym level cap. The AI fights gym trainers in configured order, auto-heals after each win, then fights Brock. If Brock is defeated, the ROM shows a completed-run ending that simulates beating the game.

The experience should feel like watching an AI play a tiny self-contained Pokemon challenge, not like using a general-purpose emulator bot.

## 3. Users

Primary user: someone who wants to watch an AI attempt a short Pokemon FireRed gym challenge.

Secondary user: an engineer extending the ROM hack, AI controller, or emulator harness.

## 4. Goals

- Fork or vendor `pret/pokefirered` as the base decompilation project.
- Keep the ROM hack focused on Pewter City Gym.
- Let the user start normally enough to select New Game.
- Generate a party of two random Pokemon using the Pewter Gym level cap.
- Give each player Pokemon a random passive/triggered held item where possible.
- Prevent the player/AI from leaving the gym once the challenge starts.
- Disable or avoid start-menu, saving, bag-item, and broad overworld concerns.
- Auto-heal the player's party after winning each trainer battle.
- Make the AI clear gym trainers in deterministic configured order before Brock.
- Use an external emulator controller for AI logic instead of embedding an LLM inside the GBA ROM.
- Provide a stable state/action surface so the controller can make legal decisions without brittle screen scraping.
- Support deterministic scripted and heuristic agents before adding an LLM agent.
- Run project tooling through containers by default.

## 5. Non-Goals

- Full-game ROM hack content beyond the Pewter Gym challenge.
- Strict Nuzlocke rules. Only the level cap matters for random party generation.
- Permadeath, capture limits, encounter rules, or other Nuzlocke constraints.
- General Pokemon adventure navigation.
- User-controlled menus, saving, PC usage, shops, Pokemon Center trips, or Bag item decisions.
- Training, grinding, catching Pokemon, or party management outside the challenge.
- Letting the LLM directly mutate ROM memory or call arbitrary emulator commands.
- Shipping copyrighted base ROM files in the repository.

## 6. Initial Technical Decisions

| Area | Decision | Rationale |
| --- | --- | --- |
| Base ROM project | Direct repo fork of `pret/pokefirered` | Keeps ROM source changes first-class instead of hiding them in a nested dependency. |
| Project layout | Add AI tooling inside the fork under project-specific directories | Lets the ROM, bridge, controller, docs, and tests version together. |
| Build policy | Container-first with Ubuntu 24.04-based build image | Keeps the GBA toolchain reproducible and avoids host package installs. |
| GBA compiler | Pin `pret/agbcc` in `third_party/agbcc` and build/install it inside the container | Matches upstream expectations while keeping the compiler revision reproducible. |
| AI placement | External controller | LLM/model iteration is much easier outside the ROM, and the ROM remains deterministic. |
| Emulator bridge | mGBA Lua bridge first | mGBA exposes scripting, memory reads, callbacks, input control, and socket options. |
| Controller actions | Validated button inputs | The viewer sees real gameplay, and the agent cannot skip battles by mutating state. |
| State surface | ROM-maintained telemetry block | More reliable than visual OCR or ad hoc screen scraping. |
| Random run | Seeded randomization | Reproducible seeds make bugs and AI regressions debuggable. |
| First AI | Deterministic/scripted bot | Proves the ROM, bridge, and action contract before model behavior is introduced. |
| LLM role | Qwen3-0.6B via a local OpenAI-compatible `llama-server` adapter | Small enough for local experimentation; the controller still validates every action. |

## 7. High-Level Architecture

```text
User
  -> mGBA running hacked FireRed ROM
      -> constrained Pewter Gym challenge
      -> AI telemetry block in RAM
  -> mGBA Lua bridge
      -> reads telemetry and emulator state
      -> presses emulator buttons
  -> AI controller process
      -> route planner
      -> battle state adapter
      -> scripted/heuristic/LLM policy
      -> action validator and fallback policy
  -> run logs and replay artifacts
```

Core rule: the ROM defines the game and the controller drives it through legal inputs. The controller may read exposed state, but the production controller must not write memory to win, heal, warp, skip dialog, or alter battle outcomes.

## 8. Component Responsibilities

### FireRed ROM Hack

- Owns the playable challenge.
- Initializes the challenge after New Game setup.
- Gives the player two random Pokemon at or below the Pewter Gym level cap.
- Assigns valid moves, stats, held items, and party metadata.
- Locks the player inside Pewter Gym.
- Prevents start-menu/save/bag paths from mattering during the challenge.
- Ensures gym trainers are fought before Brock.
- Auto-heals the player party after trainer victories.
- Ends the run after Brock is defeated.
- Updates the AI telemetry block with enough state for the external controller.

### AI Telemetry Block

The ROM should maintain a compact state mirror in RAM. The bridge should discover its address from build artifacts, preferably through a generated symbol manifest rather than a hard-coded offset.

Candidate fields:

- Magic/version bytes for validation.
- Run seed.
- Challenge phase: initializing, overworld, dialog, battle, victory, loss, complete.
- Current map and player position.
- Current target trainer index.
- Whether movement input is accepted.
- Whether dialog advancement is needed.
- Battle state id: menu, move select, switch select, fainted, victory, loss.
- Player active Pokemon slot, HP, status, level, species, moves, PP, and held item.
- Enemy active Pokemon species, level, HP estimate, and status where available.
- Legal action mask for the current state.
- Stalled-frame counter or state-change counter for recovery logic.

### Symbol/Build Manifest

The build should produce a small machine-readable manifest for the controller:

```json
{
  "rom": "build/pokefirered_ai_gym.gba",
  "symbols": {
    "gAiGymTelemetry": "0x02000000"
  },
  "version": "0.1.0"
}
```

The exact address above is illustrative, not a committed offset. The implementation should derive it from the linker map or another reliable build artifact.

### mGBA Lua Bridge

- Starts with the hacked ROM in mGBA.
- Reads the telemetry block every frame or on a fixed tick.
- Sends state snapshots to the controller process.
- Receives a desired action from the controller.
- Converts actions into button press/release timing.
- Captures screenshots or run logs when useful for debugging.
- Does not implement core battle intelligence.

### AI Controller

- Maintains the high-level run state.
- Handles deterministic overworld routing through Pewter Gym.
- Advances dialog safely.
- Chooses battle actions through a pluggable policy.
- Validates that every chosen action is legal for the current state.
- Falls back to a deterministic safe action if a policy fails.
- Detects stalls and attempts bounded recovery.
- Writes structured logs for each decision.

Policy modes:

| Mode | Purpose |
| --- | --- |
| Manual/debug | Developer can inspect state and issue actions. |
| Scripted | Hard-coded route and simple battle defaults to prove the bridge. |
| Heuristic | Type/move/HP-aware decisions without an LLM. |
| LLM | Model chooses from legal actions using compact JSON state. |

### LLM Policy

The LLM receives compact state plus an explicit legal action list. It returns one action and a short rationale for logs. The controller validates the action before sending inputs to mGBA.

Example input shape:

```json
{
  "phase": "battle_move_select",
  "goal": "defeat Brock",
  "activePokemon": {
    "species": "Butterfree",
    "level": 14,
    "hp": 31,
    "maxHp": 38,
    "status": "none",
    "moves": [
      {"slot": 1, "name": "Confusion", "pp": 24, "type": "Psychic", "power": 50},
      {"slot": 2, "name": "PoisonPowder", "pp": 35, "type": "Poison", "power": 0}
    ]
  },
  "opponent": {
    "species": "Onix",
    "level": 14,
    "hpPercent": 64,
    "status": "poisoned"
  },
  "party": [
    {"slot": 1, "species": "Butterfree", "hp": 31, "maxHp": 38, "status": "none"},
    {"slot": 2, "species": "Squirtle", "hp": 22, "maxHp": 40, "status": "none"}
  ],
  "legalActions": [
    {"type": "use_move", "slot": 1},
    {"type": "use_move", "slot": 2},
    {"type": "switch", "slot": 2}
  ]
}
```

Example output shape:

```json
{
  "action": {"type": "use_move", "slot": 1},
  "reason": "Confusion is the best available damaging move and avoids an unnecessary switch."
}
```

## 9. Game Flow

Expected MVP flow:

1. User launches the ROM in the supported emulator setup.
2. User selects New Game.
3. ROM skips or fast-forwards the Professor Oak intro, naming flow, and unused rival setup.
4. ROM initializes the AI Gym Challenge.
5. Player is placed in Pewter Gym with two generated Pokemon.
6. Controller takes over inputs.
7. Controller walks to the nearest configured gym trainer.
8. AI wins or loses the battle.
9. If the AI wins, ROM auto-heals the party.
10. Controller continues through configured gym trainers in order.
11. Controller challenges Brock last.
12. If Brock is defeated, ROM shows a custom completed-run ending with short simulated credits/run summary text.
13. If the player loses any battle, ROM shows a failed-run ending or returns to a clear loss state.

## 10. Challenge Rules

### Party Generation

- Generate exactly two Pokemon.
- Pokemon are selected randomly from an allowed species pool.
- MVP species pool is all valid National Dex species represented in FireRed data, excluding invalid/special placeholders, eggs, legendary/mythical Pokemon, and species that have no direct-damage move at level 14.
- Duplicate species are disallowed within a generated two-Pokemon party for variety, not because of Nuzlocke rules.
- The level cap is Brock's highest Pokemon level in FireRed: level 14.
- Generated Pokemon must not exceed the level cap.
- MVP generated Pokemon start at level 14.
- Moves use the vanilla FireRed initial moveset behavior for the generated species at level 14.
- The species eligibility check should reject generated candidates whose final level-14 moveset has no direct-damage move.
- Each Pokemon receives one random held item from an allowed passive/triggered held-item pool.
- Held items requiring player menu decisions should be excluded.
- Generated party should be reproducible from the run seed.

Initial held-item pool:

- Healing/status berries: Oran Berry, Sitrus Berry, Lum Berry, Cheri Berry, Chesto Berry, Pecha Berry, Rawst Berry, Aspear Berry, and Persim Berry.
- Battle trigger items: Quick Claw, King's Rock, Focus Band, Leftovers, and Shell Bell.
- Type-boost items: Black Belt, BlackGlasses, Charcoal, Dragon Fang, Hard Stone, Magnet, Miracle Seed, Mystic Water, NeverMeltIce, Poison Barb, Sharp Beak, Silk Scarf, SilverPowder, Soft Sand, Spell Tag, and TwistedSpoon.
- Excluded for MVP: Bag-use items, evolution items, mail, money/experience items, species-specific items, Choice Band, and items that add state complexity without improving the watchable challenge.

### Nuzlocke Clarification

Only the level cap is inherited from Nuzlocke-style rules. The MVP does not include permadeath, route encounters, catch limits, nickname rules, release rules, or no-duplicate rules.

### Battle Rules

- The AI may choose moves.
- The AI may switch Pokemon if switching is legal.
- The AI may not choose Bag/item actions.
- The AI should not need to run.
- If both player Pokemon faint during a battle, the run is lost.
- After each trainer win, the player's party is fully healed before the next fight.

### Gym Traversal Rules

- The player cannot leave Pewter Gym after the challenge starts.
- The controller should fight configured gym trainers before Brock.
- Trainer order should be deterministic and data-driven.
- In vanilla FireRed, Pewter Gym has a small trainer set; the implementation should not assume more trainers than the map actually contains unless the hack adds them.

## 11. Run State And Recovery

The controller should detect and handle common states:

- Overworld movement accepted.
- Dialog requires A/B advancement.
- Battle main menu visible.
- Move selection visible.
- Switch selection visible.
- Pokemon fainted prompt visible.
- Victory/defeat text visible.
- Unexpected menu or stall.

Recovery should be bounded and logged. If recovery fails, the run should end in a clear error state rather than pressing random buttons indefinitely.

## 12. Randomness And Reproducibility

Every run should have a seed. The seed should determine:

- Random species.
- Random held items.
- Any generated moves or party metadata not derived from species/level.

Run logs should include:

- ROM build version.
- Controller version.
- Seed.
- Generated party.
- Trainer sequence.
- Battle decisions.
- Final outcome.

## 13. Local Development And Tooling

Run build and test tooling through containers by default.

Expected local workflow:

- Docker image for the `pret/pokefirered` build toolchain.
- Make or script target to build the hacked ROM.
- Script to generate controller symbol/config manifest.
- mGBA Lua bridge for emulator integration.
- Controller process for scripted, heuristic, and LLM policies.
- Containerized/headless mGBA smoke-test path where feasible, using a virtual display if the selected package requires one.
- Optional host GUI mGBA path for watching the run; this is allowed as a user-facing runtime convenience, while build and automated checks remain container-first.
- Local LLM server path using `llama-server` with a Qwen3-0.6B GGUF artifact supplied outside git and pinned by filename/hash in local config.

The repository must not include a copyrighted base ROM. Any required `baserom.gba` or equivalent file should be documented as a user-provided local input and ignored by git.

## 14. Testing And Verification

ROM-level checks:

- Build succeeds in the container.
- Base upstream build remains reproducible where applicable.
- Hacked ROM build emits expected artifacts.
- Challenge initialization gives exactly two Pokemon.
- Generated Pokemon never exceed the level cap.
- The gym exit is blocked.
- Start menu/save/bag paths do not derail the challenge.
- Auto-heal occurs after trainer wins.
- Brock victory reaches the completed-run ending.

Controller checks:

- Telemetry block version/magic validation works.
- Symbol manifest resolves the telemetry address.
- Bridge can read state snapshots.
- Bridge can press buttons deterministically.
- Scripted controller can advance dialog.
- Scripted controller can navigate to configured trainers.
- Invalid policy outputs are rejected.
- Fallback policy chooses legal actions.
- Stalls produce bounded recovery and logs.

End-to-end checks:

- A deterministic seeded run can be replayed.
- A scripted or heuristic agent can complete at least one known-good seed.
- Failed seeds produce understandable logs.
- The LLM policy can run behind the same action validator without changing ROM logic.

## 15. Milestones

### Milestone 0: Spec And Upstream Foundation

Deliverables:

- `spec.md` finalized enough to build from.
- Upstream `pret/pokefirered` fork/vendor strategy selected.
- Repo-specific development notes documented.
- Container-first build plan documented.
- Legal base ROM handling documented.

Acceptance criteria:

- Another engineer can identify the ROM, bridge, controller, and policy boundaries.
- Open decisions are explicitly listed.
- No copyrighted ROM files are committed.

### Milestone 1: Containerized ROM Build

Deliverables:

- Container image or compose service for the FireRed build toolchain.
- Build command for upstream/unmodified ROM target.
- Build command for the hacked ROM target.
- Git-ignored local placement for required base ROM input.
- Basic documentation for build commands.

Acceptance criteria:

- ROM build runs through one documented container command.
- Build artifacts are emitted to a predictable ignored path.
- Missing base ROM produces a clear error message.

### Milestone 2: Playable Pewter Gym Challenge ROM

Deliverables:

- Challenge initialization after New Game.
- Warp/place player into Pewter Gym.
- Two-Pokemon seeded party generation.
- Level-cap enforcement.
- Random held item assignment from an allowed pool.
- Gym exit blocking.
- Start-menu/save/bag concerns disabled or made unreachable.
- Auto-heal after trainer wins.
- Brock victory ending.

Acceptance criteria:

- Human tester can launch the ROM, select New Game, and observe the challenge setup.
- Player cannot leave the gym during the challenge.
- Party generation is reproducible from a seed.
- Winning Brock reaches the completed-run ending.
- Losing a battle reaches a clear loss state.

### Milestone 3: Telemetry And Emulator Bridge

Deliverables:

- ROM telemetry block.
- Generated symbol/config manifest.
- mGBA Lua bridge that reads telemetry.
- Button input adapter.
- Structured state snapshot logs.

Acceptance criteria:

- Bridge validates telemetry version/magic.
- Bridge reads current phase, map, position, party, and battle state.
- Bridge can send deterministic button presses.
- Logs are enough to reconstruct what state the controller saw.

### Milestone 4: Scripted Controller

Deliverables:

- Controller process.
- Deterministic gym route planner.
- Dialog advancement.
- Trainer target sequencing.
- Simple legal battle action policy.
- Stall detection and bounded recovery.

Acceptance criteria:

- Scripted controller can navigate the gym and start trainer battles.
- Scripted controller can complete at least one known-good seed if the generated party allows it.
- Invalid or unexpected state transitions are logged clearly.
- Controller does not write memory to alter outcomes.

### Milestone 5: Heuristic Battle Policy

Deliverables:

- Type/move/HP-aware battle decision policy.
- Switch logic for fainted or low-HP active Pokemon.
- Seeded evaluation set.
- Run outcome summary report.

Acceptance criteria:

- Heuristic policy improves clear rate over the simple scripted battle policy.
- Evaluation can run multiple seeds and summarize wins/losses.
- Logs show chosen actions and reasons.

### Milestone 6: LLM Battle Policy

Deliverables:

- LLM policy adapter.
- Compact JSON prompt/state builder.
- Legal action list generation.
- Strict output parser.
- Action validator.
- Deterministic fallback when the model fails.

Acceptance criteria:

- LLM receives only compact state and legal actions.
- LLM cannot directly access emulator tools or mutate memory.
- Invalid model output is rejected and replaced by fallback.
- LLM policy runs through the same bridge/controller interface as scripted and heuristic policies.
- Run logs capture model-chosen action and short rationale without leaking secrets.

### Milestone 7: Watchable MVP Polish

Deliverables:

- Clear startup instructions.
- Easy run command for the watchable mode.
- Optional on-screen challenge status text if practical.
- Final win/loss presentation.
- Replay/log artifacts for debugging.
- Known-good seeds documented.

Acceptance criteria:

- User can start a watchable AI run with documented steps.
- The run either clears Brock, loses, or errors with understandable output.
- The MVP demonstrates the intended experience without requiring developer intervention mid-run.

## 16. Current Decisions And Remaining Open Decisions

Resolved:

- Base project should become a direct repo fork of `pret/pokefirered`.
- GitHub fork/repository name should be `yourfriendfitz/fire-red-llm`.
- Upstream source should not be nested as a submodule; project-specific tooling should live inside the fork.
- `pret/agbcc` should be pinned under `third_party/agbcc` and built/installed inside the container.
- Build image should start from Ubuntu 24.04 and install the current upstream Linux dependencies, including `build-essential`, `binutils-arm-none-eabi`, `git`, and `libpng-dev`, plus `gcc-multilib` if required by the pinned `agbcc` build.
- Scope is Pewter City Gym only.
- "Nuzlocke" only means level cap for generated Pokemon.
- Brock's FireRed level cap is level 14.
- Generated party should use two unique, non-legendary, non-mythical National Dex species with at least one direct-damage move at level 14.
- Generated moves should use vanilla FireRed initial moveset behavior at level 14.
- Held items should come from the curated passive/triggered battle item pool listed in Party Generation.
- The post-New Game intro/name/rival flow should be skipped or fast-forwarded for MVP.
- Brock victory should use a custom completed-run ending with short simulated credits/run summary text, not the full vanilla credits path.
- The ROM owns game rules and challenge state.
- The AI/LLM runs outside the ROM.
- The controller drives the emulator through legal inputs.
- Item use decisions are out of scope.
- Auto-heal occurs after trainer wins.
- The player should not leave the gym during the challenge.
- Deterministic/scripted and heuristic agents should exist before LLM integration.
- First real LLM path should be Qwen3-0.6B through a local `llama-server` OpenAI-compatible adapter, with Qwen3-1.7B allowed as a fallback candidate if 0.6B cannot reliably emit valid actions.
- Automated validation should prefer a containerized/headless mGBA lane where feasible; watchable mode may use host GUI mGBA.

Open:

- Exact commit pins for `pret/pokefirered`, `pret/agbcc`, and the Qwen3 GGUF artifact.
- Exact directory names for project-specific tooling once upstream source is imported.
- Exact implementation hook for skipping the intro and starting the challenge.
- Exact implementation hook for the custom completed-run ending.
- Whether Qwen3-0.6B is strong enough after evaluation or Qwen3-1.7B should become the default LLM policy model.
- Exact mGBA package/command used for automated headless validation.

## 17. Definition Of Done For MVP

The MVP is done when:

- The ROM builds through the documented container workflow.
- The hacked ROM starts a Pewter Gym challenge after New Game.
- The player receives exactly two random Pokemon at or below level 14.
- The player cannot leave the gym.
- The AI/controller can traverse the gym, fight configured trainers, and fight Brock.
- The party auto-heals after trainer victories.
- The AI can choose moves and switches, but not Bag items.
- Brock victory reaches a completed-run ending.
- Battle loss reaches a clear failed-run ending.
- The controller reads a stable game-state surface and sends legal button inputs.
- A deterministic non-LLM policy works before the LLM policy.
- The LLM policy is validated, bounded, and replaceable.
- Seeded runs produce logs useful for debugging.
- Documentation explains how to build, run, verify, and extend the project.
