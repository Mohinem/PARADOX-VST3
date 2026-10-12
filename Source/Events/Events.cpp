#include "Events.h"
namespace paradox {
bool EventBuffer::push(Event e) noexcept {
    if (size == capacity) { overflow = true; return false; }
    auto i = size++;
    while (i > 0 && events[i - 1].offset > e.offset) { events[i] = events[i - 1]; --i; }
    events[i] = e;
    return true;
}
}
