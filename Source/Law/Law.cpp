#include "Law.h"
#include <cmath>
namespace paradox {
Compilation LawCompiler::compile(const LawModel& model) noexcept {
    if (!std::isfinite(model.gain) || model.gain < 0.0f || model.gain > 1.0f)
        return {{}, ValidationError::invalidGain};
    return {{model.gain}, ValidationError::none};
}
}
