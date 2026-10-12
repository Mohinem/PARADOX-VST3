# Build, plugin contracts and architecture

## Dependencies and build

C++20, CMake >=3.22, and JUCE 8.0.6 at commit
`51a8a6d7aeae7326956d747737ccf1575e61e209`. Default CMake configuration fetches
that exact commit. An existing checkout may be provided with
`-DPARADOX_JUCE_PATH=/path/to/JUCE`; use the same revision. No third-party
headless test framework is needed. Build and run CTest as shown in README.md.

Linux GUI/plugin builds require development packages for ALSA, FreeType,
Fontconfig, X11, Xrandr, Xinerama, Xcursor, Xext and OpenGL, alongside compiler,
CMake and optionally Ninja. Browser and CURL support are disabled. On a normal
Debian installation the packages are `cmake ninja-build libasound2-dev
libfreetype-dev libfontconfig-dev libx11-dev libxrandr-dev libxinerama-dev
libxcursor-dev libxext-dev libgl-dev`.

In this cloud instance system installation is read-only. Signed Debian packages
were downloaded and extracted under `/workspace/toolchain/root`. Source
`/workspace/toolchain/activate.sh` before configuration, build or tests. The
prepared JUCE checkout is `/workspace/shared/JUCE-8.0.6`. Configure with
`-G Ninja -DCMAKE_BUILD_TYPE=Release
-DPARADOX_JUCE_PATH=/workspace/shared/JUCE-8.0.6
-DCMAKE_PREFIX_PATH=/workspace/toolchain/root/usr`. These local locations are
cloud setup details, not portable project requirements. Three parallel build
jobs were chosen for the five available CPUs and JUCE's large translation units.

## Stable product and parameter identities

There was no existing code or published identifier baseline to preserve. The
following are the initial stable identities. Do not rename product targets,
manufacturer/plugin codes, bundle IDs, parameter string IDs or version hints
without an explicit compatibility decision. JUCE derives VST3 class IDs from
these definitions; the initial generated Linux IDs below are recorded and must remain stable.

| Contract | Instrument | Effect |
| --- | --- | --- |
| CMake target | ParadoxInstrument | ParadoxEffect |
| Product | PARADOX Instrument | PARADOX Effect |
| Manufacturer code | Prdx | Prdx |
| Plugin code | PxIn | PxFx |
| Bundle ID | audio.paradox.ParadoxInstrument | audio.paradox.ParadoxEffect |
| Format/category | VST3 / Instrument;Synth | VST3 / Fx |
| Default audio buses | No input, stereo output | Stereo input, stereo output |
| Supported audio layouts | Mono or stereo output; no input | Matching mono or stereo input/output |
| MIDI input/output | Yes / no | Yes / no |
| Sidechains/surround | Unsupported | Unsupported |
| Reported latency/tail | Zero / zero | Zero / zero |

Initial generated VST3 class IDs (JUCE's module-info output):

| Class | Instrument | Effect |
| --- | --- | --- |
| Audio component | ABCDEF019182FAEB507264785078496E | ABCDEF019182FAEB5072647850784678 |
| Edit controller | ABCDEF011234ABCD507264785078496E | ABCDEF011234ABCD5072647850784678 |

Verify corresponding IDs on Windows/macOS before release; their binaries have
not been built in this instance.

Both publish `output_gain` (version hint 1), linear 0–1, default 0.2. JUCE's VST3
wrapper supplies its standard host integration parameters; custom parameter IDs
must remain unique. Parameter values are sampled once per process block. This
foundation does not promise sample-accurate gain automation or smoothed gain.
One program is exposed; there is no preset browser. VST2 replacement is disabled.
No automatic copying into the host's plugin directory occurs.

Bundles are under `build/ParadoxInstrument_artefacts/Release/VST3/PARADOX
Instrument.vst3` and `build/ParadoxEffect_artefacts/Release/VST3/PARADOX Effect.vst3`
for the documented single-config Release build. These are distinct binaries and
factory identities, with one engine instance per processor. The shared static
engine library is linked into both binaries; there is one engine source and
semantic implementation, not an OS-shared singleton or cross-plugin state.

## Module boundaries

| Module | Responsibility |
| --- | --- |
| Source/Law | Domain model, validation and control-plane compiler interface |
| Source/Events | Fixed-capacity sample-offset events and stable ordering |
| Source/Engine | Shared gain evaluation, sample clock and backend dispatch |
| Source/Rendering | Fixed 16-voice sine renderer and input-audio gain renderer |
| Source/Persistence | Versioned binary state encoding and validated decoding |
| Source/Plugin | JUCE processor, host buses, raw MIDI/parameter adapters |
| Source/UI | Gain control and polling peak display |

Core modules have no JUCE dependency. The plugin adapter and editor cannot
supply arbitrary graph objects to the callback. `LawCompiler::compile` currently
produces a small `CompiledLaw` POD, adopted only by its owning audio thread.
There is no dynamic graph replacement in this foundation. Future graph
compilation must occur off-thread, with a bounded publication protocol and
explicit off-thread retirement; changing this POD into a shared graph pointer
without a lifetime protocol would violate the design.

## Timing, rendering and realtime contracts

`EventBuffer` has capacity 256. Insertion order breaks ties at the same sample
offset. `LawEngine::process` consumes a sorted span with offsets within the
current block; the caller owns the span/storage and must not modify it during
processing. The JUCE adapter filters offsets outside the block. Events do not
carry across blocks. Zero-velocity note-on means note-off. CC120/123 stops voices
on its MIDI channel. On buffer overflow the adapter drops the block's events
and stops all voices at sample zero, preventing stuck notes. A zero-sample block
does not advance the clock.

The instrument uses 16 fixed voices, first free voice then round-robin stealing.
Note-off stops matching note/channel voices immediately. Sine output is normalized
by maximum voice count; there is no envelope, release, pitch bend, sustain pedal
or anti-click transition yet. Effect output is channel-preserving input times
shared evaluated gain; it accepts MIDI but currently MIDI does not modulate gain.
Unsupported messages, including SysEx, are ignored by reading non-owning bytes.

After prepare, the callback uses stack/member buffers, bounded event storage,
fixed voices and lock-free float atomics. It performs no graph parsing, filesystem
operations, mutex calls or explicit allocation/deallocation. UI string work and
state serialization are outside `processBlock`. The allocation tests instrument
C++ new/new[]; they do not prove absence of all libc allocations, operating-system
locks or realtime deadline misses in every host wrapper. MIDI input iteration
scales with the host-supplied buffer; accepted engine events are bounded.

State is exactly 12 bytes: `PXST`, little-endian uint32 schema version 1, and
little-endian IEEE float32 gain. Invalid size/magic/version/nonfinite/out-of-range
gain is rejected without applying state. Current state contains only gain;
voices, phase, event buffers, meters and sample counters are transient. Host
restore updates the parameter, consumed on the next block, without mutating
renderer state across threads. Future schema changes need explicit migrations.

## Actual validation results

Linux x86_64 Release, GCC 14.2.0, CMake 3.31.6, JUCE 8.0.6:

- Both VST3 targets compiled and generated module-info through factory loading.
- `shared_engine`: passed, 287 executed checks.
- `ParadoxInstrument_processor`: passed.
- `ParadoxEffect_processor`: passed.
- Full CTest run: 3/3 passed; no tests skipped or disabled.
- The separately configured JUCE-free headless workflow also passed (1/1).
- Thread-local C++ allocation probes observed zero callback allocations on the
  exercised first-block engine and processor paths. This is limited coverage,
  not a complete realtime certification.

## Validation scope

CTest contains the JUCE-independent engine executable and one processor factory
smoke test for each product. Processor tests construct the actual target's
processor and exercise buses, parameter identity, save/restore and MIDI/audio
processing. They do not instantiate a VST3 host wrapper or open an editor. The
VST3 build's module-info generation loads the resulting module factory; it does
not validate a DAW's scanner or playback behavior.

DAW scan/load, MIDI routing, audio routing, host automation, offline bounce,
UI open/close, multiple instances, suspend/resume, platform-specific packaging,
Windows/macOS builds and long-running realtime performance require later checks.
PRD FR/NFR/AT status remains unassessed until the source PRD is supplied.
