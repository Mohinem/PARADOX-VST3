#pragma once
#include <atomic>
#include <cstdlib>
#include <new>
#if defined(_WIN32)
#include <malloc.h>
#endif
inline thread_local bool probeActive {};
inline thread_local unsigned allocationCount {};
void* operator new(std::size_t size) {
    if (probeActive) ++allocationCount;
    if (auto* p = std::malloc(size ? size : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t s) { return ::operator new(s); }
void* operator new(std::size_t s, std::align_val_t alignment) {
    if (probeActive) ++allocationCount;
#if defined(_WIN32)
    if (auto* p = _aligned_malloc(s ? s : 1, static_cast<std::size_t>(alignment))) return p;
#else
    void* p = nullptr;
    if (posix_memalign(&p, static_cast<std::size_t>(alignment), s ? s : 1) == 0) return p;
#endif
    throw std::bad_alloc();
}
void* operator new[](std::size_t s, std::align_val_t a) { return ::operator new(s, a); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
void freeAligned(void* p) noexcept {
#if defined(_WIN32)
    _aligned_free(p);
#else
    std::free(p);
#endif
}
void operator delete(void* p, std::align_val_t) noexcept { freeAligned(p); }
void operator delete[](void* p, std::align_val_t) noexcept { freeAligned(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { freeAligned(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { freeAligned(p); }
