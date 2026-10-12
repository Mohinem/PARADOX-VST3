#include "Engine/Engine.h"
#include "Persistence/State.h"
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include "AllocationProbe.h"
static int checks {};
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
int main() {
    using namespace paradox;
    check(bool(LawCompiler::compile({0.5f})), "valid law");
    check(!LawCompiler::compile({-1}), "negative gain rejected");
    check(!LawCompiler::compile({std::numeric_limits<float>::quiet_NaN()}), "NaN rejected");
    EventBuffer events;
    events.push({8, EventType::noteOff, 69, 0, 0});
    events.push({4, EventType::noteOn, 69, 0, 1});
    events.push({4, EventType::noteOn, 70, 0, 1});
    check(events.view()[0].note == 69 && events.view()[1].note == 70, "stable sample ordering");
    events.clear();
    for (std::size_t i=0; i<EventBuffer::capacity; ++i) check(events.push({}), "bounded capacity accepts");
    check(!events.push({}) && events.overflowed(), "overflow reported");
    events.clear();
    events.push({4, EventType::noteOn, 69, 0, 1});
    events.push({20, EventType::noteOff, 69, 0, 0});
    LawEngine whole, split;
    whole.prepare(48000); split.prepare(48000);
    whole.adopt(LawCompiler::compile({0.5f}).law); split.setGain(0.5f);
    std::array<float, 32> a {}, b {};
    float* pa[] {a.data()}; float* pb[] {b.data()};
    probeActive = true;
    whole.process(Backend::instrument, pa, 1, 32, events.view());
    probeActive = false;
    check(allocationCount == 0, "engine callback has no C++ heap allocations");
    for (int i=0; i<4; ++i) check(a[i] == 0, "silence before note offset");
    check(a[5] != 0, "instrument generates audio");
    for (int i=20; i<32; ++i) check(a[i] == 0, "note-off sample boundary");
    EventBuffer first, second;
    first.push({4, EventType::noteOn, 69, 0, 1});
    second.push({4, EventType::noteOff, 69, 0, 0});
    split.process(Backend::instrument, pb, 1, 16, first.view());
    pb[0] = b.data()+16;
    split.process(Backend::instrument, pb, 1, 16, second.view());
    check(a == b, "block partition invariance");
    check(split.state().samplesProcessed == 32, "sample clock");
    a.fill(0.8f); b.fill(-0.6f);
    float* stereo[] {a.data(), b.data()};
    whole.process(Backend::effect, stereo, 2, 32, {});
    check(std::abs(a[0]-0.4f)<1e-6f && std::abs(b[31]+0.3f)<1e-6f, "effect preserves channels with shared gain semantics");
    auto bytes = encodeState({0.7f});
    auto model = decodeState(bytes);
    check(model && model->gain == 0.7f, "state round trip");
    bytes[4] = 2;
    check(!decodeState(bytes), "unknown state version rejected");
    check(!decodeState({bytes.data(), 3}), "truncated state rejected");
    bytes = encodeState({std::numeric_limits<float>::infinity()});
    check(!decodeState(bytes), "invalid state gain rejected");
    whole.reset(); a.fill(1);
    whole.process(Backend::instrument, pa, 1, 32, {});
    check(a[5] == 0, "reset silences voices");
    std::cout << checks << " checks passed\n";
}
