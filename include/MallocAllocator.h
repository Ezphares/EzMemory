#pragma once

#include "utils.h"

#include <cstddef>
#include <cstdlib>

namespace EzMemory {
class MallocAllocator
{
public:
    void *allocate(std::size_t size)
    {
        return std::malloc(size);
    }

    void *allocate_aligned(std::size_t size, std::size_t alignment)
    {
        const constexpr size_t max_align = alignof(std::max_align_t);
        std::size_t bufSize = alignment - max_align + sizeof(void *);

        std::uintptr_t p
            = reinterpret_cast<std::uintptr_t>(std::malloc(size + bufSize));

        if (!p)
        {
            return nullptr;
        }

        void *d = align_f((void *)(p + sizeof(void *)), alignment);

        uintptr_t *bk = reinterpret_cast<uintptr_t *>(
            reinterpret_cast<std::uintptr_t>(d) - sizeof(void *));

        *bk = p;

        return d;
    }

    void deallocate(void *ptr)
    {
        std::free(ptr);
    }

    void deallocate_aligned(void *ptr)
    {
        uintptr_t *bk = reinterpret_cast<uintptr_t *>(
            reinterpret_cast<std::uintptr_t>(ptr) - sizeof(void *));

        void *p = reinterpret_cast<void *>(*bk);

        free(p);
    }
};

} // namespace EzMemory