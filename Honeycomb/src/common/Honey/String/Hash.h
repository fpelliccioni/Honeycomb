// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Core.h"

namespace honey
{

class String;
class StringStream;

/// Data hashing functions.  Produce a small unique fingerprint from a larger data set.
/**
  * Collisions (two data sets producing the same fingerprint) are possible but rare.
  */
class Hash
{
public:
    /// Quickly generate a small hash value. Each seed value produces a unique hash from the same data.
    static int fast(const uint8* data, int len, int seed = 0);
    static int fast(const String& str, int seed = 0);

    /// Convert hash value to string, produces hex code independent of platform endian
    static String toString(int hash);

    /// 512 bit secure hash data
    struct SecureData
    {
        static const int size = 64;
        uint8 a[size];

        /// To string, produces hex code
        friend StringStream& operator<<(StringStream& os, const SecureData& val);
    };

    /// Generate a large secure hash value
    static SecureData secure(const uint8* data, int len, int seed = 0);
    static SecureData secure(const String& str, int seed = 0);
};

}
