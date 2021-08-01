#pragma once

#include "AllocatorConcepts.h"

#include <cassert>
#include <cstring>

namespace EzMemory {

template <
    typename BackingAllocator,
    std::size_t BlockSize,
    std::size_t BlockCount,
    std::size_t BlockAlign = 0>
requires(IsAllocator<BackingAllocator> &&AllocatorCanOptionallyAlign<
         BackingAllocator,
         BlockAlign>) class PoolAllocator
{
    static_assert(
        BlockSize >= sizeof(void *),
        "Pool allocator block size must exceed pointer size.");

public:
    PoolAllocator(BackingAllocator *backingAllocator)
        : m_backingAllocator(backingAllocator)
    {
        m_buffer = _optionally_aligned<BackingAllocator, BlockAlign>::allocate(
            m_backingAllocator, BlockSize * BlockCount);

        reset();
    }

    ~PoolAllocator()
    {
        _optionally_aligned<BackingAllocator, BlockAlign>::deallocate(
            m_backingAllocator, m_buffer);
    }

public:
    void *allocate(std::size_t size)
    {
        assert(size <= BlockSize);

        void *p = m_head;

        if (m_head)
        {
            std::memcpy(&m_head, m_head, sizeof(std::uintptr_t));
        }

        return p;
    }

    void deallocate(void *ptr)
    {
        assert(owns(ptr));

        std::memcpy(ptr, &m_head, sizeof(std::uintptr_t));

        m_head = ptr;
    }

    void *allocate_aligned(std::size_t size, std::size_t alignment)
    {
        assert(alignment <= BlockAlign);
        return allocate(size);
    }

    void deallocate_aligned(void *ptr)
    {
        deallocate(ptr);
    }

    bool owns(void *ptr)
    {
        std::uintptr_t uPtr = reinterpret_cast<std::uintptr_t>(ptr);
        std::uintptr_t uBuf = reinterpret_cast<std::uintptr_t>(m_buffer);

        return uPtr >= uBuf && uPtr < uBuf + BlockSize * BlockCount;
    }

    void reset()
    {
        m_head = m_buffer;

        std::uintptr_t uPtr = reinterpret_cast<std::uintptr_t>(m_buffer);

        while (uPtr)
        {
            std::uintptr_t uNext = uPtr + BlockSize;

            if (!owns(reinterpret_cast<void *>(uNext)))
            {
                uNext = 0;
            }

            std::memcpy(
                reinterpret_cast<void *>(uPtr), &uNext, sizeof(std::uintptr_t));

            uPtr = uNext;
        }
    }

private:
    BackingAllocator *m_backingAllocator;
    void *m_buffer;
    void *m_head;
};

} // namespace EzMemory
