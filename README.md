# PARADOX foundation

Two independently discoverable VST3 products share one JUCE-independent C++20
engine. This is an architectural foundation: a validated gain law, a basic
16-voice sine instrument, and a gain effect. The original PRD is unavailable;
this does not claim implementation of its unidentified law semantics.

See [build and architecture](docs/BUILD_AND_ARCHITECTURE.md),
[implementation plan](docs/IMPLEMENTATION_PLAN.md), and
[requirements traceability](docs/REQUIREMENTS_TRACEABILITY.md).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure
```

Headless engine only (no JUCE or GUI/audio development packages needed):

```sh
cmake -S . -B build-headless -DPARADOX_BUILD_PLUGINS=OFF
cmake --build build-headless
ctest --test-dir build-headless --output-on-failure
```

JUCE is pinned to 8.0.6. Review JUCE and bundled VST3 SDK licensing before
redistribution; this repository does not grant rights to those dependencies.
