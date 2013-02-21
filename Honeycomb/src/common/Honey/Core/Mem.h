// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

/// Global `new_`, use in place of `new` keyword to provide allocator with debug info
#ifdef DEBUG
    #define new_                                new (__FILE__, __LINE__)
#else
    #define new_                                new
#endif

inline void* operator new(size_t size, const char* /*srcFile*/, int /*srcLine*/)        { return operator new(size); }
inline void* operator new[](size_t size, const char* /*srcFile*/, int /*srcLine*/)      { return operator new(size); }

inline void operator delete(void* p, const char* /*srcFile*/, int /*srcLine*/)          { operator delete(p); }
inline void operator delete[](void* p, const char* /*srcFile*/, int /*srcLine*/)        { operator delete(p); }

namespace honey
{

/// Allocate memory for `count` number of T objects.  Objects are not constructed.
template<class T>
T* alloc(size_t count = 1)                      { return static_cast<T*>(operator new(sizeof(T)*count)); }
/// Deallocate memory and set pointer to null. Object is not destroyed.
template<class T>
void free(T*& p)                                { if (!p) return; operator delete(p); p = nullptr; }
template<class T>
void free(T* const& p)                          { if (!p) return; operator delete(p); }

/// Align a pointer to the previous byte boundary `bytes`. Does nothing if p is already on boundary.  Alignment must be a power of two.
template<class T>
T* alignFloor(T* p, int bytes)                  { return reinterpret_cast<T*>(intptr_t(p) & ~(bytes-1)); }
/// Align a pointer to the next byte boundary `bytes`. Does nothing if p is already on boundary.  Alignment must be a power of two.
template<class T>
T* alignCeil(T* p, int bytes)                   { return alignFloor(reinterpret_cast<T*>(intptr_t(p) + bytes-1), bytes); }

/// Allocate memory with alignment.  Alignment must be a power of two.  Allocator element type must be int8.
template<class T, class Alloc>
T* allocAligned(size_t count, int align_, Alloc&& alloc)
{
    static const int diffSize = sizeof(std::ptrdiff_t);
    int8* base = alloc.allocate(diffSize + align_-1 + sizeof(T)*count);
    if (!base) return nullptr;
    int8* p = alignCeil(base+diffSize, align_);
    *reinterpret_cast<std::ptrdiff_t*>(p-diffSize) = p - base;
    return reinterpret_cast<T*>(p);
}
/// Allocate memory with alignment using default allocator
template<class T>
T* allocAligned(size_t count, int align)        { return allocAligned<T>(count, align, std::allocator<int8>()); }

/// Deallocate aligned memory.  Allocator element type must be int8.
template<class T, class Alloc>
void freeAligned(T* p, Alloc&& alloc)
{
    if (!p) return;
    int8* p_ = reinterpret_cast<int8*>(p);
    int8* base = p_ - *reinterpret_cast<std::ptrdiff_t*>(p_ - sizeof(std::ptrdiff_t));
    alloc.deallocate(base, 1);
}
/// Deallocate aligned memory using default allocator
template<class T>
void freeAligned(T* p)                          { freeAligned(p, std::allocator<int8>()); }


/// Destruct object, free memory and set pointer to null
template<class T>
void delete_(T*& p)                             { delete p; p = nullptr; }
template<class T>
void delete_(T* const& p)                       { delete p; }

/// Destruct object, free memory using allocator and set pointer to null
template<class T, class Alloc>
void delete_(T*& p, Alloc&& a)                  { if (!p) return; a.destroy(p); a.deallocate(p,1); p = nullptr; }
template<class T, class Alloc>
void delete_(T* const& p, Alloc&& a)            { if (!p) return; a.destroy(p); a.deallocate(p,1); }

/// Destruct all array objects, free memory and set pointer to null
template<class T>
void deleteArray(T*& p)                         { delete[] p; p = nullptr; }
template<class T>
void deleteArray(T* const& p)                   { delete[] p; }

/// Functor to delete a pointer
template<class T, class Alloc = void> struct finalize
{
    finalize(const Alloc& a = Alloc())          : a(a) {}
    void operator()(T*& p)                      { delete_(p,a); }
    void operator()(T* const& p)                { delete_(p,a); }
    Alloc a;
};
/** \cond */
/// Fast specialization for default allocator
template<class T> struct finalize<T,void>
{
    void operator()(T*& p)                      { delete_(p); }
    void operator()(T* const& p)                { delete_(p); }
};
/// Specialization for array
template<class T> struct finalize<T[],void>
{
    void operator()(T*& p)                      { deleteArray(p); }
    void operator()(T* const& p)                { deleteArray(p); }
};
/// Specialization for void
template<> struct finalize<void,void>
{
    void operator()(void*& p)                   { free(p); }
    void operator()(void* const& p)             { free(p); }
};
/** \endcond */

/// std::allocator compatible allocator.  Subclass must define std::allocator constructors and alloc()/free().
template<template<class> class Subclass, class T>
class Allocator
{
public:
    /// Types required by std::allocator
    typedef T           value_type;
    typedef T*          pointer;
    typedef T&          reference;
    typedef const T*    const_pointer;
    typedef const T&    const_reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;

    /// Methods required by std::allocator
    pointer address(reference x) const                      { return &x; }
    const_pointer address(const_reference x) const          { return &x; }
    pointer allocate(size_type n, const void* hint=0)       { (void)hint; return subc().alloc(n); }
    void deallocate(pointer p, size_type /*n*/)             { subc().free(p); }
    size_type max_size() const                              { return std::numeric_limits<size_type>::max(); }
    void construct(pointer p, const_reference val)          { new (p) T(val); }
    void destroy(pointer p)                                 { (void)p; p->~T(); }
    Subclass<T> select_on_container_copy_construction() const   { return subc(); }
    template<class U> struct rebind                         { typedef Subclass<U> other; };
    bool operator==(const Subclass<T>&) const               { return true; }
    bool operator!=(const Subclass<T>&) const               { return false; }

protected:
    Subclass<T>& subc()                                     { return static_cast<Subclass<T>&>(*this); }
    const Subclass<T>& subc() const                         { return static_cast<const Subclass<T>&>(*this); }
};

/// Objects may inherit from this class to use a custom allocator for new/delete ops
template<template<class> class Alloc>
class AllocatorObject
{
public:
    template<class T> struct Allocator { typedef Alloc<T> type; };

    void* operator new(size_t size)                                         { return _alloc.alloc(size); }
    void* operator new(size_t size, const char* srcFile, int srcLine)       { return _alloc.alloc(size, srcFile, srcLine); }
    
    void* operator new[](size_t size)                                       { return _alloc.alloc(size); }
    void* operator new[](size_t size, const char* srcFile, int srcLine)     { return _alloc.alloc(size, srcFile, srcLine); }
    
    void operator delete(void* p)                                           { _alloc.free(static_cast<int8*>(p)); }
    void operator delete(void* p, const char* srcFile, int srcLine)         { _alloc.free(static_cast<int8*>(p)); }
    
    void operator delete[](void* p)                                         { _alloc.free(static_cast<int8*>(p)); }
    void operator delete[](void* p, const char* srcFile, int srcLine)       { _alloc.Free(static_cast<int8*>(p)); }

private:
    static Alloc<int8> _alloc;
};
template<template<class> class Alloc> Alloc<int8> AllocatorObject<Alloc>::_alloc;

/// Inherit to use global new and delete
struct GlobalAllocatorObject
{
    template<class T> struct Allocator { typedef std::allocator<T> type; };
};

}
