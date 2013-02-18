// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Vec/Vec.h"

namespace honey
{

/// Vector for const swizzle operators
template<int Dim, class Real, int Options> class VecSwizCon;

template<int Dim, class Real, int Options>
struct matrix::priv::Traits<VecSwizCon<Dim,Real,Options>> : Traits<Vec<Dim,Real,Options>> {};

template<class SwizT>
class VecSwizConBase : public Vec<  matrix::priv::Traits<SwizT>::dim, typename matrix::priv::Traits<SwizT>::Real,
                                    matrix::priv::Traits<SwizT>::options>
{
private:
    SwizT& fromZero();
    SwizT& fromScalar(Real f);
    SwizT& operator=(const VecSwizConBase& rhs);
    template<class T>
    SwizT& operator+=(const MatrixBase<T>& rhs);
    template<class T>
    SwizT& operator-=(const MatrixBase<T>& rhs);
    SwizT& operator*=(Real rhs);
    SwizT& operator/=(Real rhs);
    SwizT& elemAddEq(Real rhs);
    SwizT& elemSubEq(Real rhs);
    template<class T>
    SwizT& elemMulEq(const MatrixBase<T>& rhs);
    template<class T>
    SwizT& elemDivEq(const MatrixBase<T>& rhs);
};

/** \cond */
template<class T, int D, class R, int O>
struct priv::map_impl0<T, VecSwizCon<D,R,O>>
{
    template<class T, class O, class Func>
    static O&& func(T&&, O&&, Func&&)                               { static_assert(false, "Can't map with const swizzle output"); }
};

template<class T, class T2, int D, class R, int O>
struct priv::map_impl1<T, T2, VecSwizCon<D,R,O>>
{
    template<class T, class T2, class O, class Func>
    static O&& func(T&&, T2&&, O&&, Func&&)                         { static_assert(false, "Can't map with const swizzle output"); }
};
/** \endcond */

/// Vector reference holder for mutable swizzle operators
template<int Dim, class Real, int Options> class VecSwizRef;

template<int Dim, class Real, int Options>
struct matrix::priv::Traits<VecSwizRef<Dim,Real,Options>> : Traits<Vec<Dim,Real,Options>> {};

template<class SwizT>
class VecSwizRefBase : public Vec<  matrix::priv::Traits<SwizT>::dim, typename matrix::priv::Traits<SwizT>::Real,
                                    matrix::priv::Traits<SwizT>::options>
{
public:
    typedef Vec Super;

    /// Allow scalar ops
    SwizT& operator=(Real rhs)                                      { return fromScalar(rhs); }
    SwizT& operator+=(Real rhs)                                     { return elemAddEq(rhs); }
    SwizT& operator-=(Real rhs)                                     { return elemSubEq(rhs); }

    /// Wrapper ops
    SwizT& fromZero()                                               { Vec::fromZero(); return swiz().commit(); }
    SwizT& fromScalar(Real f)                                       { Vec::fromScalar(f); return swiz().commit(); }
    SwizT& operator=(const VecSwizRefBase& rhs)                     { Vec::operator=(rhs); return swiz().commit(); }
    template<class T>
    SwizT& operator=(const MatrixBase<T>& rhs)                      { Vec::operator=(rhs.subc()); return swiz().commit(); }
    template<class T>
    SwizT& operator+=(const MatrixBase<T>& rhs)                     { Vec::operator+=(rhs.subc()); return swiz().commit(); }
    template<class T>
    SwizT& operator-=(const MatrixBase<T>& rhs)                     { Vec::operator-=(rhs.subc()); return swiz().commit(); }
    SwizT& operator*=(Real rhs)                                     { Vec::operator*=(rhs); return swiz().commit(); }
    SwizT& operator/=(Real rhs)                                     { Vec::operator/=(rhs); return swiz().commit(); }
    SwizT& elemAddEq(Real rhs)                                      { Vec::elemAddEq(rhs); return swiz().commit(); }
    SwizT& elemSubEq(Real rhs)                                      { Vec::elemSubEq(rhs); return swiz().commit(); }
    template<class T>
    SwizT& elemMulEq(const MatrixBase<T>& rhs)                      { Vec::elemMulEq(rhs.subc()); return swiz().commit(); }
    template<class T>
    SwizT& elemDivEq(const MatrixBase<T>& rhs)                      { Vec::elemDivEq(rhs.subc()); return swiz().commit(); }

    /// Get subclass
    const SwizT& swiz() const                                       { return reinterpret_cast<const SwizT&>(*this); }
    SwizT& swiz()                                                   { return reinterpret_cast<SwizT&>(*this); }
};

/** \cond */
template<class T, int D, class R, int O>
struct priv::map_impl0<T, VecSwizRef<D,R,O>>
{
    template<class T, class O, class Func>
    static O&& func(T&& v, O&& o, Func&& f)                         { map(forward<T>(v), forward<typename O::Super>(o), forward<Func>(f)); o.swiz().commit(); return forward<O>(o); }
};

template<class T, class T2, int D, class R, int O>
struct priv::map_impl1<T, T2, VecSwizRef<D,R,O>>
{
    template<class T, class T2, class O, class Func>
    static O&& func(T&& v, T2&& v2, O&& o, Func&& f)                { map(forward<T>(v), forward<T2>(v2), forward<typename O::Super>(o), forward<Func>(f)); o.swiz().commit(); return forward<O>(o); }
};
/** \endcond */

}
