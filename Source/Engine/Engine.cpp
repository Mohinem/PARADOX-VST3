#include "Engine.h"
#include <algorithm>
#include <cmath>
namespace paradox {
void LawEngine::prepare(double rate) noexcept { instrument.prepare(rate); evaluation.samplesProcessed = 0; }
void LawEngine::reset() noexcept { instrument.reset(); evaluation.samplesProcessed = 0; }
void LawEngine::adopt(CompiledLaw law) noexcept { setGain(law.gain); }
void LawEngine::setGain(float gain) noexcept { if (std::isfinite(gain)) evaluation.gain = std::clamp(gain, 0.0f, 1.0f); }
void LawEngine::process(Backend backend, float* const* channels, int count, int samples,
                        std::span<const Event> events) noexcept {
    std::size_t index = 0;
    for (int s = 0; s < samples; ++s) {
        while (index < events.size() && events[index].offset <= static_cast<std::uint32_t>(s))
            instrument.apply(events[index++]);
        const auto generated = backend == Backend::instrument ? instrument.next() * evaluation.gain : 0.0f;
        for (int c = 0; c < count; ++c)
            channels[c][s] = backend == Backend::instrument ? generated : EffectRenderer::render(channels[c][s], evaluation.gain);
    }
    evaluation.samplesProcessed += static_cast<std::uint64_t>(std::max(samples, 0));
}
}
