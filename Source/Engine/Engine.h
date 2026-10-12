#pragma once
#include "Law/Law.h"
#include "Rendering/Renderers.h"
#include <cstdint>
namespace paradox {
enum class Backend { instrument, effect };
struct EvaluationState { float gain = 0.2f; std::uint64_t samplesProcessed {}; };
class LawEngine {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    // Audio-thread owned POD. No graph publication/lifetime protocol implied.
    void adopt(CompiledLaw law) noexcept;
    void setGain(float gain) noexcept;
    void process(Backend, float* const* channels, int channelCount, int samples,
                 std::span<const Event>) noexcept;
    const EvaluationState& state() const noexcept { return evaluation; }
private:
    EvaluationState evaluation {};
    InstrumentRenderer instrument;
};
}
