// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Memory/Pool.h"

namespace honey
{

MemPool& SmallAllocator_createSingleton();

/// Global allocator for small memory blocks.  To provide a custom pool define `SmallAllocator_createSingleton_` and implement SmallAllocator_createSingleton().
template<class T>
class SmallAllocator : public MemPoolAllocator<SmallAllocator, T>
{
public:
    SmallAllocator() {}
    SmallAllocator(const SmallAllocator&) {}
    template<class U>
    SmallAllocator(const SmallAllocator<U>&) {}

    static MemPool& pool()                              { static UniquePtr<MemPool> inst = &createSingleton(); return *inst; }
    
private:
    static MemPool& createSingleton()                   { MemPool& pool = SmallAllocator_createSingleton(); pool.setId("Small"); return pool; }
};

/// Inherit from this class to use the small block allocator
typedef AllocatorObject<SmallAllocator> SmallAllocatorObject;

#ifndef SmallAllocator_createSingleton_
    /// Default implementation
    inline MemPool& SmallAllocator_createSingleton()
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
