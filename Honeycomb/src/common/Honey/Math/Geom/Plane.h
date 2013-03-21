// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Geom/Shape.h"
#include "Honey/Math/Alge/Transform.h"

namespace honey
{

template<class Real> class Triangle_;

/// 3D plane geometry class
/**    
  * Plane equation is: \f$ \vec{n} \cdot \vec{x} = d \f$ \n
  * This class represents a plane by the plane normal \f$\vec{n}\f$ and the distance from origin along that normal _d_.
  *
  * An equivalent plane equation is: \f$ Ax + By + Cz - D = 0 \f$ \n
  * This class represents a plane by the plane normal (_A_,_B_,_C_) and the constant _D_.
  */
template<class Real>
class Plane_ : public Shape<Real>
{
    typedef Vec<3,Real>     Vec3;
    typedef Triangle_<Real> Triangle;

public:
    enum class Side
    {
        neg = -1,   ///< negative side of plane
        pos = 1     ///< positive side or on plane
    };

    /// No initialization
    Plane_()                                                                {}

    /// Construct from normal and distance along normal
    Plane_(const Vec3& normal, Real dist)                                   : normal(normal), dist(dist)  {}

    /// Construct from normal and point
    Plane_(const Vec3& normal, const Vec3& point)                           : normal(normal), dist(normal.dot(point)) {}

    /// Construct from 3 points
    Plane_(const Vec3& p0, const Vec3& p1, const Vec3& p2)                  { fromPoints(p0, p1, p2); }

    /// Construct from triangle
    Plane_(const Triangle& tri);

    ~Plane_()                                                               {}

    /// Transform operator
    friend Plane_ operator*(const Matrix<4,4,Real>& mat, const Plane_& plane)
    {
        Plane_ ret;
        ret.normal = mat.mulRotScale(plane.normal).normalize();
        ret.dist = (mat*(plane.normal*plane.dist)).dot(ret.normal);
        return ret;
    }

    friend Plane_ operator*(const Transform_<Real>& tm, const Plane_& plane)
    {
        Plane_ ret;
        ret.normal = tm.hasScale() ? tm.mulRotScale(plane.normal).normalize() : tm.mulRotScale(plane.normal);
        ret.dist = (tm*(plane.normal*plane.dist)).dot(ret.normal);
        return ret;
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Plane_& val)
    {
        return os << "{ normal" << val.normal << ", dist(" << val.dist << ") }";
    }

protected:

    void fromPoints(const Vec3& p0, const Vec3& p1, const Vec3& p2)
    {
        normal = (p1-p0).crossUnit(p2-p0);
        dist = normal.dot(p0);
    }

public:
    Vec3 normal;
    Real dist;
};

typedef Plane_<Real>   Plane;
typedef Plane_<Float>  Plane_f;
typedef Plane_<Double> Plane_d;

}