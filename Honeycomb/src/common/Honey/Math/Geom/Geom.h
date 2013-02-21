// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Alge/Trig.h"
#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Ray.h"
#include "Honey/Math/Geom/Plane.h"
#include "Honey/Math/Geom/Triangle.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Geom/Box.h"
#include "Honey/Math/Geom/Cylinder.h"
#include "Honey/Math/Geom/Cone.h"
#include "Honey/Math/Geom/Capsule.h"
#include "Honey/Math/Geom/OrientBox.h"
#include "Honey/Math/Geom/Frustum.h"

namespace honey
{

template<class Real> class Intersect_;

/// Geometry math
template<class Real>
class Geom_ : mt::NoCopy
{
    typedef typename Numeral<Real>::Real_ Real_;
    typedef Alge_<Real> Alge;
    typedef Trig_<Real> Trig;
    typedef Intersect_<Real> Intersect;

public:
    typedef Vec<3,Real>         Vec3;
    typedef Line_<Real>         Line;
    typedef Ray_<Real>          Ray;
    typedef Plane_<Real>        Plane;
    typedef Triangle_<Real>     Triangle;
    typedef Sphere_<Real>       Sphere;
    typedef Box_<Real>          Box;
    typedef Cylinder_<Real>     Cylinder;
    typedef Cone_<Real>         Cone;
    typedef Capsule_<Real>      Capsule;
    typedef OrientBox_<Real>    OrientBox;
    typedef Frustum_<Real>      Frustum;

    /// Side of plane that shape is on.  Returns `pos` if any part of the shape is on the positive side or on the plane.
    static typename Plane::Side side(const Triangle& triPlane,  const Box& box)             { return side(Plane(triPlane), box); }
    static typename Plane::Side side(const Triangle& triPlane,  const Sphere& sphere)       { return side(Plane(triPlane), sphere); }
    static typename Plane::Side side(const Triangle& triPlane,  const Triangle& tri)        { return side(Plane(triPlane), tri); }
    static typename Plane::Side side(const Triangle& triPlane,  const Vec3& point)          { return side(Plane(triPlane), point); }
    static typename Plane::Side side(const Plane& plane,        const Frustum& frustum);
    static typename Plane::Side side(const Plane& plane,        const OrientBox& box);
    static typename Plane::Side side(const Plane& plane,        const Capsule& capsule);
    static typename Plane::Side side(const Plane& plane,        const Cone& cone);
    static typename Plane::Side side(const Plane& plane,        const Cylinder& cylinder);
    static typename Plane::Side side(const Plane& plane,        const Box& box);
    static typename Plane::Side side(const Plane& plane,        const Sphere& sphere);
    static typename Plane::Side side(const Plane& plane,        const Triangle& tri);
    static typename Plane::Side side(const Plane& plane,        const Line& line);
    static typename Plane::Side side(const Plane& plane,        const Ray& ray);
    static typename Plane::Side side(const Plane& plane,        const Vec3& point);

    /// Project onto plane
    static Triangle project(const Plane& plane,     const Triangle& tri)                    { return Triangle(project(plane, tri.v0), project(plane, tri.v1), project(plane, tri.v2)); }
    static Line     project(const Plane& plane,     const Line& line)                       { return Line(project(plane, line.v0), project(plane, line.v1)); }
    static Vec3  project(const Plane& plane,     const Vec3& point)                         { return point - plane.normal*(plane.normal.dot(point) - plane.dist); }
 
    /// Project point onto line. Returns the projected point on the line and the parametric t along line (no line bounds check)
    static Vec3 project(const Line& line, const Vec3& point, option<Real&> t = optnull);
    static Vec3 project(const Ray& ray,   const Vec3& point, option<Real&> t = optnull);

    /// Returns vector `v` reflected by a plane with normal `n`
    template<class Vector> static Vector reflect(const Vector& v, const Vector& n)          { return v - (n.dot(v) * 2 * n); }

    /// Returns vector `v` refracted by a plane with normal `n` with refraction index `eta`
    template<class Vector> static Vector refract(const Vector& v, const Vector& n, Real eta)
    {
        Real dot = v.dot(n);
        Real k = 1 - (1 - dot*dot)*eta*eta;
        return (k < 0) ? Vector::zero : v*eta - n*(eta*dot + Alge::sqrt(k));
    }
};

typedef Geom_<Real>         Geom;
typedef Geom_<Float>        Geom_f;
typedef Geom_<Double>       Geom_d;


}
