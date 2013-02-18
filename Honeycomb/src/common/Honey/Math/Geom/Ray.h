// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Shape.h"
#include "Honey/Math/Alge/Transform.h"

namespace honey
{

template<class Real> class Line_;

/// 3D ray geometry class
template<class Real>
class Ray_ : public Shape<Real>
{
    typedef Vec<3,Real> Vec3;
    typedef Line_<Real> Line;

public:
    /// No initialization
    Ray_()                                                                      {}

    /// Construct from origin and unit direction
    Ray_(const Vec3& origin, const Vec3& dir) : origin(origin), dir(dir)        {}

    /// Construct from line start point and unit direction between line end points.
    Ray_(const Line& line);

    ~Ray_()                                                                     {}

    /// Get point on ray given ray parameter (distance along ray)
    Vec3 point(Real t) const                                                    { return origin + dir*t; }

    /// Get ray parameter (distance along ray) given any point
    Real param(const Vec3& point) const                                         { return dir.dot(point-origin); }

    /// Transform operator
    friend Ray_ operator*(const Matrix<4,4,Real>& mat, const Ray_& ray)         { return Ray_(mat*ray.origin, mat.mulRotScale(ray.dir).normalize()); }
    friend Ray_ operator*(const Transform_<Real>& tm, const Ray_& ray)          { return Ray_(tm*ray.origin, tm.hasScale() ? tm.mulRotScale(ray.dir).normalize() : tm.mulRotScale(ray.dir)); }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Ray_& val)
    {
        return os << "{ origin" << val.origin << ", dir" << val.dir << " }";
    }

public:
    Vec3 origin;
    Vec3 dir;    ///< Direction
};


typedef Ray_<Real>     Ray;
typedef Ray_<Float>    Ray_f;
typedef Ray_<Double>   Ray_d;


}