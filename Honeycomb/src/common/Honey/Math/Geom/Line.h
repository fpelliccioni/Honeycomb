// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Shape.h"
#include "Honey/Math/Alge/Transform.h"

namespace honey
{

template<class Real> class Ray_;

/// 3D line segment geometry class
template<class Real>
class Line_ : public Shape<Real>
{
    typedef Alge_<Real>     Alge;
    typedef Vec<3,Real>     Vec3;
    typedef Ray_<Real>      Ray;

public:

    /// No initialization
    Line_()                                                                         {}

    /// Construct from end points
    Line_(const Vec3& v0, const Vec3& v1)                                           : v0(v0), v1(v1)    {}

    /// Construct from start point, direction and length to end point
    Line_(const Vec3& v, const Vec3& dir, Real length)                              : v0(v), v1(v+dir*length)   {}

    /// Construct from ray
    Line_(const Ray& ray, Real length = 1);

    ~Line_()                                                                        {}

    /// Construct from center, axis and extent along each axis direction
    Line_& fromCenter(const Vec3& center, const Vec3& axis, Real extent)
    {
        v0 = center - axis*extent;
        v1 = center + axis*extent;
        return *this;
    }

    /// Number of points (constant)
    static const int pointCount = 2;

    /// Access points
    Vec3& operator[](int i)                                                         { return point(i); }
    const Vec3& operator[](int i) const                                             { return point(i); }

    /// Get point
    const Vec3& point(int index) const                                              { return *(&v0+index); }
    Vec3& point(int index)                                                          { return *(&v0+index); }

    /// Get difference between end points
    Vec3 diff() const                                                               { return v1-v0; }

    /// Get unit vector direction.  Also returns length if desired.
    Vec3 dir(option<Real&> len = optnull) const                                     { return (v1-v0).normalize(len); }

    /// Get center point
    Vec3 center() const                                                             { return (v0+v1)*0.5; }

    /// Extent from center of line in each direction (half length)
    Real extent() const                                                             { return length()*0.5; }

    /// Get line with end points swapped, so line runs in opposite direction
    Line_ reverse() const                                                           { return Line_(v1,v0); }

    /// Square of length
    Real lengthSqr() const                                                          { return (v1-v0).lengthSqr(); }

    /// Distance between end points
    Real length() const                                                             { return (v1-v0).length(); }

    /// Get point on line given line parameter (distance along line)
    Vec3 point(Real t) const                                                        { return v0 + dir()*t; }

    /// Get line parameter (distance along line) given any point
    Real param(const Vec3& point) const                                             { return dir().dot(point-v0); }

    /// Transform operator
    friend Line_ operator*(const Matrix<4,4,Real>& mat, const Line_& line)          { return Line_(mat*line.v0, mat*line.v1); }
    friend Line_ operator*(const Transform_<Real>& tm, const Line_& line)           { return Line_(tm*line.v0, tm*line.v1); }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Line_& val)
    {
        return os << "{ v0" << val.v0 << ", v1" << val.v1 << " }";
    }

public:
    Vec3 v0;    ///< Start point
    Vec3 v1;    ///< End point
};


typedef Line_<Real>    Line;
typedef Line_<Float>   Line_f;
typedef Line_<Double>  Line_d;


}
