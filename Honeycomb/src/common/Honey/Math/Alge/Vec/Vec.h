// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Vec/Base.h"
#include "Honey/Math/Alge/Vec/priv/Storage.h"
#include "Honey/Math/Alge/Matrix/Block.h"

namespace honey
{

namespace vec { namespace priv
{
    /// N-dimensional vector traits
    template<int Dim, class Real, int Options, class Alloc>
    struct Traits
    {
        typedef priv::Storage<Vec<Dim,Real,Options,Alloc>> Storage;
        typedef Real                Real;
        typedef Real                ElemT;
        static const int dim        = Dim;
        static const int rows       = Options & matrix::Option::vecRow ? 1 : dim;
        static const int cols       = Options & matrix::Option::vecRow ? dim : 1;
        static const int options    = Options;
        typedef Alloc               Alloc;
    };
} }

template<int Dim, class Real, int Options, class Alloc>
struct matrix::priv::Traits<Vec<Dim,Real,Options,Alloc>> : vec::priv::Traits<Dim,Real,Options,Alloc> {};

/// N-dimensional vector
template<int Dim, class Real, int Options, class Alloc>
class Vec : public VecBase<Vec<Dim,Real,Options,Alloc>>
{
public:
    /// No init
    Vec()                                                           {}
    /// Allocate elements for dimension size, only available if vector is dynamic.
    template<class Int>
    explicit Vec(Int dim, typename std::enable_if<std::is_integral<Int>::value && s_size == matrix::dynamic>::type*_=0)
                                                                    { mt_unused(_); resize(dim); }
    /// Construct uniform vector
    explicit Vec(Real scalar)                                       { fromScalar(scalar); }
    /// Initialize from array with dimension `dim`
    Vec(const Real* a, int dim)                                     { resize(dim); fromArray(a); }
    /// Construct with allocator, for a dynamic vector. Allocator element type must be int8.
    Vec(const Alloc& alloc)                                         { setAllocator(alloc); }
    /// Construct from row or column vector of any dimension. Asserts that if this vector has a fixed dimension then it matches rhs.
    template<class T>
    Vec(const MatrixBase<T>& rhs)                                   { operator=(rhs); }

    /// Assign to row or column vector of any dimension. Asserts that if this vector has a fixed dimension then it matches rhs.
    template<class T>
    Vec& operator=(const MatrixBase<T>& rhs)                        { VecBase::operator=(rhs); return *this; }
};

/// N-dimensional column vector types
typedef Vec<matrix::dynamic>           VecN;
typedef Vec<matrix::dynamic, Float>    VecN_f;
typedef Vec<matrix::dynamic, Double>   VecN_d;

/// N-dimensional row vector types
typedef Vec<matrix::dynamic, Real, matrix::Option::vecRow>      VecRowN;
typedef Vec<matrix::dynamic, Float, matrix::Option::vecRow>     VecRowN_f;
typedef Vec<matrix::dynamic, Double, matrix::Option::vecRow>    VecRowN_d;


/// It's not possible to inherit ctors, so this macro is required
#define MATRIX_VEC_ADAPTER                                                                                              \
public:                                                                                                                 \
    Matrix()                                                        {}                                                  \
    template<class Int>                                                                                                 \
    explicit Matrix(Int dim, typename std::enable_if<std::is_integral<Int>::value && s_size == matrix::dynamic>::type*_=0)    \
                                                                    : Vec(dim) {}                                       \
    explicit Matrix(Real scalar)                                    : Vec(scalar) {}                                    \
    Matrix(const Real* a, int dim)                                  : Vec(a,dim) {}                                     \
    Matrix(const Alloc& alloc)                                      : Vec(alloc) {}                                     \
    template<class T>                                                                                                   \
    Matrix(const MatrixBase<T>& rhs)                                : Vec(rhs) {}                                       \
    template<class T>                                                                                                   \
    Matrix& operator=(const MatrixBase<T>& rhs)                     { Vec::operator=(rhs); return *this; }              \

/// Matrix column vector 
template<int Dim, class Real, int Options, class Alloc>
class Matrix<Dim,1,Real,Options,Alloc> : public Vec<Dim,Real,Options,Alloc>
{ MATRIX_VEC_ADAPTER };

/// Matrix row vector
template<int Dim, class Real, int Options, class Alloc>
class Matrix<1,Dim,Real,Options,Alloc> : public Vec<Dim,Real, Options | matrix::Option::vecRow, Alloc>
{ MATRIX_VEC_ADAPTER };

}
