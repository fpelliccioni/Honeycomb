// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/String.h"

namespace honey
{
/** \cond */
namespace numeral_priv { template<class T> struct Info; }
/** \endcond */

class Float_;
class Double_;
class Quad_;

/// Numeric type information, use numeral() to get instance safely from a static context
template<class T> class Numeral;

/// Numeric type info for integer types
template<class T>
class NumeralInt
{
protected:
    typedef numeral_priv::Info<T> Info;
    Info _info;

public:
    /// Signed version of type
    typedef typename Info::Signed           Signed;
    /// Unsigned version of type
    typedef typename Info::Unsigned         Unsigned;
    /// Integer representation of type
    typedef typename Info::Int              Int;
    /// Real representation of type
    typedef typename Info::Real             Real;
    /// Real operations and constants class
    typedef typename Info::Real_            Real_;

    /// Whether type is signed
    static const bool isSigned              = Info::isSigned;
    /// Whether type is integral
    static const bool isInteger             = Info::isInteger;

    /// Size of type in bits
    static const int sizeBits               = sizeof(T)*8;

    /// Minimum possible value for type (negative for signed types)
    T min() const                           { return _info.min; }
    /// Maximum possible value for type
    T max() const                           { return _info.max; }
};

/// Numeric type info for floating point types
template<class T>
class NumeralFloat : public NumeralInt<T>
{
public:
    /// Smallest representable value (close to zero)
    T smallest() const                      { return this->_info.smallest; }
    /// Smallest value such that 1.0 + epsilon != 1.0
    T epsilon() const                       { return this->_info.epsilon; }
    /// Infinity. ie. 1.0 / 0.0
    T inf() const                           { return this->_info.inf; }
    /// Not a number. ie. 0.0 / 0.0, sqrt(-1)
    T nan() const                           { return this->_info.nan; }
};


/// Get numeric type info safely from a static context
template<class T>
const Numeral<T>& numeral()                 { static Numeral<T> _obj; return _obj; }

/// Get numeric type info using type deduction
template<class T>
const Numeral<T>& numeral(const T&)         { return numeral<T>(); }


/// Safe conversion from an unsigned type to a signed type
template<class Unsigned>
typename Numeral<Unsigned>::Signed utos(Unsigned u)
{
    assert(static_cast<typename Numeral<Unsigned>::Signed>(u) >= 0);
    return static_cast<typename Numeral<Unsigned>::Signed>(u);
};

/// Safe conversion from a signed type to an unsigned type
template<class Signed>
typename Numeral<Signed>::Unsigned stou(Signed s)
{
    assert(s >= 0);
    return static_cast<typename Numeral<Signed>::Unsigned>(s);
};

/** \cond */
namespace numeral_priv
{
    template<> struct Info<int8>
    {
        typedef int8    Signed;
        typedef uint8   Unsigned;
        typedef int8    Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = true;
        static const bool isInteger = true;
        static const int8 min       = -0x7F - 1; 
        static const int8 max       = 0x7F;
    };
    
    template<> struct Info<uint8> 
    {
        typedef int8    Signed;
        typedef uint8   Unsigned;
        typedef uint8   Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = false;
        static const bool isInteger = true;
        static const uint8 min      = 0;
        static const uint8 max      = 0xFFU;
    };

    template<> struct Info<int16>
    {
        typedef int16   Signed;
        typedef uint16  Unsigned;
        typedef int16   Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = true;
        static const bool isInteger = true;
        static const int16 min      = -0x7FFF - 1;
        static const int16 max      = 0x7FFF;
    };

    template<> struct Info<uint16>
    {
        typedef int16   Signed;
        typedef uint16  Unsigned;
        typedef uint16  Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = false;
        static const bool isInteger = true;
        static const uint16 min     = 0;
        static const uint16 max     = 0xFFFFU;
    };
    
    template<> struct Info<int32>
    {
        typedef int32   Signed;
        typedef uint32  Unsigned;
        typedef int32   Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = true;
        static const bool isInteger = true;
        static const int32 min      = -0x7FFFFFFF - 1;
        static const int32 max      = 0x7FFFFFFF;
    };

    template<> struct Info<uint32>
    {
        typedef int32   Signed;
        typedef uint32  Unsigned;
        typedef uint32  Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = false;
        static const bool isInteger = true;
        static const uint32 min     = 0;
        static const uint32 max     = 0xFFFFFFFFU;
    };

    template<> struct Info<int64>
    {
        typedef int64   Signed;
        typedef uint64  Unsigned;
        typedef int64   Int;
        typedef double  Real;
        typedef Double_ Real_;
        static const bool isSigned  = true;
        static const bool isInteger = true;
        static const int64 min      = -0x7FFFFFFFFFFFFFFFL - 1;
        static const int64 max      = 0x7FFFFFFFFFFFFFFFL;
    };

    template<> struct Info<uint64>
    {
        typedef int64   Signed;
        typedef uint64  Unsigned;
        typedef uint64  Int;
        typedef double  Real;
        typedef Double_ Real_;
        static const bool isSigned  = false;
        static const bool isInteger = true;
        static const uint64 min     = 0;
        static const uint64 max     = 0xFFFFFFFFFFFFFFFFUL;
    };

    template<> struct Info<float>
    {
        typedef float   Signed;
        typedef float   Unsigned;
        typedef int32   Int;
        typedef float   Real;
        typedef Float_  Real_;
        static const bool isSigned  = true;
        static const bool isInteger = false;

        // Disable divide by zero warning
        #pragma warning(push)
        #pragma warning(disable:4723)

        Info() :
            min(        -3.402823466e+38f),
            max(        3.402823466e+38f),
            smallest(   1.175494351e-38f),
            epsilon(    1.192092896e-07f),
            one(        1.f),
            inf(        1.f / (1.f - one)),
            nan(        0.f / (1.f - one)) {}

        #pragma warning(pop)

        const float min;
        const float max;
        const float smallest;
        const float epsilon;
        const float one;
        const float inf;
        const float nan;
    };

    template<> struct Info<double>
    {
        typedef double  Signed;
        typedef double  Unsigned;
        typedef int64   Int;
        typedef double  Real;
        typedef Double_ Real_;
        static const bool isSigned  = true;
        static const bool isInteger = false;

        // Disable divide by zero warning
        #pragma warning(push)
        #pragma warning(disable:4723)

        Info() :
            min(        -1.7976931348623158e+308),
            max(        1.7976931348623158e+308),
            smallest(   2.2250738585072014e-308),
            epsilon(    2.2204460492503131e-016),
            one(        1.0),
            inf(        1.0 / (1.0 - one)),
            nan(        0.0 / (1.0 - one)) {}

        #pragma warning(pop)

        const double min;
        const double max;
        const double smallest;
        const double epsilon;
        const double one;
        const double inf;
        const double nan;
    };
}

template<> class Numeral<int8> : public NumeralInt<int8> {};
template<> class Numeral<uint8> : public NumeralInt<uint8> {};
template<> class Numeral<int16> : public NumeralInt<int16> {};
template<> class Numeral<uint16> : public NumeralInt<uint16> {};
template<> class Numeral<int32> : public NumeralInt<int32> {};
template<> class Numeral<uint32> : public NumeralInt<uint32> {};
template<> class Numeral<int64> : public NumeralInt<int64> {};
template<> class Numeral<uint64> : public NumeralInt<uint64> {};
template<> class Numeral<float> : public NumeralFloat<float> {};
template<> class Numeral<double> : public NumeralFloat<double> {};
/** \endcond */

}

#include "Honey/Math/platform/Numeral.h"

