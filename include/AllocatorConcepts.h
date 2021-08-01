#pragma once

#include <cstddef>

namespace EzMemory {

template <typename T>
concept IsAllocator = requires(T x, void *ptr, std::size_t s)
{
    ptr = x.allocate(s);
    x.deallocate(ptr);
};

template <typename T>
concept AllocatorCanAlign
    = requires(T x, void *ptr, std::size_t s, std::size_t a)
{
    ptr = x.allocate_aligned(s, a);
    x.deallocate_aligned(ptr);
};

template <typename T>
concept AllocatorKnowsOwnership = requires(T x, void *ptr, bool result)
{
    result = x.owns(ptr);
};

template <typename T>
concept AllocatorCanReset = requires(T x)
{
    x.reset();
};

template <typename T, std::size_t Align>
concept AllocatorCanOptionallyAlign = (Align == 0 || AllocatorCanAlign<T>);

template <typename T, std::size_t Align>
struct _optionally_aligned
{
    static void *allocate(T *allocator, std::size_t size)
    {
        return allocator->allocate_aligned(size, Align);
    }
    static void deallocate(T *allocator, void *ptr)
    {
        return allocator->deallocate_aligned(ptr);
    }
};

template <typename T>
struct _optionally_aligned<T, 0>
{
    static void *allocate(T *allocator, std::size_t size)
    {
        return allocator->allocate(size);
    }
    static void deallocate(T *allocator, void *ptr)
    {
        return allocator->deallocate(ptr);
    }
};

} // namespace EzMemory