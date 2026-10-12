#pragma once
#include "Events/Events.h"
#include <array>
namespace paradox {
class InstrumentRenderer {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void apply(Event) noexcept;
    float next() noexcept;
private:
    struct Voice { bool active {}; std::uint8_t note {}, channel {}; double phase {}, step {}; float velocity {}; };
    std::array<Voice, 16> voices {};
    double rate = 44100.0;
    std::size_t stealIndex {};
};
struct EffectRenderer { static float render(float input, float gain) noexcept { return input * gain; } };
}
