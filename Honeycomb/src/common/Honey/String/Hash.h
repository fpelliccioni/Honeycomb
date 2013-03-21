// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Core.h"

namespace honey
{

class String;
class StringStream;

/// Data hashing functions.  Produce a small fingerprint from a larger data set.  Two data sets may 'collide', producing the same fingerprint.
namespace hash
{

/** \cond */
/// Constexpr version of MurmurHash3_x86_32
namespace priv { namespace murmur_constexpr
{
    constexpr uint32 rotLeft(uint32 v, int32 n)      { return (v << n) | (v >> (32-n)); }
    constexpr uint32 fMix_1(uint32 h)                { return h^(h >> 16); }
    constexpr uint32 fMix_0(uint32 h)                { return fMix_1((h^(h >> 13))*0xc2b2ae35); }
    constexpr uint32 fMix(uint32 h)                  { return fMix_0((h^(h >> 16))*0x85ebca6b); }
    constexpr uint32 c1                              = 0xcc9e2d51;
    constexpr uint32 c2                              = 0x1b873593;
    
    template<int Endian>
    constexpr uint32 block(const char* data, int i)                 { return data[i*4] | data[i*4+1] << 8 | data[i*4+2] << 16 | data[i*4+3] << 24; }
    template<>
    constexpr uint32 block<ENDIAN_BIG>(const char* data, int i)     { return data[i*4] << 24 | data[i*4+1] << 16 | data[i*4+2] << 8 | data[i*4+3]; }
    
    constexpr uint32 tail(const char* data, int len, int nblocks, uint32 h1)
    {
        return rotLeft((
                ((len&3) >= 3 ? data[nblocks*4+2] << 16 : 0) |
                ((len&3) >= 2 ? data[nblocks*4+1] << 8 : 0) |
                ((len&3) >= 1 ? data[nblocks*4] : 0))
                *c1, 15)*c2 ^ h1;
    }

    constexpr uint32 loop(const char* data, int len, int nblocks, int i, uint32 h1)
    {
        return i < nblocks ?    loop(data, len, nblocks, i+1, rotLeft(rotLeft(block<ENDIAN>(data, i)*c1, 15)*c2 ^ h1, 13)*5 + 0xe6546b64) :
                                fMix(tail(data, len, nblocks, h1) ^ len);
    }
} }
/** \endcond */

/// Quickly generate a small hash value. Each seed value produces a unique hash from the same data.
int fast(const uint8* data, int len, int seed = 0);
/// fast() for strings.  The string is converted to UTF-8 to optimize data density.
int fast(const String& str, int seed = 0);
/// Hash a UTF-8 string at compile-time
constexpr int fast_constexpr(const char* str, int len, int seed = 0)    { return priv::murmur_constexpr::loop(str, len, len / 4, 0, seed); }

/// Convert hash value to string, produces hex code independent of platform endian
String toString(int hash);

/// 512 bit secure hash data
struct SecureData
{
    static const int size = 64;
    uint8 a[size];

    /// To string, produces hex code
    friend StringStream& operator<<(StringStream& os, const SecureData& val);
};

/// Generate a large secure hash value
SecureData secure(const uint8* data, int len, int seed = 0);
/// secure() for strings.  The string is converted to UTF-8 to optimize data density.
SecureData secure(const String& str, int seed = 0);

} }
