// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Geom.h"

namespace honey
{

/// Geometric intersection
template<class Real>
class Intersect_
{
    typedef typename Numeral<Real>::RealT RealT;
    typedef Alge_<Real> Alge;
    typedef Trig_<Real> Trig;
    typedef Geom_<Real> Geom;

    Intersect_() {}

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

    /// \name Test for intersection
    /// Inexact tests are fast but give false positives in certain shape configurations.
    /// @{

    /// Inexact
    static bool test(const Frustum& frustum1,        const Frustum& frustum2);
    /// Inexact
    static bool test(const Frustum& frustum,         const OrientBox& box);
    /// Inexact
    static bool test(const Frustum& frustum,         const Capsule& capsule);
    /// Inexact
    static bool test(const Frustum& frustum,         const Cone& cone);
    /// Inexact
    static bool test(const Frustum& frustum,         const Cylinder& cylinder);
    /// Inexact
    static bool test(const Frustum& frustum,         const Box& box);
    static bool test(const Frustum& frustum,         const Sphere& sphere);
    /// Inexact
    static bool test(const Frustum& frustum,         const Triangle& tri);
    static bool test(const Frustum& frustum,         const Plane& plane);
    /// Inexact
    static bool test(const Frustum& frustum,         const Line& line);
    /// Inexact
    static bool test(const Frustum& frustum,         const Ray& ray);
    static bool test(const Frustum& frustum,         const Vec3& point);
    static bool test(const OrientBox& box1,          const OrientBox& box2);
    static bool test(const OrientBox& box,           const Capsule& capsule);
    static bool test(const OrientBox& box1,          const Box& box2);
    static bool test(const OrientBox& box,           const Sphere& sphere);
    static bool test(const OrientBox& box,           const Plane& plane);
    static bool test(const OrientBox& box,           const Vec3& point);
    static bool test(const Capsule& capsule1,        const Capsule& capsule2);
    static bool test(const Capsule& capsule,         const Box& box);
    static bool test(const Capsule& capsule,         const Sphere& sphere);
    static bool test(const Capsule& capsule,         const Plane& plane);
    static bool test(const Capsule& capsule,         const Vec3& point);
    static bool test(const Cone& cone,               const Sphere& sphere);
    static bool test(const Cone& cone,               const Triangle& tri);
    static bool test(const Cone& cone,               const Plane& plane);
    static bool test(const Cone& cone,               const Vec3& point);
    static bool test(const Cylinder& cylinder,       const Plane& plane);
    static bool test(const Cylinder& cylinder,       const Vec3& point);
    static bool test(const Box& box1,                const Box& box2);
    static bool test(const Box& box,                 const Sphere& sphere);
    static bool test(const Box& box,                 const Triangle& tri);
    static bool test(const Box& box,                 const Plane& plane);
    static bool test(const Box& box,                 const Line& line);
    static bool test(const Box& box,                 const Ray& ray);
    static bool test(const Box& box,                 const Vec3& point)         { return point >= box.min && point <= box.max; }
    static bool test(const Sphere& sphere1,          const Sphere& sphere2)     { return (sphere2.center - sphere1.center).lengthSqr() <= Alge::sqr(sphere1.radius + sphere2.radius); }
    static bool test(const Sphere& sphere,           const Triangle& tri)       { return distanceSqr(tri, sphere.center) <= Alge::sqr(sphere.radius); }
    static bool test(const Sphere& sphere,           const Plane& plane)        { return Alge::abs(distance(plane, sphere.center)) <= sphere.radius; }
    static bool test(const Sphere& sphere,           const Line& line);
    static bool test(const Sphere& sphere,           const Ray& ray);
    static bool test(const Sphere& sphere,           const Vec3& point)         { return (point - sphere.center).lengthSqr() <= Alge::sqr(sphere.radius); }
    static bool test(const Triangle& tri1,           const Triangle& tri2);
    static bool test(const Triangle& tri,            const Plane& plane);
    static bool test(const Triangle& tri,            const Line& line);
    static bool test(const Triangle& tri,            const Ray& ray);
    /// @}

    /// \name Shortest distance
    /// Distances to planes/triangles will be negative if closest point on shape is on negative side
    /// @{
    static Real distance(const Frustum& frustum,        const Box& box,         option<Vec3&> frustumPoint = optnull,   option<Vec3&> boxPoint = optnull);
    static Real distance(const Frustum& frustum,        const Sphere& sphere,   option<Vec3&> frustumPoint = optnull,   option<Vec3&> spherePoint = optnull);
    static Real distance(const Frustum& frustum,        const Vec3& point,      option<Vec3&> frustumPoint = optnull);
    static Real distance(const OrientBox& box,          const Line& line,       option<Vec3&> boxPoint = optnull,       option<Vec3&> linePoint = optnull);
    static Real distance(const OrientBox& box,          const Ray& ray,         option<Vec3&> boxPoint = optnull,       option<Vec3&> rayPoint = optnull);
    static Real distance(const OrientBox& box,          const Vec3& point,      option<Vec3&> boxPoint = optnull);
    static Real distance(const Capsule& capsule,        const Line& line,       option<Vec3&> capsulePoint = optnull,   option<Vec3&> linePoint = optnull);
    static Real distance(const Capsule& capsule,        const Ray& ray,         option<Vec3&> capsulePoint = optnull,   option<Vec3&> rayPoint = optnull);
    static Real distance(const Capsule& capsule,        const Vec3& point,      option<Vec3&> capsulePoint = optnull);
    static Real distance(const Box& box1,               const Box& box2,        option<Vec3&> boxPoint1 = optnull,      option<Vec3&> boxPoint2 = optnull);
    static Real distance(const Box& box,                const Sphere& sphere,   option<Vec3&> boxPoint = optnull,       option<Vec3&> spherePoint = optnull);
    static Real distance(const Box& box,                const Triangle& tri,    option<Vec3&> boxPoint = optnull,       option<Vec3&> triPoint = optnull);
    static Real distance(const Box& box,                const Plane& plane,     option<Vec3&> boxPoint = optnull,       option<Vec3&> planePoint = optnull);
    static Real distance(const Box& box,                const Line& line,       option<Vec3&> boxPoint = optnull,       option<Vec3&> linePoint = optnull);
    static Real distance(const Box& box,                const Ray& ray,         option<Vec3&> boxPoint = optnull,       option<Vec3&> rayPoint = optnull);
    static Real distance(const Box& box,                const Vec3& point,      option<Vec3&> boxPoint = optnull);
    static Real distance(const Sphere& sphere1,         const Sphere& sphere2,  option<Vec3&> spherePoint1 = optnull,   option<Vec3&> spherePoint2 = optnull);
    static Real distance(const Sphere& sphere,          const Triangle& tri,    option<Vec3&> spherePoint = optnull,    option<Vec3&> triPoint = optnull);
    static Real distance(const Sphere& sphere,          const Plane& plane,     option<Vec3&> spherePoint = optnull,    option<Vec3&> planePoint = optnull);
    static Real distance(const Sphere& sphere,          const Line& line,       option<Vec3&> spherePoint = optnull,    option<Vec3&> linePoint = optnull);
    static Real distance(const Sphere& sphere,          const Ray& ray,         option<Vec3&> spherePoint = optnull,    option<Vec3&> rayPoint = optnull);
    static Real distance(const Sphere& sphere,          const Vec3& point,      option<Vec3&> spherePoint = optnull);
    static Real distance(const Triangle& tri1,          const Triangle& tri2,   option<Vec3&> triPoint1 = optnull,      option<Vec3&> triPoint2 = optnull);
    static Real distance(const Triangle& tri,           const Line& line,       option<Vec3&> triPoint = optnull,       option<Vec3&> linePoint = optnull);
    static Real distance(const Triangle& tri,           const Ray& ray,         option<Vec3&> triPoint = optnull,       option<Vec3&> rayPoint = optnull);
    static Real distance(const Triangle& tri,           const Vec3& point,      option<Vec3&> triPoint = optnull);
    static Real distance(const Plane& plane,            const Vec3& point,      option<Vec3&> planePoint = optnull);
    static Real distance(const Line& line1,             const Line& line2,      option<Vec3&> linePoint1 = optnull,     option<Vec3&> linePoint2 = optnull);
    static Real distance(const Line& line,              const Ray& ray,         option<Vec3&> linePoint = optnull,      option<Vec3&> rayPoint = optnull);
    static Real distance(const Line& line,              const Vec3& point,      option<Vec3&> linePoint = optnull);
    static Real distance(const Ray& ray1,               const Ray& ray2,        option<Vec3&> rayPoint1 = optnull,      option<Vec3&> rayPoint2 = optnull);
    static Real distance(const Ray& ray,                const Vec3& point,      option<Vec3&> rayPoint = optnull);
    static Real distance(const Vec3& point1,            const Vec3& point2)     { return Alge::sqrt(distanceSqr(point1, point2)); }
    /// @}

    /// \name Square of shortest distance
    /// @{
    static Real distanceSqr(const OrientBox& box,   const Line& line,       option<Vec3&> boxPoint = optnull,       option<Vec3&> linePoint = optnull);
    static Real distanceSqr(const OrientBox& box,   const Ray& ray,         option<Vec3&> boxPoint = optnull,       option<Vec3&> rayPoint = optnull);
    static Real distanceSqr(const OrientBox& box,   const Vec3& point,      option<Vec3&> boxPoint = optnull);
    static Real distanceSqr(const Box& box1,        const Box& box2,        option<Vec3&> boxPoint1 = optnull,      option<Vec3&> boxPoint2 = optnull);
    static Real distanceSqr(const Box& box,         const Line& line,       option<Vec3&> boxPoint = optnull,       option<Vec3&> linePoint = optnull);
    static Real distanceSqr(const Box& box,         const Ray& ray,         option<Vec3&> boxPoint = optnull,       option<Vec3&> rayPoint = optnull);
    static Real distanceSqr(const Box& box,         const Vec3& point,      option<Vec3&> boxPoint = optnull);
    static Real distanceSqr(const Triangle& tri,    const Vec3& point,      option<Vec3&> triPoint = optnull);
    static Real distanceSqr(const Line& line1,      const Line& line2,      option<Vec3&> linePoint1 = optnull,     option<Vec3&> linePoint2 = optnull);
    static Real distanceSqr(const Line& line,       const Ray& ray,         option<Vec3&> linePoint = optnull,      option<Vec3&> rayPoint = optnull);
    static Real distanceSqr(const Line& line,       const Vec3& point,      option<Vec3&> linePoint = optnull);
    static Real distanceSqr(const Ray& ray1,        const Ray& ray2,        option<Vec3&> rayPoint1 = optnull,      option<Vec3&> rayPoint2 = optnull);
    static Real distanceSqr(const Ray& ray,         const Vec3& point,      option<Vec3&> rayPoint = optnull);
    static Real distanceSqr(const Vec3& point1,     const Vec3& point2)         { return (point2 - point1).lengthSqr(); }
    /// @}

    /// \name Find intersection
    /// @{
    static bool find(const OrientBox& box,       const Triangle& tri,   option<int&> pointCount = optnull,      Vec3 intPoints[3] = nullptr);
    static bool find(const OrientBox& box,       const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const OrientBox& box,       const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Capsule& capsule,     const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Capsule& capsule,     const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Cone& cone,           const Sphere& sphere,  option<Vec3&> intPoint = optnull);
    static bool find(const Cone& cone,           const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Cone& cone,           const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Cylinder& cylinder,   const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Cylinder& cylinder,   const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Box& box1,            const Box& box2,       option<Box&> intBox = optnull);

    static bool find(const Box& box,             const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull,
                                                                        option<Vec3&> intNormal1 = optnull,     option<Vec3&> intNormal2 = optnull);

    static bool find(const Box& box,             const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull,
                                                                        option<Vec3&> intNormal1 = optnull,     option<Vec3&> intNormal2 = optnull);

    static bool find(const Sphere& sphere,       const Line& line,      option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Sphere& sphere,       const Ray& ray,        option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Triangle& tri1,       const Triangle& tri2,  option<Vec3&> intPoint1 = optnull,      option<Vec3&> intPoint2 = optnull);
    static bool find(const Triangle& tri,        const Plane& plane,    option<int&> pointCount = optnull,      Vec3 intPoints[3] = nullptr);
    static bool find(const Triangle& tri,        const Line& line,      option<Vec3&> intPoint = optnull);
    static bool find(const Triangle& tri,        const Ray& ray,        option<Vec3&> intPoint = optnull);
    static bool find(const Plane& plane1,        const Plane& plane2,   option<Ray&> intRay = optnull);
    static bool find(const Plane& plane,         const Line& line,      option<Vec3&> intPoint = optnull);
    static bool find(const Plane& plane,         const Ray& ray,        option<Vec3&> intPoint = optnull);
    /// @}

private:
    // Many of these algorithms are adapted from the Wild Magic lib

    /// Support for fast Box-Ray test
    static bool pluecker(const Box& box, const Ray& ray, option<Real&> t = optnull);

    struct OrientBoxRayDist
    {
        static void face(   const Vec3& extent, Real& lineParam, int i0, int i1, int i2,
                            Vec3& pnt, const Vec3& dir, const Vec3& PmE, Real& sqrDistance);

        static void caseNoZeros(const Vec3& extent, Real& lineParam,
                                Vec3& pnt, const Vec3& dir, Real& sqrDistance);

        static void case0(  const Vec3& extent, Real& lineParam,
                            int i0, int i1, int i2, Vec3& pnt, const Vec3& dir, Real& sqrDistance);

        static void case00( const Vec3& extent, Real& lineParam,
                            int i0, int i1, int i2, Vec3& pnt, const Vec3& dir, Real& sqrDistance);

        static void case000(const Vec3& extent, Real& lineParam,
                            Vec3& pnt, Real& sqrDistance);
    };

    /// \name Support for Shape-Ray intersection.
    /// @{
    static int  find(const OrientBox& box,      const Ray& ray, Real t[2]);
    static int  find(const Capsule& capsule,    const Ray& ray, Real t[2]);
    static bool find(const Cone& cone,          const Ray& ray, Real t[2]);
    static int  find(const Cylinder& cylinder,  const Ray& ray, Real t[2]);
    static bool find(const Box& box,            const Ray& ray, Real t[2], Vec3 norm[2]);
    static bool find(const Sphere& sphere,      const Ray& ray, Real t[2]);
    /// @}

    struct OrientBoxTriangleInt
    {
        static void clipConvexPolygonAgainstPlane(const Vec3& normal, Real constant, int& quantity, Vec3* P);
    };

    struct OrientBoxRayInt
    {
        static bool clip(Real denom, Real numer, Real t[2]);
    };

    struct BoxTriangleInt
    {
        static bool planeBoxOverlap(const Vec3& normal, const Real d, const Vec3& maxbox);
    };

    /// Support for Triangle-Triangle intersection
    struct TriangleInt
    {
        static void projectOntoAxis(const Triangle& tri, const Vec3& axis, Real& rfMin, Real& rfMax);
        static void trianglePlaneRelations(const Triangle& triangle, const Plane& plane, Real distances[3], int signs[3], int& positive, int& negative, int& zero);
        static void interval(const Triangle& triangle, const Ray& line, const Real distances[3], const int signs[3], Real params[2]);
    };
};


template<class Real>
inline bool Intersect_<Real>::test(const Box& box1, const Box& box2)
{
    return !(box1.max.x < box2.min.x || box1.min.x > box2.max.x
            || box1.max.y < box2.min.y || box1.min.y > box2.max.y
            || box1.max.z < box2.min.z || box1.min.z > box2.max.z);
}

template<class Real>
inline bool Intersect_<Real>::test(const Box& box, const Sphere& sphere)
{
    Real d = 0;
    
    if (sphere.center.x < box.min.x)
        d += Alge::sqr(sphere.center.x - box.min.x);
    else if (sphere.center.x > box.max.x)
        d += Alge::sqr(sphere.center.x - box.max.x);

    if (sphere.center.y < box.min.y)
        d += Alge::sqr(sphere.center.y - box.min.y);
    else if (sphere.center.y > box.max.y)
        d += Alge::sqr(sphere.center.y - box.max.y);

    if (sphere.center.z < box.min.z)
        d += Alge::sqr(sphere.center.z - box.min.z);
    else if (sphere.center.z > box.max.z)
        d += Alge::sqr(sphere.center.z - box.max.z);

    return d <= Alge::sqr(sphere.radius);
}

typedef Intersect_<Real>         Intersect;
typedef Intersect_<Float>        Intersect_f;
typedef Intersect_<Double>       Intersect_d;


}
