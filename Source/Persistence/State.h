#pragma once
#include "Law/Law.h"
#include <array>
#include <cstdint>
#include <optional>
#include <span>
namespace paradox {
// Explicit endian-independent wire format: PXST, uint32 version, float32 gain.
using StateBytes = std::array<std::uint8_t, 12>;
StateBytes encodeState(LawModel) noexcept;
std::optional<LawModel> decodeState(std::span<const std::uint8_t>) noexcept;
}
