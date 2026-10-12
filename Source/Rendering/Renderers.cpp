#include "Renderers.h"
#include <cmath>
#include <algorithm>
#include <numbers>
namespace paradox {
void InstrumentRenderer::prepare(double sr) noexcept { rate = std::isfinite(sr) && sr > 0 ? sr : 44100.0; reset(); }
void InstrumentRenderer::reset() noexcept { voices = {}; stealIndex = 0; }
void InstrumentRenderer::apply(Event e) noexcept {
    if (e.type == EventType::allNotesOff) {
        for (auto& v : voices) if (e.channel == 255 || v.channel == e.channel) v.active = false;
        return;
    }
    if (e.note > 127 || e.channel > 15 || !std::isfinite(e.velocity)) return;
    if (e.type == EventType::noteOff || e.velocity <= 0) {
        for (auto& v : voices) if (v.note == e.note && v.channel == e.channel) v.active = false;
        return;
    }
    Voice* selected = nullptr;
    for (auto& v : voices) if (!v.active) { selected = &v; break; }
    if (!selected) { selected = &voices[stealIndex]; stealIndex = (stealIndex + 1) % voices.size(); }
    *selected = {true, e.note, e.channel, 0.0,
        440.0 * std::exp2((static_cast<int>(e.note) - 69) / 12.0) / rate,
        std::min(e.velocity, 1.0f)};
}
float InstrumentRenderer::next() noexcept {
    float result = 0;
    for (auto& v : voices) if (v.active) {
        result += static_cast<float>(std::sin(v.phase * 2 * std::numbers::pi)) * v.velocity / 16.0f;
        v.phase += v.step;
        v.phase -= std::floor(v.phase);
    }
    return result;
}
}
