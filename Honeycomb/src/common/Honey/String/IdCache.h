// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"
#include "Honey/Thread/Thread.h"

namespace honey
{

/// Create id from string literal using cache
/**
  * \ingroup Id
  * Every translation unit (cpp file) that calls ID will have its own per-thread cache.
  *
  * Example:
  *     
  *     //Update is called repeatedly, it would be slow to hash the string every call.
  *     //We can either create an id statically or fetch the id from a local cache.
  *     void update()
  *     {
  *         static Id s_id = "my long string";  //Create statically
  *         Id id = ID"my long string";         //Fetch from cache
  *     }
  */
#define ID priv::IdCache::inst() << 

/** \cond */
namespace priv
{
    /// Caches Ids for string literals
    class IdCache
    {
    public:
        // Get per-thread singleton
        static IdCache& inst();

        /// Create id from string literal
        const Id& operator<<(const char* name)      { return cacheId(name); }
        const Id& operator<<(const Char* name)      { return cacheId(name); }

    private:
        typedef unordered_map<const void*, Id> IdMap;
    
        IdCache() {}

        template<class T>
        const Id& cacheId(T* name)
        {
            IdMap::iterator it = _idMap.find(name);
            if (it != _idMap.end()) return it->second;
            return _idMap[name] = name;
        }

        IdMap _idMap;
    };

    static UniquePtr<thread::Local<IdCache>> g_idCache = nullptr;

    inline IdCache& IdCache::inst()                 { return *(g_idCache ? g_idCache : g_idCache = new thread::Local<IdCache>([] { return new IdCache; }));  }
}
/** \endcond */

}
