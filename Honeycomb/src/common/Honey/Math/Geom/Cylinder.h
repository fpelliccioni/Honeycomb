// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Plane.h"
#include "Honey/Math/Geom/Sphere.h"

namespace honey
{

template<class Real> class Alge_;
template<class Real> class Sphere_;

/// 3D cylinder geometry class.  Cylinder end points are at `center +- axis*(height/2)`.
template<class Real>
class Cylinder_ : public Shape<Real>
{
    typedef Alge_<Real>     Alge;
    typedef Vec<2,Real>     Vec2;
    typedef Vec<3,Real>     Vec3;
    typedef Line_<Real>     Line;
    typedef Plane_<Real>    Plane;
    typedef Sphere_<Real>   Sphere;

public:
    ///No initialization
    Cylinder_() {}
    
    ///Construct from center, unit axis, radius, and height
    Cylinder_(const Vec3& center, const Vec3& axis, Real radius, Real height)   : center(center), axis(axis), radius(radius), height(height)  {}

    ~Cylinder_() {}

    /// Number of planes (constant)
    static const int planeCount = 2;

    /// Get cylinder bottom/top planes with normals facing outwards.  Bottom = 0, Top = 1
    Plane plane(int index) const                                                { return (index == 0) ? Plane(-axis, center - 0.5*height*axis) : Plane(axis, center + 0.5*height*axis); }

    /// Get line along axis from bottom plane to top plane
    Line axisLine() const                                                       { return Line(center - 0.5*height*axis, center + 0.5*height*axis); }

    /// Get bounding sphere
    Sphere toSphere() const;

    /// Transform operator
    friend Cylinder_ operator*(const Matrix<4,4,Real>& mat, const Cylinder_& cyl)
    {
        Vec3 perpAxis = Alge::abs(cyl.axis.dot(Vec3::axisX)) < 0.5 ? cyl.axis.cross(Vec3::axisX) : cyl.axis.cross(Vec3::axisY);
        return Cylinder_(mat*cyl.center, mat.mulRotScale(cyl.axis).normalize(), cyl.radius*mat.mulRotScale(perpAxis).length(), cyl.height*mat.mulRotScale(cyl.axis).length());
    }
    friend Cylinder_ operator*(const Transform_<Real>& tm, const Cylinder_& cyl)
    {
        if (tm.hasScale())
        {
            Vec3 perpAxis = Alge::abs(cyl.axis.dot(Vec3::axisX)) < 0.5 ? cyl.axis.cross(Vec3::axisX) : cyl.axis.cross(Vec3::axisY);
            return Cylinder_(tm*cyl.center, tm.mulRotScale(cyl.axis).normalize(), cyl.radius*tm.mulRotScale(perpAxis).length(), cyl.height*tm.mulRotScale(cyl.axis).length());
        }
        else
            return Cylinder_(tm*cyl.center, tm.mulRotScale(cyl.axis), cyl.radius, cyl.height);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Cylinder_& val)
    {
        return os << "{ center" << val.center << ", axis" << val.axis << ", radius(" << val.radius << "), height(" << val.height << ") }";
    }

public:
    Vec3 center;    ///< center point of cylinder
    Vec3 axis;      ///< cyclindrical axis
    Real radius;    ///< radius around axis
    Real height;    ///< total extent of cylinder along axis
};    

typedef Cylinder_<Real>    Cylinder;
typedef Cylinder_<Float>   Cylinder_f;
typedef Cylinder_<Double>  Cylinder_d;

}
