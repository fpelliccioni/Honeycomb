// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Matrix/Base.h"

namespace honey
{

/// (m x n)-dimensional matrix traits
template<int Rows, int Cols, class Real, int Options, class Alloc>
struct matrix::priv::Traits<Matrix<Rows,Cols,Real,Options,Alloc>>
{
    typedef Storage<Matrix<Rows,Cols,Real,Options,Alloc>> Storage;
    typedef Real                Real;
    typedef Real                ElemT;
    static const int rows       = Rows;
    static const int cols       = Cols;
    static const int options    = Options;
    typedef Alloc               Alloc;
};

/// (m x n)-dimensional matrix
template<int Rows, int Cols, class Real, int Options, class Alloc>
class Matrix : public MatrixBase<Matrix<Rows,Cols,Real,Options,Alloc>>
{
public:
    /// No init
    Matrix()                                                            {}
    /// Allocate elements for dimension sizes. Asserts that any fixed dimensions match rows / cols.
    Matrix(int rows, int cols)                                          { resize(rows, cols); }
    /// Initialize with scalar in every element
    explicit Matrix(Real scalar)                                        { fromScalar(scalar); }
    /// Initialize from array with dimensions (rows x cols). If the array is in row-major format set rowMajor to true, otherwise set to false for column-major.
    Matrix(const Real* a, int rows, int cols, bool rowMajor = true)     { resize(rows, cols); fromArray(a, rowMajor); }
    /// Construct with allocator, for a dynamic matrix. Allocator element type must be int8.
    Matrix(const Alloc& alloc)                                          { setAllocator(alloc); }
    /// Construct from matrix of any size. Asserts that any fixed dimensions in this matrix match those in rhs.
    template<class T>
    Matrix(const MatrixBase<T>& rhs)                                    { operator=(rhs); }

    /// Assign to matrix of any size. Asserts that any fixed dimensions in this matrix match those in rhs.
    template<class T>
    Matrix& operator=(const MatrixBase<T>& rhs)                         { MatrixBase::operator=(rhs.subc()); return *this; }
};

/// (m x n)-dimensional matrix types
typedef Matrix<matrix::dynamic, matrix::dynamic>            MatrixN;
typedef Matrix<matrix::dynamic, matrix::dynamic, Float>     MatrixN_f;
typedef Matrix<matrix::dynamic, matrix::dynamic, Double>    MatrixN_d;

}
