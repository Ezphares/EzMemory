#pragma once

#include "AllocatorConcepts.h"

namespace EzMemory {

template <typename BackingAllocator>
requires(IsAllocator<BackingAllocator> &&
             AllocatorCanReset<BackingAllocator>) struct DoubleBufferedAllocator
{
public:
    DoubleBufferedAllocator(
        BackingAllocator *backing1, BackingAllocator *backing2)
        : m_current(0)
    {
        m_backingAllocators[0] = backing1;
        m_backingAllocators[1] = backing2;

        reset();
    }

    void *allocate(std::size_t size)
    {
        return m_currentAllocator->allocate(size);
    }

    void deallocate(void *)
    {
        return;
    }

    template <typename = std::enable_if_t<AllocatorCanAlign<BackingAllocator>>>
    void *allocate_aligned(std::size_t size, std::size_t align)
    {
        return m_currentAllocator->allocate_aligned(size, align);
    }

    void deallocate_aligned(void *)
    {
        return false;
    }

    template <
        typename = std::enable_if_t<AllocatorKnowsOwnership<BackingAllocator>>>
    bool owns(void *ptr)
    {
        return m_backingAllocators[0]->owns(ptr)
               || m_backingAllocators[1]->owns(ptr);
    }

    void reset()
    {
        m_backingAllocators[0]->reset();
        m_backingAllocators[1]->reset();
        m_current          = 0;
        m_currentAllocator = m_backingAllocators[0];
    }

    void flip()
    {
        m_current          = -m_current + 1;
        m_currentAllocator = m_backingAllocators[m_current];
        m_currentAllocator->reset();
    }

private:
    BackingAllocator *m_backingAllocators[2];
    BackingAllocator *m_currentAllocator;
    int m_current;
};
} // namespace EzMemory