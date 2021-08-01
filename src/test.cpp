#include <DoubleBufferedAllocator.h>
#include <FallbackAllocator.h>
#include <LinearAllocator.h>
#include <MallocAllocator.h>
#include <PoolAllocator.h>

#include <iostream>

using namespace EzMemory;

// Static checks
// Malloc is an allocator that can align...
static_assert(IsAllocator<MallocAllocator>);
static_assert(AllocatorCanAlign<MallocAllocator>);
// ... but does not know ownership or reset
static_assert(!AllocatorKnowsOwnership<MallocAllocator>);
static_assert(!AllocatorCanReset<MallocAllocator>);

typedef PoolAllocator<MallocAllocator, 16, 4> PoolAlloc;
// Pool allocators have all features
static_assert(IsAllocator<PoolAlloc>);
static_assert(AllocatorCanAlign<PoolAlloc>);
static_assert(AllocatorKnowsOwnership<PoolAlloc>);
static_assert(AllocatorCanReset<PoolAlloc>);

typedef LinearAllocator<MallocAllocator, 1024> LinAlloc;
// Linear allocators have all features
static_assert(IsAllocator<LinAlloc>);
static_assert(AllocatorCanAlign<LinAlloc>);
static_assert(AllocatorKnowsOwnership<LinAlloc>);
static_assert(AllocatorCanReset<LinAlloc>);

typedef FallbackAllocator<PoolAlloc, MallocAllocator> FallbackAlloc;
typedef FallbackAllocator<PoolAlloc, LinAlloc> FallbackToLin;

// Fallback allocators inherit capabilities from their least capable backing
// allocator
static_assert(IsAllocator<FallbackAlloc>);
static_assert(AllocatorCanAlign<FallbackAlloc>);
static_assert(!AllocatorKnowsOwnership<FallbackAlloc>);
static_assert(!AllocatorCanReset<FallbackAlloc>);

static_assert(IsAllocator<FallbackToLin>);
static_assert(AllocatorCanAlign<FallbackToLin>);
static_assert(AllocatorKnowsOwnership<FallbackToLin>);
static_assert(AllocatorCanReset<FallbackToLin>);

typedef DoubleBufferedAllocator<LinAlloc> DblAlloc;

// Double buffered allocators inherit capabilities from their backing
// allocator (but require all features except for alignment)
static_assert(IsAllocator<DblAlloc>);
static_assert(AllocatorCanAlign<DblAlloc>);
static_assert(AllocatorKnowsOwnership<DblAlloc>);
static_assert(AllocatorCanReset<DblAlloc>);

void testPoolAlloc()
{
    MallocAllocator m;
    PoolAlloc p(&m);

    // Check the pool allocator
    assert(p.allocate(16));
    void *pa1 = p.allocate(16);
    assert(pa1);
    assert(p.allocate(16));
    assert(p.allocate(16));
    // Full - deallocations should make room
    assert(!p.allocate(16));
    p.deallocate(pa1);
    assert(p.allocate(16));
    p.reset();
}

void testFallbackAllocator_PoolToMalloc()
{
    MallocAllocator m;
    PoolAlloc p(&m);
    FallbackAlloc fb(&p, &m);

    // Check the fallback allocator
    assert(fb.allocate(16));
    assert(fb.allocate(16));
    assert(fb.allocate(16));
    assert(fb.allocate(16));
    // Primary allocator is now full
    assert(fb.allocate(16));
}

void testLinearAllocator()
{
    MallocAllocator m;
    LinAlloc l(&m);

    // Check that linear allocators can use their entire buffer
    assert(l.allocate(512));
    assert(l.allocate(512));
    assert(!l.allocate(512));
    l.reset();
    assert(l.allocate(512));
    l.reset();

    // Check that failed asserts do not consume the buffer
    assert(!l.allocate(1025));
    assert(l.allocate(1024));
    l.reset();
}

void testDoubleBufferedAllocator_Linear()
{
    MallocAllocator m;
    LinAlloc l1(&m);
    LinAlloc l2(&m);
    DblAlloc db(&l1, &l2);

    // Check the double buffer
    assert(db.allocate(1024));
    assert(!db.allocate(1));
    db.flip();
    assert(db.allocate(1024));
    assert(!db.allocate(1));
    db.flip();
    assert(db.allocate(1024));
    db.reset();
}

int main()
{
    testPoolAlloc();
    testFallbackAllocator_PoolToMalloc();
    testLinearAllocator();
    testDoubleBufferedAllocator_Linear();

    return 0;
}