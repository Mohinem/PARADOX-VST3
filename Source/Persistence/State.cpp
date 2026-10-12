#include "State.h"
#include <bit>
#include <limits>
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
namespace paradox {
StateBytes encodeState(LawModel model) noexcept {
    StateBytes bytes {'P', 'X', 'S', 'T', 1, 0, 0, 0};
    const auto bits = std::bit_cast<std::uint32_t>(model.gain);
    for (int i = 0; i < 4; ++i) bytes[8+i] = static_cast<std::uint8_t>(bits >> (i * 8));
    return bytes;
}
std::optional<LawModel> decodeState(std::span<const std::uint8_t> b) noexcept {
    if (b.size() != 12 || b[0]!='P' || b[1]!='X' || b[2]!='S' || b[3]!='T'
        || b[4]!=1 || b[5]!=0 || b[6]!=0 || b[7]!=0) return {};
    std::uint32_t bits = 0;
    for (int i = 0; i < 4; ++i) bits |= static_cast<std::uint32_t>(b[8+i]) << (i*8);
    LawModel model {std::bit_cast<float>(bits)};
    if (!LawCompiler::compile(model)) return {};
    return model;
}
}
