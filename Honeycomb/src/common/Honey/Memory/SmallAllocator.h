// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Memory/Pool.h"

namespace honey
{

/// Memory pool tailored for small block allocation
class SmallMemPool : mt::NoCopy
{
public:
    /// Get singleton
    mt_staticObj(SmallMemPool, inst,);

    ~SmallMemPool() {}

    MemPool& pool()                             { return *_pool; }

private:
    SmallMemPool()                              : _pool(&createPool()) { _pool->setId("Small"); }

    /// Creates the pool used by SmallMemPool. To provide a custom pool, define SMALL_ALLOCATOR_CUSTOM and a createPool() implementation.
    static MemPool& createPool();

    UniquePtr<MemPool> _pool;
};

/// SmallMemPool allocator
template<class T>
class SmallAllocator : public MemPoolAllocator<SmallAllocator, T>
{
public:
    SmallAllocator()                            {}
    SmallAllocator(const SmallAllocator&)       {}
    template<class U>
    SmallAllocator(const SmallAllocator<U>&)    {}

    MemPool& pool()                             { return SmallMemPool::inst().pool(); }
};

/// Inherit from this class to use the small block allocator
typedef AllocatorObject<SmallAllocator> SmallAllocatorObject;


#ifndef SMALL_ALLOCATOR_CUSTOM
    /// Default implementation
    inline MemPool& SmallMemPool::createPool()
    {
        MemPool::Factory factory;
        factory.addBucket(8, 5000);
        factory.addBucket(16, 2000);
        factory.addBucket(32, 2000);
        factory.addBucket(64, 2000);
        factory.addBucket(128, 500);
        factory.addBucket(256, 100);
        factory.addBucket(512, 50);
        return factory.create();
    }
#endif

}
