// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Line.h"

namespace honey
{

template<class Real> class Sphere_;

/// 3D capsule geometry class. A capsule is a cyclinder with spherical ends.
template<class Real>
class Capsule_ : public Shape<Real>
{
    typedef Line_<Real>     Line;
    typedef Vec<3,Real>     Vec3;
    typedef Sphere_<Real>   Sphere;

public:
    ///No initialization
    Capsule_()                                                                  {}
    
    ///Construct from line segment and radius
    Capsule_(const Line& line, Real radius)                                     : line(line), radius(radius)    {}

    ~Capsule_()                                                                 {}

    /// Get bounding sphere
    Sphere toSphere() const;

    /// Transform operator
    friend Capsule_ operator*(const Matrix<4,4,Real>& mat, const Capsule_& cap)
    {
        static const Vec3 one = Vec3::one.normalize();
        return Capsule_(mat*cap.line, cap.radius*mat.mulRotScale(one).length());
    }
    friend Capsule_ operator*(const Transform_<Real>& tm, const Capsule_& cap)
    {
        static const Vec3 one = Vec3::one.normalize();
        return Capsule_(tm*cap.line, tm.hasScale() ? cap.radius*tm.mulRotScale(one).length() : cap.radius);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Capsule_& val)
    {
        return os << "{ line" << val.line << ", radius(" << val.radius << ") }";
    }

public:
    Line line;
    Real radius;
};    


typedef Capsule_<Real>     Capsule;
typedef Capsule_<Float>    Capsule_f;
typedef Capsule_<Double>   Capsule_d;


}