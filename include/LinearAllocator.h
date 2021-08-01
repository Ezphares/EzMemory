#pragma once

#include "AllocatorConcepts.h"
#include "utils.h"

namespace EzMemory {

template <typename BackingAllocator, std::size_t BufferSize>
requires(IsAllocator<BackingAllocator>) struct LinearAllocator
{
public:
    LinearAllocator(BackingAllocator *backingAllocator)
        : m_backingAllocator(backingAllocator)
    {
        m_buffer = m_backingAllocator->allocate(BufferSize);
        reset();
    }

    ~LinearAllocator()
    {
        m_backingAllocator->deallocate(m_buffer);
    }

    void *allocate(std::size_t size)
    {
        void *d = m_head;

        std::uintptr_t uBuf    = reinterpret_cast<std::uintptr_t>(m_buffer);
        std::uintptr_t newHead = (reinterpret_cast<std::uintptr_t>(d) + size);
        if (newHead > uBuf + BufferSize)
        {
            return nullptr;
        }

        m_head = reinterpret_cast<void *>(newHead);
        return d;
    }

    void *allocate_aligned(std::size_t size, std::size_t align)
    {
        void *d = align_f(m_head, align);

        void *newHead = reinterpret_cast<void *>(
            reinterpret_cast<std::uintptr_t>(d) + size);
        if (!owns(newHead))
        {
            return nullptr;
        }

        m_head = newHead;
        return d;
    }

    void deallocate(void *)
    {
        return;
    }

    void deallocate_aligned(void *)
    {
        return;
    }

    bool owns(void *ptr)
    {
        std::uintptr_t uPtr = reinterpret_cast<std::uintptr_t>(ptr);
        std::uintptr_t uBuf = reinterpret_cast<std::uintptr_t>(m_buffer);

        return uPtr >= uBuf && uPtr < uBuf + BufferSize;
    }

    void reset()
    {
        m_head = m_buffer;
    }

private:
    BackingAllocator *m_backingAllocator;
    void *m_buffer;
    void *m_head;
};
} // namespace EzMemory