#pragma once

#include "AllocatorConcepts.h"

namespace EzMemory {

template <typename PrimaryAllocator, typename SecondaryAllocator>
requires(
    IsAllocator<PrimaryAllocator> &&AllocatorKnowsOwnership<PrimaryAllocator>
        &&IsAllocator<PrimaryAllocator>) struct FallbackAllocator
{
public:
    FallbackAllocator(PrimaryAllocator *primary, SecondaryAllocator *secondary)
        : m_primary(primary)
        , m_secondary(secondary)
    {}

    void *allocate(std::size_t size)
    {
        void *p = m_primary->allocate(size);

        if (!p)
        {
            p = m_secondary->allocate(size);
        }

        return p;
    }

    void deallocate(void *ptr)
    {
        if (m_primary->owns(ptr))
        {
            m_primary->deallocate(ptr);
        }
        else
        {
            m_secondary->deallocate(ptr);
        }
    }

    template <
        typename = std::enable_if_t<
            AllocatorCanAlign<
                PrimaryAllocator> && AllocatorCanAlign<SecondaryAllocator>>>
    void *allocate_aligned(std::size_t size, std::size_t align)
    {
        void *p = m_primary->allocate_aligned(size, align);

        if (!p)
        {
            p = m_secondary->allocate_aligned(size, align);
        }

        return p;
    }

    template <
        typename = std::enable_if_t<
            AllocatorCanAlign<
                PrimaryAllocator> && AllocatorCanAlign<SecondaryAllocator>>>
    void deallocate_aligned(void *ptr)
    {
        if (m_primary->owns(ptr))
        {
            m_primary->deallocate_aligned(ptr);
        }
        else
        {
            m_secondary->deallocate_aligned(ptr);
        }
    }

    template <
        typename
        = std::enable_if_t<AllocatorKnowsOwnership<SecondaryAllocator>>>
    bool owns(void *ptr)
    {
        return m_primary->owns(ptr) || m_secondary->owns(ptr);
    }

    template <
        typename = std::enable_if_t<
            AllocatorCanReset<
                PrimaryAllocator> && AllocatorCanReset<SecondaryAllocator>>>
    void reset()
    {
        m_primary->reset();
        m_secondary->reset();
    }

private:
    PrimaryAllocator *m_primary;
    SecondaryAllocator *m_secondary;
};

} // namespace EzMemory
