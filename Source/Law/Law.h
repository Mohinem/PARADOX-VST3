#pragma once
#include <cstdint>
namespace paradox {
// Foundation only: the PRD's law language is not yet available.
struct LawModel { float gain = 0.2f; };
struct CompiledLaw { float gain = 0.2f; };
enum class ValidationError { none, invalidGain };
struct Compilation { CompiledLaw law {}; ValidationError error {}; explicit operator bool() const noexcept { return error == ValidationError::none; } };
class LawCompiler {
public:
    // Control-plane interface. Future graph compilers must not run in processBlock.
    static Compilation compile(const LawModel&) noexcept;
};
}
