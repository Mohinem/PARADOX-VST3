#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
namespace paradox {
enum class EventType { noteOn, noteOff, allNotesOff };
struct Event {
    std::uint32_t offset {};
    EventType type {};
    std::uint8_t note {}, channel {};
    float velocity {};
};
class EventBuffer {
public:
    static constexpr std::size_t capacity = 256;
    void clear() noexcept { size = 0; overflow = false; }
    // Stable ordering: equal-offset events retain insertion order.
    bool push(Event) noexcept;
    std::span<const Event> view() const noexcept { return {events.data(), size}; }
    bool overflowed() const noexcept { return overflow; }
private:
    std::array<Event, capacity> events {};
    std::size_t size {};
    bool overflow {};
};
}
