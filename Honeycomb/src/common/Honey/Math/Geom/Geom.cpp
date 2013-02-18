// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Geom.h"
#include "Honey/Math/Geom/Intersect.h"

namespace honey
{

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Frustum& frustum)
{
    //Check if any points are on positive side
    for (int i = 0; i < frustum.vertexCount; ++i)
        if (side(plane, frustum.vertex(i)) == Plane::Side::pos)
            return Plane::Side::pos;
    return Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const OrientBox& box)
{
    Real tmp[3] =
    {
        box.extent[0]*(plane.normal.dot(box.axis[0])),
        box.extent[1]*(plane.normal.dot(box.axis[1])),
        box.extent[2]*(plane.normal.dot(box.axis[2]))
    };

    Real radius = Alge::abs(tmp[0]) + Alge::abs(tmp[1]) + Alge::abs(tmp[2]);
    Real signedDistance = Intersect::distance(plane, box.center);
    return signedDistance >= -radius ? Plane::Side::pos : Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Capsule& capsule)
{
    Real dist = Intersect::distance(plane, capsule.line.v1);
    if (dist < 0)
    {
        Real nDist = Intersect::distance(plane, capsule.line.v0);
        if (nDist < 0)
        {
            if (dist <= nDist)
                return dist >= -capsule.radius ? Plane::Side::pos : Plane::Side::neg;
            else
                return nDist >= -capsule.radius ? Plane::Side::pos : Plane::Side::neg;
        }
    }

    return Plane::Side::pos;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Cone& cone)
{
    return (side(plane, cone.vertex) == Plane::Side::pos || Intersect::test(cone, plane)) ? Plane::Side::pos : Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Cylinder& cylinder)
{
    // Compute extremes of signed distance dot(N,X)-d for points on the
    // cylinder.  These are
    //   min = (dot(N,C)-d) - r*sqrt(1-dot(N,W)^2) - (h/2)*|dot(N,W)|
    //   max = (dot(N,C)-d) + r*sqrt(1-dot(N,W)^2) + (h/2)*|dot(N,W)|
    Real dist = Intersect::distance(plane, cylinder.center);
    Real absNdW = Alge::abs(plane.normal.dot(cylinder.axis));
    Real root = Alge::sqrt(Alge::abs(1 - absNdW*absNdW));
    Real term = cylinder.radius*root + 0.5*cylinder.height*absNdW;

    // Culling occurs if and only if max <= 0.
    return dist + term >= 0 ? Plane::Side::pos : Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Box& box)
{
    //Check if any points are on positive side
    for (int i = 0; i < box.vertexCount; ++i)
        if (side(plane, box.vertex(i)) == Plane::Side::pos)
            return Plane::Side::pos;
    return Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Sphere& sphere)
{
    return Intersect::distance(plane, sphere.center) >= -sphere.radius ? Plane::Side::pos : Plane::Side::neg;
} 

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Triangle& tri)
{
    //Check if any points are on positive side
    for (int i = 0; i < tri.vertexCount; ++i)
        if (side(plane, tri.vertex(i)) == Plane::Side::pos)
            return Plane::Side::pos;
    return Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Line& line)
{
    //Check if any points are on positive side
    for (int i = 0; i < line.pointCount; ++i)
        if (side(plane, line.point(i)) == Plane::Side::pos)
            return Plane::Side::pos;
    return Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Ray& ray)
{
    return (side(plane, ray.origin) == Plane::Side::pos || ray.dir.dot(plane.normal) > 0) ? Plane::Side::pos : Plane::Side::neg;
}

template<class Real>
typename Plane_<Real>::Side Geom_<Real>::side(const Plane& plane, const Vec3& point)
{
    return Intersect::distance(plane, point) >= 0 ? Plane::Side::pos : Plane::Side::neg;
}

template<class Real>
typename Geom_<Real>::Vec3 Geom_<Real>::project(const Line& line, const Vec3& point, option<Real&> t_)
{
    Vec3 dir = line.diff();
    Real t = 0;
    Real div = dir.lengthSqr();
    if (div != 0)
        t = (point-line[0]).dot(dir) / div;

    if (t_) t_ = t;
    return line[0] + dir*t;
}

template<class Real>
typename Geom_<Real>::Vec3 Geom_<Real>::project(const Ray& ray, const Vec3& point, option<Real&> t_)
{
    Real t = (point - ray.origin).dot(ray.dir);
    if (t_) t_ = t;
    return ray.origin + ray.dir*t;
}


template class Geom_<Float>;
template class Geom_<Double>;


}
