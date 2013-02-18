// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Plane.h"

namespace honey
{

template<class Real> class Alge_;
template<class Real> class Trig_;
template<class Real> class Sphere_;

/// 3D cone geometry class
template<class Real>
class Cone_ : public Shape<Real>
{
    typedef Alge_<Real>     Alge;
    typedef Trig_<Real>     Trig;
    typedef Vec<2,Real>     Vec2;
    typedef Vec<3,Real>     Vec3;
    typedef Line_<Real>     Line;
    typedef Plane_<Real>    Plane;
    typedef Sphere_<Real>   Sphere;

public:
    ///No initialization
    Cone_() {}
    
    ///Construct from vertex (origin), unit axis (direction), cutoff angle in range [0,PI/2], and height (extent along axis)
    Cone_(const Vec3& vertex, const Vec3& axis, Real angle, Real height)    : vertex(vertex), axis(axis), angle(angle), height(height)  {}

    ~Cone_() {}

    /// Number of planes (constant)
    static const int planeCount = 2;

    /// Get cone vertex/base planes with normals facing outwards.  vertex = 0, base = 1
    Plane plane(int index) const                                            { return (index == 0) ? Plane(-axis, vertex) : Plane(axis, vertex + axis*height); }

    /// Get line along axis from vertex plane to base plane
    Line axisLine() const                                                   { return Line(vertex, vertex + axis*height); }

    /// Get radius at base of cone (ie. max radius)
    Real radius() const                                                     { return Trig::tan(angle)*height; }

    /// Get bounding sphere
    Sphere toSphere() const;

    /// Transform operator
    friend Cone_ operator*(const Matrix<4,4,Real>& mat, const Cone_& cone)
    {
        Vec3 perpAxis = Alge::abs(cone.axis.dot(Vec3::axisX)) < 0.5 ? cone.axis.cross(Vec3::axisX) : cone.axis.cross(Vec3::axisY);
        return Cone_(mat*cone.vertex, mat.mulRotScale(cone.axis).normalize(), Trig::atan(Trig::tan(cone.angle)*mat.mulRotScale(perpAxis).length()), cone.height*mat.mulRotScale(cone.axis).length());
    }
    friend Cone_ operator*(const Transform_<Real>& tm, const Cone_& cone)
    {
        if (tm.hasScale())
        {
            Vec3 perpAxis = Alge::abs(cone.axis.dot(Vec3::axisX)) < 0.5 ? cone.axis.cross(Vec3::axisX) : cone.axis.cross(Vec3::axisY);
            return Cone_(tm*cone.vertex, tm.mulRotScale(cone.axis).normalize(), Trig::atan(Trig::tan(cone.angle)*tm.mulRotScale(perpAxis).length()), cone.height*tm.mulRotScale(cone.axis).length());
        }
        else
            return Cone_(tm*cone.vertex, tm.mulRotScale(cone.axis), cone.angle, cone.height);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Cone_& val)
    {
        return os << "{ vertex" << val.vertex << ", axis" << val.axis << ", angle(" << val.angle << "), height(" << val.height << ") }";
    }

public:
    Vec3 vertex;    ///< origin
    Vec3 axis;      ///< direction
    Real angle;     ///< cutoff angle
    Real height;    ///< extent of cone along axis
};    

typedef Cone_<Real>    Cone;
typedef Cone_<Float>   Cone_f;
typedef Cone_<Double>  Cone_d;


}
