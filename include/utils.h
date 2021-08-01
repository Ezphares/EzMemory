#pragma once

#include <cstdint>
#include <cstdlib>

namespace EzMemory {

void *align_f(void *ptr, std::size_t alignment)
{
    std::uintptr_t t = reinterpret_cast<std::uintptr_t>(ptr);

    t -= 1;
    t /= alignment;
    t += 1;
    t *= alignment;

    return reinterpret_cast<void *>(t);
}

} // namespace EzMemory