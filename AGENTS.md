# PARADOX task guidance

## Current evidence and scope

This checkout was empty at the original planning baseline; foundation code now exists. The original PRD remains unavailable. `docs/IMPLEMENTATION_PLAN.md` is provisional;
`docs/REQUIREMENTS_TRACEABILITY.md` enumerates all requested IDs without inventing
their meanings. The user authorized foundational behavior; obtain the authoritative PRD before expanding its law semantics.
Review especially Sections 3, 4, 6, 8, 9, 10 and 13, and read exact FR-001–FR-027,
NFR-01–NFR-10 and AT-01–AT-06 definitions. PRD scope exclusions and stage gates
control work; architecture proposals are not authority to expand scope.

## Implementation constraints

- Inspect existing instructions, source, working-tree changes, build targets,
  pinned JUCE/toolchain and dependencies before editing. Preserve working code.
- Use the existing isolated checkout; do not create a worktree unless requested.
- Keep law-engine semantics independent of JUCE, UI and render backends where
  consistent with the recovered PRD. Separate instrument/effect rendering contracts.
- Audio callbacks must avoid allocation/deallocation, blocking locks, file/network
  I/O, logging and unbounded work. Prepare bounded storage before processing.
- Specify sample timing, equal-time event ordering, overflow and transport policies.
  Never silently substitute wall-clock timing for host/sample timing.
- Validate/prepare graph snapshots off-thread; adopt at a defined boundary and
  retire/destroy off-thread. Prove lifetime safety, including shutdown.
- Separate editable state, host parameters, runtime snapshots and transient DSP
  state. Validate restoration, version schemas and preserve stable published IDs.
- Do not add dependencies, plugin targets or features solely from provisional plans.

## Testing expectations

Use existing repository build/CI commands once available. Test actual law outputs,
backend rendering, event ordering and relevant block-size/sample-rate variations.
Add state round-trip/migration/malformed-input checks, graph replacement stress,
callback allocation instrumentation and relevant host integration validation.
Use exact PRD performance thresholds and acceptance criteria rather than invented
budgets. Record executed counts/results and distinguish skipped/unrun from passed.
A successful build does not establish audio behavior or realtime safety. Update
traceability with code symbols and meaningful tests as work lands.

## Task authorization and onboarding boundary

The later foundational architecture request authorizes the shared engine, JUCE
plugin targets and meaningful tests. It does not define the unavailable PRD law
language or authorize inventing FR/NFR/AT definitions. Preserve stable identities
in docs/BUILD_AND_ARCHITECTURE.md and keep the engine shared. For
cloud onboarding, preserve tracked source, tests, manifests and lockfiles; keep
setup helpers/generated artifacts outside the checkout or in supported ignored
locations. Report unavailable source and external blockers honestly.
