// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Shape.h"
#include "Honey/Math/Alge/Transform.h"

namespace honey
{

template<class Real> class Box_;

/// 3D sphere geometry class
template<class Real>
class Sphere_ : public Shape<Real>
{
    typedef typename Numeral<Real>::RealT RealT;
    typedef Alge_<Real>     Alge;
    typedef Vec<3,Real>     Vec3;
    typedef Box_<Real>      Box;

public:
    /// No initialization
    Sphere_()                                                       {}

    /// Construct from center point and radius
    Sphere_(const Vec3& center, Real radius)                        : center(center), radius(radius) {}

    /// Construct a sphere that bounds a cloud of points
    Sphere_(const Vec3* vs, int count)                              { fromPoints(vs, count); }

    ~Sphere_()                                                      {}

    /// Construct a sphere that bounds a cloud of points
    Sphere_& fromPoints(const Vec3* vs, int count);

    /// Calculates a minimum sphere center/radius that encompasses this sphere and the sphere passed in
    Sphere_ minSphere(const Sphere_& sphere);

    /// Extend the sphere radius to include another sphere
    void extend(const Sphere_& sphere)                              { Alge::max((sphere.center - center).length() + sphere.radius, radius); }

    /// Extend the sphere radius to include a point
    void extend(const Vec3& p)                                      { Alge::max((p - center).length(), radius); }

    /// Calc sphere surface area
    Real area() const                                               { return 4 * RealT::pi * radius * radius; }

    /// Calc sphere volume
    Real volume() const                                             { return (4. / 3) * RealT::pi * radius * radius * radius; }

    /// Get bounding box
    Box toBox() const;

    /// Transform operator
    friend Sphere_ operator*(const Matrix<4,4,Real>& mat, const Sphere_& sphere)
    {
        static const Vec3 one = Vec3::one.normalize();
        return Sphere_(mat*sphere.center, mat.mulRotScale(one).length()*sphere.radius);
    }
    friend Sphere_ operator*(const Transform_<Real>& tm, const Sphere_& sphere)
    {
        static const Vec3 one = Vec3::one.normalize();
        return Sphere_(tm*sphere.center, tm.hasScale() ? sphere.radius*tm.mulRotScale(one).length() : sphere.radius);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Sphere_& val)
    {
        return os << "{ center" << val.center << ", radius(" << val.radius << ") }";
    }

public:
    Vec3 center;
    Real radius;
};

typedef Sphere_<Real>      Sphere;
typedef Sphere_<Float>     Sphere_f;
typedef Sphere_<Double>    Sphere_d;

}
