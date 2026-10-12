# PARADOX implementation plan

## Foundation update

A subsequent user request authorized foundational implementation despite the
unavailable PRD. The repository now has CMake/CTest, shared core modules and two
JUCE targets; see BUILD_AND_ARCHITECTURE.md for current contracts and validation.
The original inspection below describes the baseline before that work. It is
not a current claim that source/build files are absent. PRD mappings and stage
alignment remain blocked; no FR/NFR/AT definitions have been invented.

## Original evidence and scope

The selected checkout is `/workspace/PARADOX-VST3`, with origin identity
`github.com/Mohinem/PARADOX-VST3`. Inspection found an unborn local branch
`work`, no commit, and no tracked or pre-existing untracked files. The remote
advertised no refs; an explicit lookup of `refs/heads/main` returned exit 2
(no matching ref), without an access error. No PRD was available anywhere in
this checkout. This is a source availability blocker, not a build failure.

Consequently no functionality can be classified as implemented, partially
implemented, or missing from the intended product. All implementation status
is **unassessed**. JUCE version, toolchain, plugin targets, DSP architecture,
editor, test framework, dependencies, CI, and licensing are unknown. No build,
plugin startup, tests, allocation audit, or host validation was possible.
No source or build configuration was changed. These documents are provisional
and do not substitute for a PRD review.

## Proposed architecture (subject to PRD review)

Use a JUCE-independent law engine with explicit input/output contracts, bounded
storage, and deterministic scheduling. Keep host integration and audio
rendering outside that engine. Instrument and effect backends consume the same
law-engine output, translated into backend-specific render commands. Avoid
assuming that the product requires two plugin binaries: confirm host bus and
plugin-category requirements first.

Suggested dependency direction:

`JUCE host adapter -> event normalizer -> law engine -> render commands -> backend`

A control-plane compiler validates editable state and produces prepared runtime
snapshots. Audio processing adopts snapshots at a defined boundary; an off-thread
reclaimer destroys retired snapshots. The editor observes bounded telemetry and
submits commands rather than mutating audio-owned state directly.

### Proposed file structure

```text
Source/
  Core/             # Law engine, domain model, bounded events, deterministic clock
  Graph/            # Validation, compilation, prepared snapshots, handoff/reclamation
  Rendering/
    Instrument/     # Voice/render lifecycle, only as required by the PRD
    Effect/         # Input-audio render path, only as required by the PRD
  State/            # Versioned persistence, migrations, parameter/state translation
  Plugin/           # JUCE processor, buses, transport/MIDI/automation adapters
  Editor/           # UI models, components, bounded telemetry consumer
Tests/
  Core/             # Deterministic semantics and event order
  Graph/            # Validation and replacement lifetime tests
  Rendering/        # Offline signal and lifecycle tests
  State/            # Round trip, malformed input, migration tests
  Integration/      # Host, timing, realtime and performance checks
```

Names and boundaries are recommendations, not existing modules or confirmed
PRD requirements. Select frameworks and dependencies after source inspection.

## Dependency-aware prioritized backlog

The PRD's actual stage names and gates cannot be recovered from an empty
repository. The sequence below is provisional; map it to those stages before
implementation, rather than claiming it matches them.

| Priority | Work | Depends on | Exit evidence |
| --- | --- | --- | --- |
| P0 | Obtain intended source and PRD; review Sections 3, 4, 6, 8, 9, 10, 13 and full requirement definitions | Repository availability | Reviewed baseline, completed traceability, exact stage gates |
| P0 | Inspect build, JUCE pin, targets, buses, dependencies, tests and CI; reproduce baseline | Source baseline | Build and existing test results with platform limits |
| P1 | Define law-engine semantics, event clock/order, capacity limits, state ownership and backend contracts | PRD and baseline | Reviewed contracts and executable reference tests |
| P1 | Establish standalone core tests and host adapter harness | Contracts and build | Repeatable meaningful tests without a GUI host |
| P1 | Implement smallest law-engine vertical slice in PRD stage order | Core harness | Deterministic expected output across block partitions |
| P2 | Integrate instrument/effect backends in PRD order, reusing working components | Core slice and confirmed product targets | Offline rendering and relevant host checks |
| P2 | Add prepared graph publication, retirement and persistence/migrations | Ownership contracts and core semantics | Replacement stress and state restoration tests |
| P2 | Connect editor and automation to validated command/state interfaces | Stable contracts and parameter model | UI/state/automation consistency checks |
| P3 | Complete remaining scoped requirements and acceptance gates | All prior dependencies | Requirement-specific evidence for every FR/NFR/AT |

Do not begin this backlog's implementation as part of the documentation task.

## Architectural risks to investigate

These are proposed audit areas, not findings about existing code.

- **Realtime allocation and blocking:** inspect callbacks and their transitive
  calls for allocation, container growth, locks, logging, I/O, lazy initialization,
  reference-count final destruction, and graph construction. Preallocate bounded
  scratch/event storage; define overflow behavior. Verify after preparation with
  allocation instrumentation and stress tests, including MIDI bursts and swaps.
- **State management:** separate host parameters, editable domain state, prepared
  runtime data, and transient DSP state. Define save/restore threading, validation,
  schema versions, migrations and rejection behavior. Do not assume host callbacks
  run only on the message thread. Never serialize partially edited runtime data.
- **Deterministic event timing:** preserve sample offsets and an explicit clock;
  define tie-breaking among MIDI, automation, transport and law events. Decide
  seek/loop/reset behavior and PRNG seed ownership. Test block-partition invariance
  where required and avoid imposing it on algorithms whose PRD permits differences.
- **Graph replacement:** prepare and validate off the audio thread; transfer
  ownership without callback destruction. Define adoption boundaries, retirement
  acknowledgements and shutdown behavior. An atomic pointer alone does not solve
  lifetime safety. Specify transition/crossfade, tail preservation and peak CPU.
- **Backend coupling:** prohibit JUCE/plugin/UI types in core semantics. Make backend
  capabilities explicit, with defined rejection for unsupported commands.
- **Host/product identity:** plugin category and buses affect host compatibility;
  parameter IDs and plugin IDs require stability. Confirm identities before shipping.

## Decisions requiring clarification

1. Where are the intended PRD and source, and which revision is authoritative?
2. What are the PRD's stages, scope exclusions, and exact FR/NFR/AT definitions?
3. Does hybrid mean one switchable target, separate instrument/effect targets, or
   simultaneous input processing and synthesis? Which formats/platforms/hosts?
4. What law semantics, graph limits, feedback rules and timing guarantees apply?
5. What are event tie-break, overflow, transport reset and random-seed policies?
6. What must persist across presets, host restoration and graph replacement?
7. Must graph transitions preserve tails/voices, crossfade, or reset? What latency
   and CPU budget apply during replacement?
8. Which JUCE/toolchain versions, licenses, performance hardware and test gates
   does the PRD require?

Questions 3–8 should first be answered from the recovered PRD and code. Ask the
user only about remaining ambiguities; do not invent product decisions.
