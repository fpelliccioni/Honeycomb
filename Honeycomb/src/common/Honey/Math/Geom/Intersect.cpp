// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Intersect.h"

namespace honey
{

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum1, const Frustum& frustum2)
{
    for (int i = 0; i < frustum1.planeCount; ++i)
        if (Geom::side(frustum1.plane(i), frustum2) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const OrientBox& box)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), box) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Capsule& capsule)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), capsule) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Cone& cone)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), cone) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Cylinder& cylinder)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), cylinder) != Plane::Side::pos)
            return false;
    return true;
}


template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Box& box)
{
    for (int i = 0; i < frustum.planeCount; ++i)
    {
        auto& plane = frustum.plane(i);
        Vec3 max;
        max.x = plane.normal.x > 0 ? box.max.x : box.min.x;
        max.y = plane.normal.y > 0 ? box.max.y : box.min.y;
        max.z = plane.normal.z > 0 ? box.max.z : box.min.z;
        if (Geom::side(plane, max) != Plane::Side::pos)
            return false;
    }
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Sphere& sphere)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), sphere) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Triangle& tri)
{
    //Plane test is inaccurate, test box first
    if (!test(frustum.box(), tri.toBox())) return false;
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), tri) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Plane& plane)
{
    // There is an intersection if a vertex is on the opposite side of another
    typename Plane::Side side = Geom::side(plane, frustum.vertex(0));
    for (int i = 1; i < frustum.vertexCount; ++i)
        if (Geom::side(plane, frustum.vertex(i)) != side)
            return true;
    return false;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Line& line)
{
    //Plane test is inaccurate, test box first
    if (!test(frustum.box(), line)) return false;
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), line) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Ray& ray)
{
    //Plane test is inaccurate, test box first
    if (!test(frustum.box(), ray)) return false;
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), ray) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Frustum& frustum, const Vec3& point)
{
    for (int i = 0; i < frustum.planeCount; ++i)
        if (Geom::side(frustum.plane(i), point) != Plane::Side::pos)
            return false;
    return true;
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box1, const OrientBox& box2)
{
    // Cutoff for cosine of angles between box axes.  This is used to catch
    // the cases when at least one pair of axes are parallel.  If this
    // happens, there is no need to test for separation along the
    // cross(A[i],B[j]) directions.
    const Real cutoff = 1 - Real_::zeroTol;
    bool existsParallelPair = false;
    int i;

    // Convenience variables.
    const Vec3* A = box1.axis;
    const Vec3* B = box2.axis;
    const Real* EA = box1.extent;
    const Real* EB = box2.extent;

    // Compute difference of box centers, D = C1-C0.
    Vec3 D = box2.center - box1.center;

    Real C[3][3];     // matrix C = A^T B, c_{ij} = dot(A_i,B_j)
    Real AbsC[3][3];  // |c_{ij}|
    Real AD[3];       // dot(A_i,D)
    Real r0, r1, r;   // interval radii and distance between centers
    Real r01;         // = R0 + R1

    // axis C0+t*A0
    for (i = 0; i < 3; ++i)
    {
        C[0][i] = A[0].dot(B[i]);
        AbsC[0][i] = Alge::abs(C[0][i]);
        if (AbsC[0][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[0] = A[0].dot(D);
    r = Alge::abs(AD[0]);
    r1 = EB[0]*AbsC[0][0] + EB[1]*AbsC[0][1] + EB[2]*AbsC[0][2];
    r01 = EA[0] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1
    for (i = 0; i < 3; ++i)
    {
        C[1][i] = A[1].dot(B[i]);
        AbsC[1][i] = Alge::abs(C[1][i]);
        if (AbsC[1][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[1] = A[1].dot(D);
    r = Alge::abs(AD[1]);
    r1 = EB[0]*AbsC[1][0] + EB[1]*AbsC[1][1] + EB[2]*AbsC[1][2];
    r01 = EA[1] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2
    for (i = 0; i < 3; ++i)
    {
        C[2][i] = A[2].dot(B[i]);
        AbsC[2][i] = Alge::abs(C[2][i]);
        if (AbsC[2][i] > cutoff)
        {
            existsParallelPair = true;
        }
    }
    AD[2] = A[2].dot(D);
    r = Alge::abs(AD[2]);
    r1 = EB[0]*AbsC[2][0] + EB[1]*AbsC[2][1] + EB[2]*AbsC[2][2];
    r01 = EA[2] + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B0
    r = Alge::abs(B[0].dot(D));
    r0 = EA[0]*AbsC[0][0] + EA[1]*AbsC[1][0] + EA[2]*AbsC[2][0];
    r01 = r0 + EB[0];
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B1
    r = Alge::abs(B[1].dot(D));
    r0 = EA[0]*AbsC[0][1] + EA[1]*AbsC[1][1] + EA[2]*AbsC[2][1];
    r01 = r0 + EB[1];
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*B2
    r = Alge::abs(B[2].dot(D));
    r0 = EA[0]*AbsC[0][2] + EA[1]*AbsC[1][2] + EA[2]*AbsC[2][2];
    r01 = r0 + EB[2];
    if (r > r01)
    {
        return false;
    }

    // At least one pair of box axes was parallel, so the separation is
    // effectively in 2D where checking the "edge" normals is sufficient for
    // the separation of the boxes.
    if (existsParallelPair)
    {
        return true;
    }

    // axis C0+t*A0xB0
    r = Alge::abs(AD[2]*C[1][0] - AD[1]*C[2][0]);
    r0 = EA[1]*AbsC[2][0] + EA[2]*AbsC[1][0];
    r1 = EB[1]*AbsC[0][2] + EB[2]*AbsC[0][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A0xB1
    r = Alge::abs(AD[2]*C[1][1] - AD[1]*C[2][1]);
    r0 = EA[1]*AbsC[2][1] + EA[2]*AbsC[1][1];
    r1 = EB[0]*AbsC[0][2] + EB[2]*AbsC[0][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A0xB2
    r = Alge::abs(AD[2]*C[1][2] - AD[1]*C[2][2]);
    r0 = EA[1]*AbsC[2][2] + EA[2]*AbsC[1][2];
    r1 = EB[0]*AbsC[0][1] + EB[1]*AbsC[0][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB0
    r = Alge::abs(AD[0]*C[2][0] - AD[2]*C[0][0]);
    r0 = EA[0]*AbsC[2][0] + EA[2]*AbsC[0][0];
    r1 = EB[1]*AbsC[1][2] + EB[2]*AbsC[1][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB1
    r = Alge::abs(AD[0]*C[2][1] - AD[2]*C[0][1]);
    r0 = EA[0]*AbsC[2][1] + EA[2]*AbsC[0][1];
    r1 = EB[0]*AbsC[1][2] + EB[2]*AbsC[1][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A1xB2
    r = Alge::abs(AD[0]*C[2][2] - AD[2]*C[0][2]);
    r0 = EA[0]*AbsC[2][2] + EA[2]*AbsC[0][2];
    r1 = EB[0]*AbsC[1][1] + EB[1]*AbsC[1][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB0
    r = Alge::abs(AD[1]*C[0][0] - AD[0]*C[1][0]);
    r0 = EA[0]*AbsC[1][0] + EA[1]*AbsC[0][0];
    r1 = EB[1]*AbsC[2][2] + EB[2]*AbsC[2][1];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB1
    r = Alge::abs(AD[1]*C[0][1] - AD[0]*C[1][1]);
    r0 = EA[0]*AbsC[1][1] + EA[1]*AbsC[0][1];
    r1 = EB[0]*AbsC[2][2] + EB[2]*AbsC[2][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    // axis C0+t*A2xB2
    r = Alge::abs(AD[1]*C[0][2] - AD[0]*C[1][2]);
    r0 = EA[0]*AbsC[1][2] + EA[1]*AbsC[0][2];
    r1 = EB[0]*AbsC[2][1] + EB[1]*AbsC[2][0];
    r01 = r0 + r1;
    if (r > r01)
    {
        return false;
    }

    return true;
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box, const Capsule& capsule)
{
    return distanceSqr(box, capsule.line) <= Alge::sqr(capsule.radius);
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box1, const Box& box2)
{
    return test(box1, OrientBox(box2));
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box, const Sphere& sphere)
{
    // Test for intersection in the coordinate system of the box by
    // transforming the sphere into that coordinate system.
    Vec3 cdiff = sphere.center - box.center;

    Real ax = Alge::abs(cdiff.dot(box.axis[0]));
    Real ay = Alge::abs(cdiff.dot(box.axis[1]));
    Real az = Alge::abs(cdiff.dot(box.axis[2]));
    Real dx = ax - box.extent[0];
    Real dy = ay - box.extent[1];
    Real dz = az - box.extent[2];

    if (ax <= box.extent[0])
    {
        if (ay <= box.extent[1])
        {
            if (az <= box.extent[2])
            {
                // Sphere center inside box.
                return true;
            }
            else
            {
                // Potential sphere-face intersection with face z.
                return dz <= sphere.radius;
            }
        }
        else
        {
            if (az <= box.extent[2])
            {
                // Potential sphere-face intersection with face y.
                return dy <= sphere.radius;
            }
            else
            {
                // Potential sphere-edge intersection with edge formed
                // by faces y and z.
                Real rsqr = sphere.radius*sphere.radius;
                return dy*dy + dz*dz <= rsqr;
            }
        }
    }
    else
    {
        if (ay <= box.extent[1])
        {
            if (az <= box.extent[2])
            {
                // Potential sphere-face intersection with face x.
                return dx <= sphere.radius;
            }
            else
            {
                // Potential sphere-edge intersection with edge formed
                // by faces x and z.
                Real rsqr = sphere.radius*sphere.radius;
                return dx*dx + dz*dz <= rsqr;
            }
        }
        else
        {
            if (az <= box.extent[2])
            {
                // Potential sphere-edge intersection with edge formed
                // by faces x and y.
                Real rsqr = sphere.radius*sphere.radius;
                return dx*dx + dy*dy <= rsqr;
            }
            else
            {
                // Potential sphere-vertex intersection at corner formed
                // by faces x,y,z.
                Real rsqr = sphere.radius*sphere.radius;
                return dx*dx + dy*dy + dz*dz <= rsqr;
            }
        }
    }
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box, const Plane& plane)
{
    Real tmp[3] =
    {
        box.extent[0]*(plane.normal.dot(box.axis[0])),
        box.extent[1]*(plane.normal.dot(box.axis[1])),
        box.extent[2]*(plane.normal.dot(box.axis[2]))
    };

    Real radius = Alge::abs(tmp[0]) + Alge::abs(tmp[1]) + Alge::abs(tmp[2]);
    Real signedDistance = distance(plane, box.center);
    return Alge::abs(signedDistance) <= radius;
}

template<class Real>
bool Intersect_<Real>::test(const OrientBox& box, const Vec3& point)
{
    return distanceSqr(box, point) == 0;
}

template<class Real>
bool Intersect_<Real>::test(const Capsule& capsule1, const Capsule& capsule2)
{
    return distanceSqr(capsule1.line, capsule2.line) <= Alge::sqr(capsule1.radius + capsule2.radius);
}

template<class Real>
bool Intersect_<Real>::test(const Capsule& capsule, const Box& box)
{
    return distanceSqr(box, capsule.line) <= Alge::sqr(capsule.radius);
}

template<class Real>
bool Intersect_<Real>::test(const Capsule& capsule, const Sphere& sphere)
{
    //Test if sphere center is within radius of capsule axis line
    return distanceSqr(capsule.line, sphere.center) <= Alge::sqr(capsule.radius + sphere.radius);
}

template<class Real>
bool Intersect_<Real>::test(const Capsule& capsule, const Plane& plane)
{
    Real dist = distance(plane, capsule.line.v1);
    Real nDist = distance(plane, capsule.line.v0);
    if (dist*nDist <= 0)
        // Capsule segment endpoints on opposite sides of the plane.
        return true;

    // Endpoints on same side of plane, but the endpoint spheres (with
    // radius of the capsule) might intersect the plane.
    return Alge::abs(dist) <= capsule.radius || Alge::abs(nDist) <= capsule.radius;
}

template<class Real>
bool Intersect_<Real>::test(const Capsule& capsule, const Vec3& point)
{
    //Test if point is within radius of capsule axis line
    return distanceSqr(capsule.line, point) <= Alge::sqr(capsule.radius);
}

template<class Real>
bool Intersect_<Real>::test(const Cone& cone, const Sphere& sphere)
{
    return find(cone, sphere);
}

template<class Real>
bool Intersect_<Real>::test(const Cone& cone, const Triangle& tri)
{
    // Triangle is <P0,P1,P2>, edges are E0 = P1-P0, E1=P2-P0.
    Real cosAngle = Trig::cos(cone.angle);
    int onConeSide = 0;
    Real p0Test = 0, p1Test = 0, p2Test = 0;
    Real AdE, EdE, EdD, c1, c2;

    Real cosSqr = cosAngle*cosAngle;

    // Test vertex P0.
    Vec3 diff0 = tri[0] - cone.vertex;
    Real AdD0 = cone.axis.dot(diff0);
    if (AdD0 >= 0)
    {
        // P0 is on cone side of plane.
        p0Test = AdD0*AdD0 - cosSqr*(diff0.dot(diff0));
        if (p0Test >= 0)
        {
            // P0 is inside the cone.
            return true;
        }
        else
        {
            // P0 is outside the cone, but on cone side of plane.
            onConeSide |= 1;
        }
    }
    // else P0 is not on cone side of plane.

    // Test vertex P1.
    Vec3 edge0 = tri[1] - tri[0];
    Vec3 diff1 = diff0 + edge0;
    Real AdD1 = cone.axis.dot(diff1);
    if (AdD1 >= 0)
    {
        // P1 is on cone side of plane.
        p1Test = AdD1*AdD1 - cosSqr*(diff1.dot(diff1));
        if (p1Test >= 0)
        {
            // P1 is inside the cone.
            return true;
        }
        else
        {
            // P1 is outside the cone, but on cone side of plane.
            onConeSide |= 2;
        }
    }
    // else P1 is not on cone side of plane.

    // Test vertex P2.
    Vec3 edge1 = tri[2] - tri[0];
    Vec3 diff2 = diff0 + edge1;
    Real AdD2 = cone.axis.dot(diff2);
    if (AdD2 >= 0)
    {
        // P2 is on cone side of plane.
        p2Test = AdD2*AdD2 - cosSqr*(diff2.dot(diff2));
        if (p2Test >= 0)
        {
            // P2 is inside the cone.
            return true;
        }
        else
        {
            // P2 is outside the cone, but on cone side of plane.
            onConeSide |= 4;
        }
    }
    // else P2 is not on cone side of plane.

    // Test edge <P0,P1> = E0.
    if (onConeSide & 3)
    {
        AdE = AdD1 - AdD0;
        EdE = edge0.dot(edge0);
        c2 = AdE*AdE - cosSqr*EdE;
        if (c2 < 0)
        {
            EdD = edge0.dot(diff0);
            c1 = AdE*AdD0 - cosSqr*EdD;
            if (onConeSide & 1)
            {
                if (onConeSide & 2)
                {
                    // <P0,P1> fully on cone side of plane, c0 = p0Test.
                    if (0 <= c1 && c1 <= -c2 && c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
                else
                {
                    // P0 on cone side (dot(A,P0-V) >= 0),
                    // P1 on opposite side (dot(A,P1-V) <= 0)
                    // (dot(A,E0) <= 0), c0 = p0Test.
                    if (0 <= c1 && c2*AdD0 <= c1*AdE
                    &&  c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
            }
            else
            {
                // P1 on cone side (dot(A,P1-V) >= 0),
                // P0 on opposite side (dot(A,P0-V) <= 0)
                // (dot(A,E0) >= 0), c0 = p0Test (needs calculating).
                if (c1 <= -c2 && c2*AdD0 <= c1*AdE)
                {
                    p0Test = AdD0*AdD0 - cosSqr*(diff0.dot(diff0));
                    if (c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    // else <P0,P1> does not intersect cone half space.

    // Test edge <P0,P2> = E1.
    if (onConeSide & 5)
    {
        AdE = AdD2 - AdD0;
        EdE = edge1.dot(edge1);
        c2 = AdE*AdE - cosSqr*EdE;
        if (c2 < 0)
        {
            EdD = edge1.dot(diff0);
            c1 = AdE*AdD0 - cosSqr*EdD;
            if (onConeSide & 1)
            {
                if (onConeSide & 4)
                {
                    // <P0,P2> fully on cone side of plane, c0 = p0Test.
                    if (0.0 <= c1 && c1 <= -c2 && c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
                else
                {
                    // P0 on cone side (dot(A,P0-V) >= 0),
                    // P2 on opposite side (dot(A,P2-V) <= 0)
                    // (dot(A,E1) <= 0), c0 = p0Test.
                    if (0.0 <= c1 && c2*AdD0 <= c1*AdE
                    &&  c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
            }
            else
            {
                // P2 on cone side (dot(A,P2-V) >= 0),
                // P0 on opposite side (dot(A,P0-V) <= 0)
                // (dot(A,E1) >= 0), c0 = p0Test (needs calculating).
                if (c1 <= -c2 && c2*AdD0 <= c1*AdE)
                {
                    p0Test = AdD0*AdD0 - cosSqr*(diff0.dot(diff0));
                    if (c1*c1 >= p0Test*c2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    // else <P0,P2> does not intersect cone half space.

    // Test edge <P1,P2> = E1-E0 = E2.
    if (onConeSide & 6)
    {
        Vec3 kE2 = edge1 - edge0;
        AdE = AdD2 - AdD1;
        EdE = kE2.dot(kE2);
        c2 = AdE*AdE - cosSqr*EdE;
        if (c2 < 0.0)
        {
            EdD = kE2.dot(diff1);
            c1 = AdE*AdD1 - cosSqr*EdD;
            if (onConeSide & 2)
            {
                if (onConeSide & 4)
                {
                    // <P1,P2> fully on cone side of plane, c0 = p1Test.
                    if (0.0 <= c1 && c1 <= -c2 && c1*c1 >= p1Test*c2)
                    {
                        return true;
                    }
                }
                else
                {
                    // P1 on cone side (dot(A,P1-V) >= 0),
                    // P2 on opposite side (dot(A,P2-V) <= 0)
                    // (dot(A,E2) <= 0), c0 = p1Test.
                    if (0.0 <= c1 && c2*AdD1 <= c1*AdE
                    &&  c1*c1 >= p1Test*c2)
                    {
                        return true;
                    }
                }
            }
            else
            {
                // P2 on cone side (dot(A,P2-V) >= 0),
                // P1 on opposite side (dot(A,P1-V) <= 0)
                // (dot(A,E2) >= 0), c0 = p1Test (needs calculating).
                if (c1 <= -c2 && c2*AdD1 <= c1*AdE)
                {
                    p1Test = AdD1*AdD1 - cosSqr*(diff1.dot(diff1));
                    if (c1*c1 >= p1Test*c2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    // else <P1,P2> does not intersect cone half space.

    // Test triangle <P0,P1,P2>.  It is enough to handle only the case when
    // at least one pi is on the cone side of the plane.  In this case and
    // after the previous testing, if the triangle intersects the cone, the
    // set of intersection must contain the poInt of intersection between
    // the cone axis and the triangle.
    if (onConeSide > 0)
    {
        Vec3 N = edge0.cross(edge1);
        Real NdA = N.dot(cone.axis);
        Real NdD = N.dot(diff0);
        Vec3 U = NdD*cone.axis - NdA*diff0;
        Vec3 NcU = N.cross(U);

        Real NcUdE0 = NcU.dot(edge0), NcUdE1, NcUdE2, NdN;
        if (NdA >= 0)
        {
            if (NcUdE0 <= 0)
            {
                NcUdE1 = NcU.dot(edge1);
                if (NcUdE1 >= 0)
                {
                    NcUdE2 = NcUdE1 - NcUdE0;
                    NdN = N.lengthSqr();
                    if (NcUdE2 <= NdA*NdN)
                    {
                        return true;
                    }
                }
            }
        }
        else
        {
            if (NcUdE0 >= 0)
            {
                NcUdE1 = NcU.dot(edge1);
                if (NcUdE1 <= 0)
                {
                    NcUdE2 = NcUdE1 - NcUdE0;
                    NdN = N.lengthSqr();
                    if (NcUdE2 >= NdA*NdN)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

template<class Real>
bool Intersect_<Real>::test(const Cone& cone, const Plane& plane)
{
    Vec3 B = cone.axisLine()[1];
    Vec3 D = -cone.axis;

    //If P is parallel to C's axis, then there will be an intersection if the distance between
    //P and C's axis is less than or equal to the radius of the cone.
    Real pdotD = plane.normal.dot(D);
    if (Alge::isNearZero(pdotD))
        return Alge::abs(distance(plane, B)) <= cone.radius();

    //If P is perpendicular to C's axis, then there will be an intersection if the signed
    //distance (relative to d) from B to P is between 0 and h.
    Vec3 Ia;
    Real iaDist;
    if (!find(plane, Ray(B, D), Ia))
        if (!find(plane, Ray(B, -D), Ia))
            return false;
    iaDist = (Ia-B).dot(D);

    if (Alge::isNear(Alge::abs(pdotD), 1))
        return Alge::isInRange(iaDist, 0, cone.height);

    //If P is neither parallel nor perpendicular to C's axis, then there are two cases to consider:
    //a.    The signed distance (relative to d) from B to P is between 0 and h; in this
    //      case, there is definitely an intersection.
    //b.    P intersects the axis of C outside the apex or end cap of the cone, in which
    //      case there may or may not be an intersection, depending on the relative
    //      location of the point of intersection and the angle between the plane and the axis.
    if (Alge::isInRange(iaDist, 0, cone.height))
        return true;

    Vec3 W = D.crossUnit(plane.normal.crossUnit(D));
    Real sqr = Alge::sqr(iaDist / plane.normal.dot(W)) - Alge::sqr(iaDist);

    return sqr <= Alge::sqr(cone.radius());
}

template<class Real>
bool Intersect_<Real>::test(const Cone& cone, const Vec3& point)
{
    //Test if point is on vertex
    //Test if point is between cone bottom/top planes and within angular span of cone axis
    Vec3 vertex_point_dir = (point - cone.vertex).normalize();
    return  vertex_point_dir.isNearZero() ||
            (   Geom::side(cone.plane(0), point) == Plane::Side::neg &&
                Geom::side(cone.plane(1), point) == Plane::Side::neg &&
                Trig::acos(cone.axis.dot(vertex_point_dir)) <= cone.angle);
}

template<class Real>
bool Intersect_<Real>::test(const Cylinder& cylinder, const Plane& plane)
{
    // Compute extremes of signed distance dot(N,X)-d for points on the
    // cylinder.  These are
    //   min = (dot(N,C)-d) - r*sqrt(1-dot(N,W)^2) - (h/2)*|dot(N,W)|
    //   max = (dot(N,C)-d) + r*sqrt(1-dot(N,W)^2) + (h/2)*|dot(N,W)|
    Real dist = distance(plane, cylinder.center);
    Real absNdW = Alge::abs(plane.normal.dot(cylinder.axis));
    Real root = Alge::sqrt(Alge::abs(1 - absNdW*absNdW));
    Real term = cylinder.radius*root + 0.5*cylinder.height*absNdW;

    // Intersection occurs if and only if 0 is in the interval [min,max].
    return Alge::abs(dist) <= term;
}

template<class Real>
bool Intersect_<Real>::test(const Cylinder& cylinder, const Vec3& point)
{
    //Test if point is between cylinder bottom/top planes and within radius to cylinder axis
    return  Geom::side(cylinder.plane(0), point) == Plane::Side::neg &&
            Geom::side(cylinder.plane(1), point) == Plane::Side::neg &&
            distanceSqr(cylinder.axisLine(), point) <= Alge::sqr(cylinder.radius);
}

#define FINDMINMAX(x0,x1,x2,min,max)    \
  min = max = x0;                       \
  if(x1<min) min=x1;                    \
  if(x1>max) max=x1;                    \
  if(x2<min) min=x2;                    \
  if(x2>max) max=x2;

template<class Real>
bool Intersect_<Real>::BoxTriangleInt::
    planeBoxOverlap(const Vec3& normal, const Real d, const Vec3& maxbox)
{
    Vec3 vmin,vmax;

    if(normal.x>0)
    {
        vmin.x=-maxbox.x;
        vmax.x=maxbox.x;
    }
    else
    {
        vmin.x=maxbox.x;
        vmax.x=-maxbox.x;
    }

    if(normal.y>0)
    {
        vmin.y=-maxbox.y;
        vmax.y=maxbox.y;
    }
    else
    {
        vmin.y=maxbox.y;
        vmax.y=-maxbox.y;
    }

    if(normal.z>0)
    {
        vmin.z=-maxbox.z;
        vmax.z=maxbox.z;
    }
    else
    {
        vmin.z=maxbox.z;
        vmax.z=-maxbox.z;
    }

    if(normal.dot(vmin)+d > 0) return false;
    if(normal.dot(vmax)+d >= 0) return true;

    return false;
}


#define AXISTEST_X01(a, b, fa, fb)                          \
    p0 = a*v0.y - b*v0.z;                                   \
    p2 = a*v2.y - b*v2.z;                                   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;          \
    if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)                           \
    p0 = a*v0.y - b*v0.z;                                   \
    p1 = a*v1.y - b*v1.z;                                   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;          \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Y02(a, b, fa, fb)                          \
    p0 = -a*v0.x + b*v0.z;                                  \
    p2 = -a*v2.x + b*v2.z;                                  \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;          \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)                           \
    p0 = -a*v0.x + b*v0.z;                                  \
    p1 = -a*v1.x + b*v1.z;                                  \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;          \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Z12(a, b, fa, fb)                          \
    p1 = a*v1.x - b*v1.y;                                   \
    p2 = a*v2.x - b*v2.y;                                   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}  \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;          \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)                           \
    p0 = a*v0.x - b*v0.y;                                   \
    p1 = a*v1.x - b*v1.y;                                   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;          \
    if(min>rad || max<-rad) return false;

template<class Real>
bool Intersect_<Real>::test(const Box& box, const Triangle& tri)
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-direction) */
  /*       this gives 3x3=9 more tests */
   Vec3 v0,v1,v2;
   Real min,max,d,p0,p1,p2,rad,fex,fey,fez;
   Vec3 normal,e0,e1,e2;
   Vec3 boxcenter = box.getCenter();
   Vec3 boxhalfsize = (box.max-box.min) / 2;

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   v0 = tri.v0-boxcenter;
   v1 = tri.v1-boxcenter;
   v2 = tri.v2-boxcenter;

   /* compute triangle edges */
   e0 = v1-v0;      /* tri edge 0 */
   e1 = v2-v1;      /* tri edge 1 */
   e2 = v0-v2;      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = Alge::abs(e0.x);
   fey = Alge::abs(e0.y);
   fez = Alge::abs(e0.z);
   AXISTEST_X01(e0.z, e0.y, fez, fey);
   AXISTEST_Y02(e0.z, e0.x, fez, fex);
   AXISTEST_Z12(e0.y, e0.x, fey, fex);

   fex = Alge::abs(e1.x);
   fey = Alge::abs(e1.y);
   fez = Alge::abs(e1.z);
   AXISTEST_X01(e1.z, e1.y, fez, fey);
   AXISTEST_Y02(e1.z, e1.x, fez, fex);
   AXISTEST_Z0(e1.y, e1.x, fey, fex);

   fex = Alge::abs(e2.x);
   fey = Alge::abs(e2.y);
   fez = Alge::abs(e2.z);
   AXISTEST_X2(e2.z, e2.y, fez, fey);
   AXISTEST_Y1(e2.z, e2.x, fez, fex);
   AXISTEST_Z12(e2.y, e2.x, fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0.x,v1.x,v2.x,min,max);
   if(min>boxhalfsize.x || max<-boxhalfsize.x) return false;

   /* test in Y-direction */
   FINDMINMAX(v0.y,v1.y,v2.y,min,max);
   if(min>boxhalfsize.y || max<-boxhalfsize.y) return false;

   /* test in Z-direction */
   FINDMINMAX(v0.z,v1.z,v2.z,min,max);
   if(min>boxhalfsize.z || max<-boxhalfsize.z) return false;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   normal = e0.cross(e1);
   d=-normal.dot(v0);  /* plane eq: normal.x+d=0 */
   if(!BoxTriangleInt::planeBoxOverlap(normal,d,boxhalfsize)) return false;

   return true;   /* box and triangle overlaps */
}

template<class Real>
bool Intersect_<Real>::test(const Box& box, const Plane& plane)
{
    // There is an intersection if a vertex is on the opposite side of another
    typename Plane::Side side = Geom::side(plane, box.vertex(0));
    for (int i = 1; i < box.vertexCount; ++i)
        if (Geom::side(plane, box.vertex(i)) != side)
            return true;
    return false;
}

template<class Real>
bool Intersect_<Real>::test(const Box& box, const Line& line)
{
    Real t;
    if (!pluecker(box, Ray(line[0], line.diff()), t))
        return false;
    return t <= 1;
}

template<class Real>
bool Intersect_<Real>::test(const Box& box, const Ray& ray)
{
    return pluecker(box, ray);
}

template<class Real>
bool Intersect_<Real>::pluecker(const Box& box, const Ray& ray, option<Real&> t)
{
    const Vec3& ro = ray.origin;
    const Vec3& rd = ray.dir;
    const Vec3& b0 = box.min;
    const Vec3& b1 = box.max;

    if(rd.x < 0) 
    {
        if(rd.y < 0) 
        {
            if(rd.z < 0)
            {
                // case MMM: side(R,HD) < 0 or side(R,FB) > 0 or side(R,EF) > 0 or side(R,DC) < 0 or side(R,CB) < 0 or side(R,HE) > 0 to miss

                if ((ro.x < b0.x) || (ro.y < b0.y) || (ro.z < b0.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * ya - rd.y * xb < 0) ||
                    (rd.x * yb - rd.y * xa > 0) ||
                    (rd.x * zb - rd.z * xa > 0) ||
                    (rd.x * za - rd.z * xb < 0) ||
                    (rd.y * za - rd.z * yb < 0) ||
                    (rd.y * zb - rd.z * ya > 0))
                    return false;

                if (t)
                {
                    // compute the intersection distance
                    t = xb / rd.x;
                    Real t1 = yb / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = zb / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
            else
            {
                // case MMP: side(R,HD) < 0 or side(R,FB) > 0 or side(R,HG) > 0 or side(R,AB) < 0 or side(R,DA) < 0 or side(R,GF) > 0 to miss

                if ((ro.x < b0.x) || (ro.y < b0.y) || (ro.z > b1.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * ya - rd.y * xb < 0) ||
                    (rd.x * yb - rd.y * xa > 0) ||
                    (rd.x * zb - rd.z * xb > 0) ||
                    (rd.x * za - rd.z * xa < 0) ||
                    (rd.y * za - rd.z * ya < 0) ||
                    (rd.y * zb - rd.z * yb > 0))
                    return false;

                if (t)
                {
                    t = xb / rd.x;
                    Real t1 = yb / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = za / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
        } 
        else 
        {
            if(rd.z < 0)
            {
                // case MPM: side(R,EA) < 0 or side(R,GC) > 0 or side(R,EF) > 0 or side(R,DC) < 0 or side(R,GF) < 0 or side(R,DA) > 0 to miss

                if ((ro.x < b0.x) || (ro.y > b1.y) || (ro.z < b0.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * ya - rd.y * xa < 0) ||
                    (rd.x * yb - rd.y * xb > 0) ||
                    (rd.x * zb - rd.z * xa > 0) ||
                    (rd.x * za - rd.z * xb < 0) ||
                    (rd.y * zb - rd.z * yb < 0) ||
                    (rd.y * za - rd.z * ya > 0))
                    return false;

                if (t)
                {
                    t = xb / rd.x;
                    Real t1 = ya / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = zb / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
            else
            {
                // case MPP: side(R,EA) < 0 or side(R,GC) > 0 or side(R,HG) > 0 or side(R,AB) < 0 or side(R,HE) < 0 or side(R,CB) > 0 to miss

                if ((ro.x < b0.x) || (ro.y > b1.y) || (ro.z > b1.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * ya - rd.y * xa < 0) ||
                    (rd.x * yb - rd.y * xb > 0) ||
                    (rd.x * zb - rd.z * xb > 0) ||
                    (rd.x * za - rd.z * xa < 0) ||
                    (rd.y * zb - rd.z * ya < 0) ||
                    (rd.y * za - rd.z * yb > 0))
                    return false;

                if (t)
                {
                    t = xb / rd.x;
                    Real t1 = ya / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = za / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
        }
    }
    else 
    {
        if(rd.y < 0) 
        {
            if(rd.z < 0)
            {
                // case PMM: side(R,GC) < 0 or side(R,EA) > 0 or side(R,AB) > 0 or side(R,HG) < 0 or side(R,CB) < 0 or side(R,HE) > 0 to miss

                if ((ro.x > b1.x) || (ro.y < b0.y) || (ro.z < b0.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * yb - rd.y * xb < 0) ||
                    (rd.x * ya - rd.y * xa > 0) ||
                    (rd.x * za - rd.z * xa > 0) ||
                    (rd.x * zb - rd.z * xb < 0) ||
                    (rd.y * za - rd.z * yb < 0) ||
                    (rd.y * zb - rd.z * ya > 0))
                    return false;

                if (t)
                {
                    t = xa / rd.x;
                    Real t1 = yb / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = zb / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
            else
            {
                // case PMP: side(R,GC) < 0 or side(R,EA) > 0 or side(R,DC) > 0 or side(R,EF) < 0 or side(R,DA) < 0 or side(R,GF) > 0 to miss

                if ((ro.x > b1.x) || (ro.y < b0.y) || (ro.z > b1.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * yb - rd.y * xb < 0) ||
                    (rd.x * ya - rd.y * xa > 0) ||
                    (rd.x * za - rd.z * xb > 0) ||
                    (rd.x * zb - rd.z * xa < 0) ||
                    (rd.y * za - rd.z * ya < 0) ||
                    (rd.y * zb - rd.z * yb > 0))
                    return false;

                if (t)
                {
                    t = xa / rd.x;
                    Real t1 = yb / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = za / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
        }
        else 
        {
            if(rd.z < 0)
            {
                // case PPM: side(R,FB) < 0 or side(R,HD) > 0 or side(R,AB) > 0 or side(R,HG) < 0 or side(R,GF) < 0 or side(R,DA) > 0 to miss

                if ((ro.x > b1.x) || (ro.y > b1.y) || (ro.z < b0.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * yb - rd.y * xa < 0) ||
                    (rd.x * ya - rd.y * xb > 0) ||
                    (rd.x * za - rd.z * xa > 0) ||
                    (rd.x * zb - rd.z * xb < 0) ||
                    (rd.y * zb - rd.z * yb < 0) ||
                    (rd.y * za - rd.z * ya > 0))
                    return false;

                if (t)
                {
                    t = xa / rd.x;
                    Real t1 = ya / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = zb / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
            else
            {
                // case PPP: side(R,FB) < 0 or side(R,HD) > 0 or side(R,DC) > 0 or side(R,EF) < 0 or side(R,HE) < 0 or side(R,CB) > 0 to miss

                if ((ro.x > b1.x) || (ro.y > b1.y) || (ro.z > b1.z))
                    return false;

                Real xa = b0.x - ro.x; 
                Real ya = b0.y - ro.y; 
                Real za = b0.z - ro.z; 
                Real xb = b1.x - ro.x;
                Real yb = b1.y - ro.y;
                Real zb = b1.z - ro.z;

                if( (rd.x * yb - rd.y * xa < 0) ||
                    (rd.x * ya - rd.y * xb > 0) ||
                    (rd.x * za - rd.z * xb > 0) ||
                    (rd.x * zb - rd.z * xa < 0) ||
                    (rd.y * zb - rd.z * ya < 0) ||
                    (rd.y * za - rd.z * yb > 0))
                    return false;

                if (t)
                {
                    t = xa / rd.x;
                    Real t1 = ya / rd.y;
                    if(t1 > *t)
                        t = t1;
                    Real t2 = za / rd.z;
                    if(t2 > *t)
                        t = t2;
                }

                return true;
            }
        }
    }
}

template<class Real>
bool Intersect_<Real>::test(const Sphere& sphere, const Line& line)
{
    Real lineExtent;
    Vec3 lineDir = line.dir(lineExtent);
    lineExtent *= 0.5;

    Vec3 diff = line.center() - sphere.center;
    Real a0 = diff.dot(diff) - sphere.radius*sphere.radius;
    Real a1 = lineDir.dot(diff);
    Real discr = a1*a1 - a0;
    if (discr < 0)
        return false;

    Real tmp0 = lineExtent*lineExtent + a0;
    Real tmp1 = 2*a1*lineExtent;
    Real qm = tmp0 - tmp1;
    Real qp = tmp0 + tmp1;
    if (qm*qp <= 0)
        return true;

    return qm > 0 && Alge::abs(a1) < lineExtent;
}

template<class Real>
bool Intersect_<Real>::test(const Sphere& sphere, const Ray& ray)
{
    Vec3 diff = ray.origin - sphere.center;
    Real a0 = diff.dot(diff) - sphere.radius*sphere.radius;
    if (a0 <= 0)
        // P is inside the sphere
        return true;
    // else: P is outside the sphere

    Real a1 = ray.dir.dot(diff);
    if (a1 >= 0)
        return false;

    // Quadratic has a real root if discriminant is nonnegative.
    return a1*a1 >= a0;
}

template<class Real>
bool Intersect_<Real>::test(const Triangle& tri1, const Triangle& tri2)
{
    // get edge vectors for triangle0
    Vec3 akE0[3] =
    {
        tri1.v1 - tri1.v0,
        tri1.v2 - tri1.v1,
        tri1.v0 - tri1.v2
    };

    // get normal vector of triangle0
    Vec3 kN0 = akE0[0].crossUnit(akE0[1]);

    // project triangle1 onto normal line of triangle0, test for separation
    Real n0dT0V0 = kN0.dot(tri1.v0);
    Real min1, max1;
    TriangleInt::projectOntoAxis(tri2,kN0,min1,max1);
    if (n0dT0V0 < min1 || n0dT0V0 > max1)
    {
        return false;
    }

    // get edge vectors for triangle1
    Vec3 akE1[3] =
    {
        tri2.v1 - tri2.v0,
        tri2.v2 - tri2.v1,
        tri2.v0 - tri2.v2
    };

    // get normal vector of triangle1
    Vec3 kN1 = akE1[0].crossUnit(akE1[1]);

    Vec3 kDir;
    Real min0, max0;
    int i0, i1;

    Vec3 kN0xN1 = kN0.crossUnit(kN1);
    if (!Alge::isNearZero(kN0xN1.dot(kN0xN1)))
    {
        // triangles are not parallel

        // Project triangle0 onto normal line of triangle1, test for
        // separation.
        Real n1dT1V0 = kN1.dot(tri2.v0);
        TriangleInt::projectOntoAxis(tri1,kN1,min0,max0);
        if (n1dT1V0 < min0 || n1dT1V0 > max0)
        {
            return false;
        }

        // directions E0[i0]xE1[i1]
        for (i1 = 0; i1 < 3; ++i1)
        {
            for (i0 = 0; i0 < 3; ++i0)
            {
                kDir = akE0[i0].crossUnit(akE1[i1]);
                TriangleInt::projectOntoAxis(tri1,kDir,min0,max0);
                TriangleInt::projectOntoAxis(tri2,kDir,min1,max1);
                if (max0 < min1 || max1 < min0)
                {
                    return false;
                }
            }
        }
    }
    else  // triangles are parallel (and, in fact, coplanar)
    {
        // directions N0xE0[i0]
        for (i0 = 0; i0 < 3; ++i0)
        {
            kDir = kN0.crossUnit(akE0[i0]);
            TriangleInt::projectOntoAxis(tri1,kDir,min0,max0);
            TriangleInt::projectOntoAxis(tri2,kDir,min1,max1);
            if (max0 < min1 || max1 < min0)
            {
                return false;
            }
        }

        // directions N1xE1[i1]
        for (i1 = 0; i1 < 3; ++i1)
        {
            kDir = kN1.crossUnit(akE1[i1]);
            TriangleInt::projectOntoAxis(tri1,kDir,min0,max0);
            TriangleInt::projectOntoAxis(tri2,kDir,min1,max1);
            if (max0 < min1 || max1 < min0)
            {
                return false;
            }
        }
    }

    return true;
}

template<class Real>
bool Intersect_<Real>::test(const Triangle& tri, const Plane& plane)
{
    // Compute the signed distances from the vertices to the plane.
    Real zero = 0;
    Real afSD[3];
    for (int i = 0; i < 3; ++i)
    {
        afSD[i] = distance(plane, tri[i]);
        if (Alge::isNearZero(afSD[i]))
        {
            afSD[i] = zero;
        }
    };

    // The triangle intersects the plane if not all vertices are on the
    // positive side of the plane and not all vertices are on the negative
    // side of the plane.
    return !(afSD[0] > zero && afSD[1] > zero && afSD[2] > zero)
        && !(afSD[0] < zero && afSD[1] < zero && afSD[2] < zero);
}

template<class Real>
bool Intersect_<Real>::test(const Triangle& tri, const Line& line)
{
    Real lineExtent;
    Vec3 lineDir = line.dir(lineExtent);
    lineExtent *= 0.5;

    // Compute the offset origin, edges, and normal.
    Vec3 diff = line.center() - tri[0];
    Vec3 edge1 = tri[1] - tri[0];
    Vec3 edge2 = tri[2] - tri[0];
    Vec3 normal = edge1.cross(edge2);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = segment direction,
    // E1 = edge1, E2 = edge2, N = cross(E1,E2)) by
    //   |dot(D,N)|*b1 = sign(dot(D,N))*dot(D,cross(Q,E2))
    //   |dot(D,N)|*b2 = sign(dot(D,N))*dot(D,cross(E1,Q))
    //   |dot(D,N)|*t = -sign(dot(D,N))*dot(Q,N)
    Real DdN = lineDir.dot(normal);
    Real sign;
    if (DdN > Real_::zeroTol)
    {
        sign = 1;
    }
    else if (DdN < -Real_::zeroTol)
    {
        sign = -1;
        DdN = -DdN;
    }
    else
    {
        // Segment and triangle are parallel, call it a "no intersection"
        // even if the segment does intersect.
        return false;
    }

    Real DdQxE2 = sign*lineDir.dot(diff.cross(edge2));
    if (DdQxE2 >= 0)
    {
        Real DdE1xQ = sign*lineDir.dot(edge1.cross(diff));
        if (DdE1xQ >= 0)
        {
            if (DdQxE2 + DdE1xQ <= DdN)
            {
                // Line intersects triangle, check if segment does.
                Real QdN = -sign*diff.dot(normal);
                Real extDdN = lineExtent*DdN;
                if (-extDdN <= QdN && QdN <= extDdN)
                {
                    // Segment intersects triangle.
                    return true;
                }
                // else: |t| > extent, no intersection
            }
            // else: b1+b2 > 1, no intersection
        }
        // else: b2 < 0, no intersection
    }
    // else: b1 < 0, no intersection

    return false;
}

template<class Real>
bool Intersect_<Real>::test(const Triangle& tri, const Ray& ray)
{
    // Compute the offset origin, edges, and normal.
    Vec3 diff = ray.origin - tri[0];
    Vec3 edge1 = tri[1] - tri[0];
    Vec3 edge2 = tri[2] - tri[0];
    Vec3 normal = edge1.cross(edge2);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
    // E1 = kEdge1, E2 = kEdge2, N = cross(E1,E2)) by
    //   |dot(D,N)|*b1 = sign(dot(D,N))*dot(D,cross(Q,E2))
    //   |dot(D,N)|*b2 = sign(dot(D,N))*dot(D,cross(E1,Q))
    //   |dot(D,N)|*t = -sign(dot(D,N))*dot(Q,N)
    Real DdN = ray.dir.dot(normal);
    Real sign;
    if (DdN > Real_::zeroTol)
    {
        sign = 1;
    }
    else if (DdN < -Real_::zeroTol)
    {
        sign = -1;
        DdN = -DdN;
    }
    else
    {
        // Ray and triangle are parallel, call it a "no intersection"
        // even if the ray does intersect.
        return false;
    }

    Real DdQxE2 = sign*ray.dir.dot(diff.cross(edge2));
    if (DdQxE2 >= 0)
    {
        Real DdE1xQ = sign*ray.dir.dot(edge1.cross(diff));
        if (DdE1xQ >= 0)
        {
            if (DdQxE2 + DdE1xQ <= DdN)
            {
                // Line intersects triangle, check if ray does.
                Real QdN = -sign*diff.dot(normal);
                if (QdN >= 0)
                {
                    // Ray intersects triangle.
                    return true;
                }
                // else: t < 0, no intersection
            }
            // else: b1+b2 > 1, no intersection
        }
        // else: b2 < 0, no intersection
    }
    // else: b1 < 0, no intersection

    return false;
}

template<class Real>
Real Intersect_<Real>::distance(const Frustum& frustum, const Box& box, option<Vec3&> frustumPoint, option<Vec3&> boxPoint)
{
    Real dist_min = -Real_::max;
    int plane_index_min = -1;
    int box_index_min = -1;

    bool inside = true;

    for (int i = 0; i < frustum.planeCount; ++i)
    {
        //Test all box corners against the plane
        //If any corners are inside, then we don't care about the minimum distance found
        //If all the corners are outside, then the minimum distance is valid
        bool in = false;

        Real inner_dist_min = -Real_::max;
        int inner_box_index_min = -1;
    
        for (int j = 0; j < box.vertexCount; ++j)
        {
            Real dist = distance(frustum.plane(i), box.vertex(j));
            if (dist >= 0)
            {
                in = true;
                break;
            }
            else
            {
                if (dist > inner_dist_min)
                {
                    inner_dist_min = dist;
                    inner_box_index_min = j;
                }
            }
        }

        if (!in)
        {
            //Box corners are all outside plane
            //The minimum distance is valid
            inside = false;

            if (inner_dist_min > dist_min)
            {
                dist_min = inner_dist_min;
                box_index_min = inner_box_index_min;
                plane_index_min = i;
            }
        }
    }

    if (inside)
    {
        //Box is inside frustum
        //Just return the box's center
        Vec3 boxCenter = box.getCenter();
        if (boxPoint) boxPoint = boxCenter;
        if (frustumPoint) frustumPoint = boxCenter;
        return 0;
    }

    dist_min = -dist_min;
    if (boxPoint) boxPoint = box.vertex(box_index_min);
    if (frustumPoint) frustumPoint = box.vertex(box_index_min) + frustum.plane(plane_index_min).normal*dist_min;
    return dist_min;
}


template<class Real>
Real Intersect_<Real>::distance(const Frustum& frustum, const Sphere& sphere, option<Vec3&> frustumPoint, option<Vec3&> spherePoint)
{
    bool outside = false;

    Real dist_min = -Real_::max;
    int index_min = -1;

    Real dist_in_min = -Real_::max;
    int index_in_min = -1;

    for (int i = 0; i < frustum.planeCount; ++i)
    {
        Real dist = distance(frustum.plane(i), sphere.center);

        if (dist < 0)
        {
            if (dist >= -sphere.radius)
            {
                //Partially inside frustum
                if (dist > dist_in_min)
                {
                    dist_in_min = dist;
                    index_in_min = i;
                }
            }
            else 
            {
                //Outside of frustum
                outside = true;
                if (dist > dist_min)
                {
                    dist_min = dist;
                    index_min = i;
                }
            }
        }
    }

    if (!outside)
    {
        //Inside frustum
        if (index_in_min == -1)
        {
            //Sphere center inside frustum, return sphere center
            if (spherePoint) spherePoint = sphere.center;
            if (frustumPoint) frustumPoint = sphere.center;
        }
        else
        {
            //Partially inside frustum, return point on frustum
            dist_in_min = -dist_in_min;
            Vec3 point = sphere.center + frustum.plane(index_in_min).normal*dist_in_min;
            if (spherePoint) spherePoint = point;
            if (frustumPoint) frustumPoint = point;
        }
        return 0;
    }

    dist_min = -dist_min;
    if (spherePoint) spherePoint = sphere.center + frustum.plane(index_min).normal*sphere.radius;
    if (frustumPoint) frustumPoint = sphere.center + frustum.plane(index_min).normal*dist_min;
    return dist_min;
}

template<class Real>
Real Intersect_<Real>::distance(const Frustum& frustum, const Vec3& point, option<Vec3&> frustumPoint)
{
    bool outside = false;
    Real dist_min = -Real_::max;
    int index_min = -1;

    for (int i = 0; i < frustum.planeCount; ++i)
    {
        Real dist = distance(frustum.plane(i), point);

        if (dist < 0)
        {
            //outside frustum
            outside = true;
            if (dist > dist_min)
            {
                dist_min = dist;
                index_min = i;
            }
        }
    }

    if (!outside)
    {
        //Inside frustum
        if (frustumPoint) frustumPoint = point;
        return 0;
    }

    dist_min = -dist_min;
    if (frustumPoint) frustumPoint = point + frustum.plane(index_min).normal*dist_min;
    return dist_min;
}

template<class Real>
Real Intersect_<Real>::distance(const OrientBox& box, const Line& line, option<Vec3&> boxPoint, option<Vec3&> linePoint)
{
    return Alge::sqrt(distanceSqr(box, line, boxPoint, linePoint));
}

template<class Real>
Real Intersect_<Real>::distance(const OrientBox& box, const Ray& ray, option<Vec3&> boxPoint, option<Vec3&> rayPoint)
{
    return Alge::sqrt(distanceSqr(box, ray, boxPoint, rayPoint));
}

template<class Real>
Real Intersect_<Real>::distance(const OrientBox& box, const Vec3& point, option<Vec3&> boxPoint)
{
    return Alge::sqrt(distanceSqr(box, point, boxPoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Capsule& capsule, const Line& line, option<Vec3&> capsulePoint_, option<Vec3&> linePoint_)
{
    Vec3 capsulePoint;
    Vec3 linePoint;
    Real dist = distance(capsule.line, line, capsulePoint, linePoint);
    if (capsulePoint_) capsulePoint_ = (dist > capsule.radius) ? capsulePoint + (linePoint - capsulePoint)*(capsule.radius / dist) : linePoint;
    if (linePoint_) linePoint_ = linePoint;
    return Alge::max(0, dist - capsule.radius);
}

template<class Real>
Real Intersect_<Real>::distance(const Capsule& capsule, const Ray& ray, option<Vec3&> capsulePoint_, option<Vec3&> rayPoint_)
{
    Vec3 capsulePoint;
    Vec3 rayPoint;
    Real dist = distance(capsule.line, ray, capsulePoint, rayPoint);
    if (capsulePoint_) capsulePoint_ = (dist > capsule.radius) ? capsulePoint + (rayPoint - capsulePoint)*(capsule.radius / dist) : rayPoint;
    if (rayPoint_) rayPoint_ = rayPoint;
    return Alge::max(0, dist - capsule.radius);
}

template<class Real>
Real Intersect_<Real>::distance(const Capsule& capsule, const Vec3& point, option<Vec3&> capsulePoint_)
{
    Vec3 capsulePoint;
    Real dist = distance(capsule.line, point, capsulePoint);
    if (capsulePoint_) capsulePoint_ = (dist > capsule.radius) ? capsulePoint + (point - capsulePoint)*(capsule.radius / dist) : point;
    return Alge::max(0, dist - capsule.radius);
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box1, const Box& box2, option<Vec3&> boxPoint1, option<Vec3&> boxPoint2)
{
    return Alge::sqrt(distanceSqr(box1, box2, boxPoint1, boxPoint2));
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Sphere& sphere, option<Vec3&> boxPoint, option<Vec3&> spherePoint)
{
    //Clamp the sphere center in the box
    Vec3 clamped = sphere.center.clamp(box.min, box.max);
    Vec3 sphereToClamped = (clamped - sphere.center);
    Real dist_sqr = sphereToClamped.lengthSqr();

    if (dist_sqr <= Alge::sqr(sphere.radius))
    {
        //Sphere is inside box
        if (boxPoint) boxPoint = clamped;
        if (spherePoint) spherePoint = clamped;
        return 0;
    }

    Real dist = Alge::sqrt(dist_sqr);
    if (boxPoint) boxPoint = clamped;
    if (spherePoint) spherePoint = sphere.center + sphereToClamped*(sphere.radius / dist);
    return dist - sphere.radius;
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Triangle& tri, option<Vec3&> boxPoint, option<Vec3&> triPoint)
{
    //This is just a quick appoximation of distance
    Vec3 point;
    Real dist = distance(tri, box.getCenter(), point);

    if (triPoint) triPoint = point;

    if (test(box, point))
    {
        //Point is inside the box, no distance
        if (boxPoint) boxPoint = point;
        return 0;
    }

    //Clamp the point in the box
    Vec3 clamped = point.clamp(box.min, box.max);
    if (boxPoint) boxPoint = clamped;
    Real len = (point-clamped).length();
    return (dist < 0) ? -len : len;
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Plane& plane, option<Vec3&> boxPoint, option<Vec3&> planePoint)
{
    //Loop through all the box vertices and get the minimum distance to the plane
    Real dist_min = distance(plane, box.vertex(0));
    Real dist_min_abs = Alge::abs(dist_min);
    int index_min = 0;

    for (int i = 1; i < box.vertexCount; ++i)
    {
        Real dist = distance(plane, box.vertex(i));
        Real dist_abs = Alge::abs(dist);
        if (dist_abs < dist_min_abs)
        {
            dist_min = dist;
            dist_min_abs = dist_abs;
            index_min = i;
        }
    }

    Vec3 minPlanePoint;
    distance(plane, box.vertex(index_min), minPlanePoint);

    if (planePoint) planePoint = minPlanePoint;

    if (test(box, plane))
    {
        //Point is inside the box, no distance
        if (boxPoint) boxPoint = minPlanePoint;
        return 0;
    }

    if (boxPoint) boxPoint = box.vertex(index_min);
    return dist_min;
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Line& line, option<Vec3&> boxPoint, option<Vec3&> linePoint)
{
    return Alge::sqrt(distanceSqr(box, line, boxPoint, linePoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Ray& ray, option<Vec3&> boxPoint, option<Vec3&> rayPoint)
{
    return Alge::sqrt(distanceSqr(box, ray, boxPoint, rayPoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Box& box, const Vec3& point, option<Vec3&> boxPoint)
{
    return Alge::sqrt(distanceSqr(box, point, boxPoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere1, const Sphere& sphere2, option<Vec3&> spherePoint1, option<Vec3&> spherePoint2)
{
    //For the points, just return the closest point on each sphere to the other's origin
    if (spherePoint1) distance(sphere1, sphere2.center, spherePoint1);
    if (spherePoint2) distance(sphere2, sphere1.center, spherePoint2);
    if (test(sphere1, sphere2)) return 0; //Sphere is inside the other sphere, no distance
    return (sphere2.center - sphere1.center).length() - sphere1.radius - sphere2.radius;
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere, const Triangle& tri, option<Vec3&> spherePoint, option<Vec3&> triPoint_)
{
    Vec3 triPoint;
    Real dist_sqr = distanceSqr(tri, sphere.center, triPoint);

    if (triPoint_) triPoint_ = triPoint;

    if (dist_sqr <= Alge::sqr(sphere.radius))
    {
        //Point is inside the sphere, no distance
        if (spherePoint) spherePoint = triPoint;
        return 0;
    }

    Real dist = Alge::sqrt(dist_sqr);
    if (spherePoint) spherePoint = sphere.center + (triPoint - sphere.center)*(sphere.radius / dist);
    dist -= sphere.radius;
    return (Plane::Side::pos == Geom::side(tri, sphere.center)) ? dist : -dist;
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere, const Plane& plane, option<Vec3&> spherePoint, option<Vec3&> planePoint_)
{
    Vec3 planePoint;
    Real sphereOriginToPlaneDist = distance(plane, sphere.center, planePoint);

    if (planePoint_) planePoint_ = planePoint;

    if (Alge::abs(sphereOriginToPlaneDist) <= sphere.radius)
    {
        //Point is inside the sphere, no distance
        if (spherePoint) spherePoint = planePoint; 
        return 0;
    }

    if (spherePoint) spherePoint = sphere.center + plane.normal * ((sphereOriginToPlaneDist < 0) ? sphere.radius : -sphere.radius);
    return sphereOriginToPlaneDist + ((sphereOriginToPlaneDist < 0) ? sphere.radius : -sphere.radius);
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere, const Line& line, option<Vec3&> spherePoint, option<Vec3&> linePoint_)
{
    Vec3 linePoint;
    Real dist_sqr = distanceSqr(line, sphere.center, linePoint);

    if (linePoint_) linePoint_ = linePoint;

    if (dist_sqr <= Alge::sqr(sphere.radius))
    {
        //Point is inside the sphere, no distance
        if (spherePoint) spherePoint = linePoint;
        return 0;
    }

    Real dist = Alge::sqrt(dist_sqr);
    if (spherePoint) spherePoint = sphere.center + (linePoint - sphere.center)*(sphere.radius / dist);
    return dist - sphere.radius;
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere, const Ray& ray, option<Vec3&> spherePoint, option<Vec3&> rayPoint_)
{
    Vec3 rayPoint;
    Real dist_sqr = distanceSqr(ray, sphere.center, rayPoint);

    if (rayPoint_) rayPoint_ = rayPoint;

    if (dist_sqr <= Alge::sqr(sphere.radius))
    {
        //Point is inside the sphere, no distance
        if (spherePoint) spherePoint = rayPoint;
        return 0;
    }

    Real dist = Alge::sqrt(dist_sqr);
    if (spherePoint) spherePoint = sphere.center + (rayPoint - sphere.center)*(sphere.radius / dist);
    return dist - sphere.radius;
}

template<class Real>
Real Intersect_<Real>::distance(const Sphere& sphere, const Vec3& point, option<Vec3&> spherePoint)
{
    Real dist_sqr = distanceSqr(point, sphere.center);

    if (dist_sqr <= Alge::sqr(sphere.radius))
    {
        //Point is inside the sphere, no distance
        if (spherePoint) spherePoint = point;
        return 0;
    }

    Real dist = Alge::sqrt(dist_sqr);
    if (spherePoint) spherePoint = sphere.center + (point - sphere.center)*(sphere.radius / dist);
    return dist - sphere.radius;
}

template<class Real>
Real Intersect_<Real>::distance(const Triangle& tri1, const Triangle& tri2, option<Vec3&> triPoint1, option<Vec3&> triPoint2)
{
    // Compare edges of tri1 to the interior of tri2.
    Real dist = Real_::max;
    Vec3 closest;

    for (int i = 0; i < tri1.edgeCount; ++i)
    {
        Vec3 triPoint, linePoint;
        Real distTmp = Alge::abs(distance(tri2, tri1.edge(i), triPoint, linePoint));

        if (distTmp < dist)
        {
            dist = distTmp;
            closest = triPoint;

            if (triPoint1) triPoint1 = linePoint;
            if (triPoint2) triPoint2 = closest;
            if (dist <= Real_::zeroTol) return 0;
        }
    }

    // Compare edges of tri2 to the interior of tri1.
    for (int i = 0; i < tri2.edgeCount; ++i)
    {
        Vec3 triPoint, linePoint;
        Real distTmp = Alge::abs(distance(tri1, tri2.edge(i), triPoint, linePoint));

        if (distTmp < dist)
        {
            dist = distTmp;
            closest = linePoint;

            if (triPoint1) triPoint1 = triPoint;
            if (triPoint2) triPoint2 = closest;
            if (dist <= Real_::zeroTol) return 0;
        }
    }

    return Geom::side(tri1, closest) == Plane::Side::pos ? dist : -dist;
}

template<class Real>
Real Intersect_<Real>::distance(const Triangle& tri, const Line& line, option<Vec3&> triPoint, option<Vec3&> linePoint_)
{
    //Convert line to ray and get distance
    Real len;
    Vec3 dir = line.dir(len);
    Vec3 linePoint;

    Real dist = distance(tri, Ray(line[0], dir), triPoint, linePoint);

    if (dir.dot(linePoint - line[0]) > len)
    {
        //Closest point is beyond line. Get distance from line end point
        if (linePoint_) linePoint_ = line[1];
        return distance(tri, line[1], triPoint);
    }

    if (linePoint_) linePoint_ = linePoint;

    return dist;
}

template<class Real>
Real Intersect_<Real>::distance(const Triangle& tri, const Ray& ray, option<Vec3&> triPoint, option<Vec3&> rayPoint_)
{
    // Test if line intersects triangle.  If so, the squared distance is zero.
    Real lineParam = 0;
    Vec3 edge0 = tri[1] - tri[0];
    Vec3 edge1 = tri[2] - tri[0];
    Vec3 normal = edge0.crossUnit(edge1);
    Real NdD = normal.dot(ray.dir);
    if (Alge::abs(NdD) > Real_::zeroTol)
    {
        // The line and triangle are not parallel, so the line intersects
        // the plane of the triangle.
        Vec3 diff = ray.origin - tri[0];
        Vec3 U, V;
        tie(U,V) = ray.dir.orthonormalBasis();
        Real UdE0 = U.dot(edge0);
        Real UdE1 = U.dot(edge1);
        Real UdDiff = U.dot(diff);
        Real VdE0 = V.dot(edge0);
        Real VdE1 = V.dot(edge1);
        Real VdDiff = V.dot(diff);
        Real invDet = 1/(UdE0*VdE1 - UdE1*VdE0);

        // Barycentric coordinates for the point of intersection.
        Real b1 = (VdE1*UdDiff - UdE1*VdDiff)*invDet;
        Real b2 = (UdE0*VdDiff - VdE0*UdDiff)*invDet;
        Real b0 = 1 - b1 - b2;

        if (b0 >= 0 && b1 >= 0 && b2 >= 0)
        {
            // Line parameter for the point of intersection.
            Real DdE0 = ray.dir.dot(edge0);
            Real DdE1 = ray.dir.dot(edge1);
            Real DdDiff = ray.dir.dot(diff);
            lineParam = b1*DdE0 + b2*DdE1 - DdDiff;

            if (lineParam < 0)
            {
                //Closest point is behind ray. Get distance from ray origin
                if (rayPoint_) rayPoint_ = ray.origin;
                return distance(tri, ray.origin, triPoint);
            }

            // The intersection point is inside or on the triangle.
            if (rayPoint_) rayPoint_ = ray.origin + lineParam*ray.dir;
            if (triPoint) triPoint = tri[0] + b1*edge0 + b2*edge1;
            return 0;
        }
    }

    // Either (1) the line is not parallel to the triangle and the point of
    // intersection of the line and the plane of the triangle is outside the
    // triangle or (2) the line and triangle are parallel.  Regardless, the
    // closest point on the triangle is on an edge of the triangle.  Compare
    // the line to all three edges of the triangle.
    Real sqrDist = Real_::max;
    Vec3 closest;
    for (int i = 0; i < tri.edgeCount; ++i)
    {
        Vec3 linePoint, rayPoint;
        Real sqrDistTmp = distanceSqr(tri.edge(i), ray, linePoint, rayPoint);

        if (sqrDistTmp < sqrDist)
        {
            sqrDist = sqrDistTmp;
            closest = rayPoint;

            if (triPoint) triPoint = linePoint;
            if (rayPoint_) rayPoint_ = closest;
        }
    }

    return Geom::side(tri, closest) == Plane::Side::pos ? Alge::sqrt(sqrDist) : -Alge::sqrt(sqrDist);
}

template<class Real>
Real Intersect_<Real>::distance(const Triangle& tri, const Vec3& point, option<Vec3&> triPoint)
{
    Real dist_sqr = distanceSqr(tri, point, triPoint);
    return Geom::side(tri, point) == Plane::Side::pos ? Alge::sqrt(dist_sqr) : -Alge::sqrt(dist_sqr);
}

template<class Real>
Real Intersect_<Real>::distance(const Plane& plane, const Vec3& point, option<Vec3&> planePoint)
{
    Real distance = plane.normal.dot(point) - plane.dist;
    if (planePoint) planePoint = point + plane.normal*(-distance);
    return distance;
}

template<class Real>
Real Intersect_<Real>::distance(const Line& line1, const Line& line2, option<Vec3&> linePoint1, option<Vec3&> linePoint2)
{
    return Alge::sqrt(distanceSqr(line1, line2, linePoint1, linePoint2));
}

template<class Real>
Real Intersect_<Real>::distance(const Line& line, const Ray& ray, option<Vec3&> linePoint, option<Vec3&> rayPoint)
{
    return Alge::sqrt(distanceSqr(line, ray, linePoint, rayPoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Line& line, const Vec3& point, option<Vec3&> linePoint)
{
    return Alge::sqrt(distanceSqr(line, point, linePoint));
}

template<class Real>
Real Intersect_<Real>::distance(const Ray& ray1, const Ray& ray2, option<Vec3&> rayPoint1, option<Vec3&> rayPoint2)
{
    return Alge::sqrt(distanceSqr(ray1, ray2, rayPoint1, rayPoint2));
}

template<class Real>
Real Intersect_<Real>::distance(const Ray& ray, const Vec3& point, option<Vec3&> rayPoint)
{
    return Alge::sqrt(distanceSqr(ray, point, rayPoint));
}


template<class Real>
Real Intersect_<Real>::distanceSqr(const OrientBox& box, const Line& line, option<Vec3&> boxPoint, option<Vec3&> linePoint_)
{
    Real len;
    Vec3 dir = line.dir(len);
    Vec3 linePoint;

    Real distSqr = distanceSqr(box, Ray(line[0], dir), boxPoint, linePoint);

    if (dir.dot(linePoint - line[0]) > len)
    {
        //Closest point is beyond line. Get distance from line end point
        if (linePoint_) linePoint_ = line[1];
        return distanceSqr(box, line[1], boxPoint);
    }

    if (linePoint_) linePoint_ = linePoint;
    return distSqr;
}


//=======================================================================================================================

template<class Real>
Real Intersect_<Real>::distanceSqr(const OrientBox& box, const Ray& ray, option<Vec3&> boxPoint, option<Vec3&> rayPoint)
{
    // Compute coordinates of line in box coordinate system.
    const Vec3& extent = box.extent;
    const Vec3& center = box.center;

    Vec3 diff = ray.origin - center;
    Vec3 point(
        diff.dot(box.axis[0]),
        diff.dot(box.axis[1]),
        diff.dot(box.axis[2]));
    Vec3 direction(
        ray.dir.dot(box.axis[0]),
        ray.dir.dot(box.axis[1]),
        ray.dir.dot(box.axis[2]));

    // Apply reflections so that direction vector has nonnegative components.
    bool reflect[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        if (direction[i] < 0)
        {
            point[i] = -point[i];
            direction[i] = -direction[i];
            reflect[i] = true;
        }
        else
        {
            reflect[i] = false;
        }
    }

    Real sqrDistance = 0;
    Real lineParam = 0;

    if (direction.x > 0)
    {
        if (direction.y > 0)
        {
            if (direction.z > 0)  // (+,+,+)
            {
                OrientBoxRayDist::caseNoZeros(extent, lineParam, point, direction, sqrDistance);
            }
            else  // (+,+,0)
            {
                OrientBoxRayDist::case0(extent, lineParam, 0, 1, 2, point, direction, sqrDistance);
            }
        }
        else
        {
            if (direction.z > 0)  // (+,0,+)
            {
                OrientBoxRayDist::case0(extent, lineParam, 0, 2, 1, point, direction, sqrDistance);
            }
            else  // (+,0,0)
            {
                OrientBoxRayDist::case00(extent, lineParam, 0, 1, 2, point, direction, sqrDistance);
            }
        }
    }
    else
    {
        if (direction.y > 0)
        {
            if (direction.z > 0)  // (0,+,+)
            {
                OrientBoxRayDist::case0(extent, lineParam, 1, 2, 0, point, direction, sqrDistance);
            }
            else  // (0,+,0)
            {
                OrientBoxRayDist::case00(extent, lineParam, 1, 0, 2, point, direction, sqrDistance);
            }
        }
        else
        {
            if (direction.z > 0)  // (0,0,+)
            {
                OrientBoxRayDist::case00(extent, lineParam, 2, 0, 1, point, direction, sqrDistance);
            }
            else  // (0,0,0)
            {
                OrientBoxRayDist::case000(extent, lineParam, point, sqrDistance);
            }
        }
    }

    if (lineParam < 0)
    {
        //Closest point is behind ray. Get distance from ray origin
        if (rayPoint)
            rayPoint = ray.origin;
        return distanceSqr(box, ray.origin, boxPoint);
    }

    // Compute closest point on line.
    if (rayPoint)
        rayPoint = ray.origin + lineParam*ray.dir;

    // Compute closest point on box.
    if (boxPoint)
    {
        boxPoint = center;
        for (i = 0; i < 3; ++i)
        {
            // Undo the reflections applied previously.
            if (reflect[i])
            {
                point[i] = -point[i];
            }

            *boxPoint += point[i]*box.axis[i];
        }
    }

    return sqrDistance;
}


template<class Real>
void Intersect_<Real>::OrientBoxRayDist::
    face(   const Vec3& extent, Real& lineParam, int i0, int i1, int i2,
            Vec3& pnt, const Vec3& dir, const Vec3& PmE, Real& sqrDistance)
{
    Vec3 PpE;
    Real lenSqr, inv, tmp, param, t, delta;

    PpE[i1] = pnt[i1] + extent[i1];
    PpE[i2] = pnt[i2] + extent[i2];
    if (dir[i0]*PpE[i1] >= dir[i1]*PmE[i0])
    {
        if (dir[i0]*PpE[i2] >= dir[i2]*PmE[i0])
        {
            // v[i1] >= -e[i1], v[i2] >= -e[i2] (distance = 0)
            pnt[i0] = extent[i0];
            inv = 1/dir[i0];
            pnt[i1] -= dir[i1]*PmE[i0]*inv;
            pnt[i2] -= dir[i2]*PmE[i0]*inv;
            lineParam = -PmE[i0]*inv;
        }
        else
        {
            // v[i1] >= -e[i1], v[i2] < -e[i2]
            lenSqr = dir[i0]*dir[i0] + dir[i2]*dir[i2];
            tmp = lenSqr*PpE[i1] - dir[i1]*(dir[i0]*PmE[i0] +
                dir[i2]*PpE[i2]);
            if (tmp <= ((Real)2)*lenSqr*extent[i1])
            {
                t = tmp/lenSqr;
                lenSqr += dir[i1]*dir[i1];
                tmp = PpE[i1] - t;
                delta = dir[i0]*PmE[i0] + dir[i1]*tmp + dir[i2]*PpE[i2];
                param = -delta/lenSqr;
                sqrDistance += PmE[i0]*PmE[i0] + tmp*tmp + PpE[i2]*PpE[i2]
                    + delta*param;

                lineParam = param;
                pnt[i0] = extent[i0];
                pnt[i1] = t - extent[i1];
                pnt[i2] = -extent[i2];
            }
            else
            {
                lenSqr += dir[i1]*dir[i1];
                delta = dir[i0]*PmE[i0] + dir[i1]*PmE[i1] + dir[i2]*PpE[i2];
                param = -delta/lenSqr;
                sqrDistance += PmE[i0]*PmE[i0] + PmE[i1]*PmE[i1]
                    + PpE[i2]*PpE[i2] + delta*param;

                lineParam = param;
                pnt[i0] = extent[i0];
                pnt[i1] = extent[i1];
                pnt[i2] = -extent[i2];
            }
        }
    }
    else
    {
        if (dir[i0]*PpE[i2] >= dir[i2]*PmE[i0])
        {
            // v[i1] < -e[i1], v[i2] >= -e[i2]
            lenSqr = dir[i0]*dir[i0] + dir[i1]*dir[i1];
            tmp = lenSqr*PpE[i2] - dir[i2]*(dir[i0]*PmE[i0] +
                dir[i1]*PpE[i1]);
            if (tmp <= ((Real)2)*lenSqr*extent[i2])
            {
                t = tmp/lenSqr;
                lenSqr += dir[i2]*dir[i2];
                tmp = PpE[i2] - t;
                delta = dir[i0]*PmE[i0] + dir[i1]*PpE[i1] + dir[i2]*tmp;
                param = -delta/lenSqr;
                sqrDistance += PmE[i0]*PmE[i0] + PpE[i1]*PpE[i1] + tmp*tmp
                    + delta*param;

                lineParam = param;
                pnt[i0] = extent[i0];
                pnt[i1] = -extent[i1];
                pnt[i2] = t - extent[i2];
            }
            else
            {
                lenSqr += dir[i2]*dir[i2];
                delta = dir[i0]*PmE[i0] + dir[i1]*PpE[i1] + dir[i2]*PmE[i2];
                param = -delta/lenSqr;
                sqrDistance += PmE[i0]*PmE[i0] + PpE[i1]*PpE[i1] +
                    PmE[i2]*PmE[i2] + delta*param;

                lineParam = param;
                pnt[i0] = extent[i0];
                pnt[i1] = -extent[i1];
                pnt[i2] = extent[i2];
            }
        }
        else
        {
            // v[i1] < -e[i1], v[i2] < -e[i2]
            lenSqr = dir[i0]*dir[i0] + dir[i2]*dir[i2];
            tmp = lenSqr*PpE[i1] - dir[i1]*(dir[i0]*PmE[i0] +
                dir[i2]*PpE[i2]);
            if (tmp >= 0)
            {
                // v[i1]-edge is closest
                if (tmp <= ((Real)2)*lenSqr*extent[i1])
                {
                    t = tmp/lenSqr;
                    lenSqr += dir[i1]*dir[i1];
                    tmp = PpE[i1] - t;
                    delta = dir[i0]*PmE[i0] + dir[i1]*tmp + dir[i2]*PpE[i2];
                    param = -delta/lenSqr;
                    sqrDistance += PmE[i0]*PmE[i0] + tmp*tmp + PpE[i2]*PpE[i2]
                        + delta*param;

                    lineParam = param;
                    pnt[i0] = extent[i0];
                    pnt[i1] = t - extent[i1];
                    pnt[i2] = -extent[i2];
                }
                else
                {
                    lenSqr += dir[i1]*dir[i1];
                    delta = dir[i0]*PmE[i0] + dir[i1]*PmE[i1]
                        + dir[i2]*PpE[i2];
                    param = -delta/lenSqr;
                    sqrDistance += PmE[i0]*PmE[i0] + PmE[i1]*PmE[i1]
                        + PpE[i2]*PpE[i2] + delta*param;

                    lineParam = param;
                    pnt[i0] = extent[i0];
                    pnt[i1] = extent[i1];
                    pnt[i2] = -extent[i2];
                }
                return;
            }

            lenSqr = dir[i0]*dir[i0] + dir[i1]*dir[i1];
            tmp = lenSqr*PpE[i2] - dir[i2]*(dir[i0]*PmE[i0] +
                dir[i1]*PpE[i1]);
            if (tmp >= 0)
            {
                // v[i2]-edge is closest
                if (tmp <= ((Real)2)*lenSqr*extent[i2])
                {
                    t = tmp/lenSqr;
                    lenSqr += dir[i2]*dir[i2];
                    tmp = PpE[i2] - t;
                    delta = dir[i0]*PmE[i0] + dir[i1]*PpE[i1] + dir[i2]*tmp;
                    param = -delta/lenSqr;
                    sqrDistance += PmE[i0]*PmE[i0] + PpE[i1]*PpE[i1] +
                        tmp*tmp + delta*param;

                    lineParam = param;
                    pnt[i0] = extent[i0];
                    pnt[i1] = -extent[i1];
                    pnt[i2] = t - extent[i2];
                }
                else
                {
                    lenSqr += dir[i2]*dir[i2];
                    delta = dir[i0]*PmE[i0] + dir[i1]*PpE[i1]
                        + dir[i2]*PmE[i2];
                    param = -delta/lenSqr;
                    sqrDistance += PmE[i0]*PmE[i0] + PpE[i1]*PpE[i1]
                        + PmE[i2]*PmE[i2] + delta*param;

                    lineParam = param;
                    pnt[i0] = extent[i0];
                    pnt[i1] = -extent[i1];
                    pnt[i2] = extent[i2];
                }
                return;
            }

            // (v[i1],v[i2])-corner is closest
            lenSqr += dir[i2]*dir[i2];
            delta = dir[i0]*PmE[i0] + dir[i1]*PpE[i1] + dir[i2]*PpE[i2];
            param = -delta/lenSqr;
            sqrDistance += PmE[i0]*PmE[i0] + PpE[i1]*PpE[i1]
                + PpE[i2]*PpE[i2] + delta*param;

            lineParam = param;
            pnt[i0] = extent[i0];
            pnt[i1] = -extent[i1];
            pnt[i2] = -extent[i2];
        }
    }
}

template<class Real>
void Intersect_<Real>::OrientBoxRayDist::
    caseNoZeros(const Vec3& extent, Real& lineParam,
                Vec3& pnt, const Vec3& dir, Real& sqrDistance)
{
    Vec3 PmE(
        pnt.x - extent[0],
        pnt.y - extent[1],
        pnt.z - extent[2]);

    Real prodDxPy = dir.x*PmE.y;
    Real prodDyPx = dir.y*PmE.x;
    Real prodDzPx, prodDxPz, prodDzPy, prodDyPz;

    if (prodDyPx >= prodDxPy)
    {
        prodDzPx = dir.z*PmE.x;
        prodDxPz = dir.x*PmE.z;
        if (prodDzPx >= prodDxPz)
        {
            // line intersects x = e0
            face(extent, lineParam, 0, 1, 2, pnt, dir, PmE, sqrDistance);
        }
        else
        {
            // line intersects z = e2
            face(extent, lineParam, 2, 0, 1, pnt, dir, PmE, sqrDistance);
        }
    }
    else
    {
        prodDzPy = dir.z*PmE.y;
        prodDyPz = dir.y*PmE.z;
        if (prodDzPy >= prodDyPz)
        {
            // line intersects y = e1
            face(extent, lineParam, 1, 2, 0, pnt, dir, PmE, sqrDistance);
        }
        else
        {
            // line intersects z = e2
            face(extent, lineParam, 2, 0, 1, pnt, dir, PmE, sqrDistance);
        }
    }
}

template<class Real>
void Intersect_<Real>::OrientBoxRayDist::
    case0(  const Vec3& extent, Real& lineParam,
            int i0, int i1, int i2, Vec3& pnt, const Vec3& dir, Real& sqrDistance)
{
    Real PmE0 = pnt[i0] - extent[i0];
    Real PmE1 = pnt[i1] - extent[i1];
    Real prod0 = dir[i1]*PmE0;
    Real prod1 = dir[i0]*PmE1;
    Real delta, invLSqr, inv;

    if (prod0 >= prod1)
    {
        // line intersects P[i0] = e[i0]
        pnt[i0] = extent[i0];

        Real PpE1 = pnt[i1] + extent[i1];
        delta = prod0 - dir[i0]*PpE1;
        if (delta >= 0)
        {
            invLSqr = 1/(dir[i0]*dir[i0] + dir[i1]*dir[i1]);
            sqrDistance += delta*delta*invLSqr;
            pnt[i1] = -extent[i1];
            lineParam = -(dir[i0]*PmE0 + dir[i1]*PpE1)*invLSqr;
        }
        else
        {
            inv = 1/dir[i0];
            pnt[i1] -= prod0*inv;
            lineParam = -PmE0*inv;
        }
    }
    else
    {
        // line intersects P[i1] = e[i1]
        pnt[i1] = extent[i1];

        Real PpE0 = pnt[i0] + extent[i0];
        delta = prod1 - dir[i1]*PpE0;
        if (delta >= 0)
        {
            invLSqr = 1/(dir[i0]*dir[i0] + dir[i1]*dir[i1]);
            sqrDistance += delta*delta*invLSqr;
            pnt[i0] = -extent[i0];
            lineParam = -(dir[i0]*PpE0 + dir[i1]*PmE1)*invLSqr;
        }
        else
        {
            inv = 1/dir[i1];
            pnt[i0] -= prod1*inv;
            lineParam = -PmE1*inv;
        }
    }

    if (pnt[i2] < -extent[i2])
    {
        delta = pnt[i2] + extent[i2];
        sqrDistance += delta*delta;
        pnt[i2] = -extent[i2];
    }
    else if (pnt[i2] > extent[i2])
    {
        delta = pnt[i2] - extent[i2];
        sqrDistance += delta*delta;
        pnt[i2] = extent[i2];
    }
}

template<class Real>
void Intersect_<Real>::OrientBoxRayDist::
    case00( const Vec3& extent, Real& lineParam,
            int i0, int i1, int i2, Vec3& pnt, const Vec3& dir,  Real& sqrDistance)
{
    Real delta;

    lineParam = (extent[i0] - pnt[i0])/dir[i0];

    pnt[i0] = extent[i0];

    if (pnt[i1] < -extent[i1])
    {
        delta = pnt[i1] + extent[i1];
        sqrDistance += delta*delta;
        pnt[i1] = -extent[i1];
    }
    else if (pnt[i1] > extent[i1])
    {
        delta = pnt[i1] - extent[i1];
        sqrDistance += delta*delta;
        pnt[i1] = extent[i1];
    }

    if (pnt[i2] < -extent[i2])
    {
        delta = pnt[i2] + extent[i2];
        sqrDistance += delta*delta;
        pnt[i2] = -extent[i2];
    }
    else if (pnt[i2] > extent[i2])
    {
        delta = pnt[i2] - extent[i2];
        sqrDistance += delta*delta;
        pnt[i2] = extent[i2];
    }
}

template<class Real>
void Intersect_<Real>::OrientBoxRayDist::
    case000(    const Vec3& extent, Real& /*lineParam*/,
                Vec3& pnt, Real& sqrDistance)
{
    Real delta;

    if (pnt.x < -extent[0])
    {
        delta = pnt.x + extent[0];
        sqrDistance += delta*delta;
        pnt.x = -extent[0];
    }
    else if (pnt.x > extent[0])
    {
        delta = pnt.x - extent[0];
        sqrDistance += delta*delta;
        pnt.x = extent[0];
    }

    if (pnt.y < -extent[1])
    {
        delta = pnt.y + extent[1];
        sqrDistance += delta*delta;
        pnt.y = -extent[1];
    }
    else if (pnt.y > extent[1])
    {
        delta = pnt.y - extent[1];
        sqrDistance += delta*delta;
        pnt.y = extent[1];
    }

    if (pnt.z < -extent[2])
    {
        delta = pnt.z + extent[2];
        sqrDistance += delta*delta;
        pnt.z = -extent[2];
    }
    else if (pnt.z > extent[2])
    {
        delta = pnt.z - extent[2];
        sqrDistance += delta*delta;
        pnt.z = extent[2];
    }
}

//=======================================================================================================================

template<class Real>
Real Intersect_<Real>::distanceSqr(const OrientBox& box, const Vec3& point, option<Vec3&> boxPoint)
{
    // Work in the box's coordinate system.
    Vec3 diff = point - box.center;

    // Compute squared distance and closest point on box.
    Real sqrDistance = 0;
    Real delta;
    Vec3 closest;
    for (int i = 0; i < 3; ++i)
    {
        closest[i] = diff.dot(box.axis[i]);
        if (closest[i] < -box.extent[i])
        {
            delta = closest[i] + box.extent[i];
            sqrDistance += delta*delta;
            closest[i] = -box.extent[i];
        }
        else if (closest[i] > box.extent[i])
        {
            delta = closest[i] - box.extent[i];
            sqrDistance += delta*delta;
            closest[i] = box.extent[i];
        }
    }

    if (boxPoint)
    {
        boxPoint = box.center;
        for (int i = 0; i < 3; ++i)
            *boxPoint += closest[i]*box.axis[i];
    }

    return sqrDistance;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Box& box1, const Box& box2, option<Vec3&> boxPoint1, option<Vec3&> boxPoint2)
{
    Vec3 p1, p2;
    Real distSqr = 0;

    for (int i = 0; i < 3; ++i)
    {
        const Real& min1 = box1.min[i];
        const Real& max1 = box1.max[i];
        const Real& min2 = box2.min[i];
        const Real& max2 = box2.max[i];

        if (min1 > max2)
        {
            distSqr += Alge::sqr(max2 - min1);
            p1[i] = min1;
            p2[i] = max2;
        }
        else if (min2 > max1)
        {
            distSqr += Alge::sqr(max1 - min2);
            p1[i] = max1;
            p2[i] = min2;
        }
        else //the projection intervals overlap.
        {
            if (min1 < min2)
            {
                p1[i] = min2;
                p2[i] = min2;
            }
            else
            {
                p1[i] = min1;
                p2[i] = min1;
            }
        }
    }

    if (boxPoint1) boxPoint1 = p1;
    if (boxPoint2) boxPoint2 = p2;
    return distSqr;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Box& box, const Line& line, option<Vec3&> boxPoint, option<Vec3&> linePoint_)
{
    Real len;
    Vec3 dir = line.dir(len);
    Vec3 linePoint;

    Real distSqr = distanceSqr(box, Ray(line[0], dir), boxPoint, linePoint);

    if (dir.dot(linePoint - line[0]) > len)
    {
        //Closest point is beyond line. Get distance from line end point
        if (linePoint_) linePoint_ = line[1];
        return distanceSqr(box, line[1], boxPoint);
    }

    if (linePoint_) linePoint_ = linePoint;
    return distSqr;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Box& box, const Ray& ray, option<Vec3&> boxPoint, option<Vec3&> rayPoint)
{
    // Compute coordinates of line in box coordinate system.
    Vec3 extent = box.getExtent();
    Vec3 center = box.getCenter();

    Vec3 point = ray.origin - center;
    Vec3 direction = ray.dir;

    // Apply reflections so that direction vector has nonnegative components.
    bool reflect[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        if (direction[i] < 0)
        {
            point[i] = -point[i];
            direction[i] = -direction[i];
            reflect[i] = true;
        }
        else
        {
            reflect[i] = false;
        }
    }

    Real sqrDistance = 0;
    Real lineParam = 0;

    if (direction.x > 0)
    {
        if (direction.y > 0)
        {
            if (direction.z > 0)  // (+,+,+)
            {
                OrientBoxRayDist::caseNoZeros(extent, lineParam, point, direction, sqrDistance);
            }
            else  // (+,+,0)
            {
                OrientBoxRayDist::case0(extent, lineParam, 0, 1, 2, point, direction, sqrDistance);
            }
        }
        else
        {
            if (direction.z > 0)  // (+,0,+)
            {
                OrientBoxRayDist::case0(extent, lineParam, 0, 2, 1, point, direction, sqrDistance);
            }
            else  // (+,0,0)
            {
                OrientBoxRayDist::case00(extent, lineParam, 0, 1, 2, point, direction, sqrDistance);
            }
        }
    }
    else
    {
        if (direction.y > 0)
        {
            if (direction.z > 0)  // (0,+,+)
            {
                OrientBoxRayDist::case0(extent, lineParam, 1, 2, 0, point, direction, sqrDistance);
            }
            else  // (0,+,0)
            {
                OrientBoxRayDist::case00(extent, lineParam, 1, 0, 2, point, direction, sqrDistance);
            }
        }
        else
        {
            if (direction.z > 0)  // (0,0,+)
            {
                OrientBoxRayDist::case00(extent, lineParam, 2, 0, 1, point, direction, sqrDistance);
            }
            else  // (0,0,0)
            {
                OrientBoxRayDist::case000(extent, lineParam, point, sqrDistance);
            }
        }
    }

    if (lineParam < 0)
    {
        //Closest point is behind ray. Get distance from ray origin
        if (rayPoint) rayPoint = ray.origin;
        return distanceSqr(box, ray.origin, boxPoint);
    }

    // Compute closest point on line.
    if (rayPoint) rayPoint = ray.origin + lineParam*ray.dir;

    // Compute closest point on box.
    if (boxPoint)
    {
        boxPoint = center;
        for (i = 0; i < 3; ++i)
        {
            // Undo the reflections applied previously.
            if (reflect[i])
            {
                point[i] = -point[i];
            }

            (*boxPoint)[i] += point[i];
        }
    }

    return sqrDistance;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Box& box, const Vec3& point, option<Vec3&> boxPoint)
{
    if (test(box, point))
    {
        //Point is inside the box, no distance
        if (boxPoint) boxPoint = point;
        return 0;
    }

    //Clamp the point in the box
    Vec3 clamped = point.clamp(box.min, box.max);
    if (boxPoint) boxPoint = clamped;
    return (point-clamped).lengthSqr();
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Triangle& tri, const Vec3& point, option<Vec3&> triPoint)
{
    Vec3 kDiff = tri.v0 - point;
    Vec3 kEdge0 = tri.v1 - tri.v0;
    Vec3 kEdge1 = tri.v2 - tri.v0;
    Real a00 = kEdge0.lengthSqr();
    Real a01 = kEdge0.dot(kEdge1);
    Real a11 = kEdge1.lengthSqr();
    Real b0 = kDiff.dot(kEdge0);
    Real b1 = kDiff.dot(kEdge1);
    Real c = kDiff.lengthSqr();
    Real det = Alge::abs(a00*a11-a01*a01);
    Real s = a01*b1-a11*b0;
    Real t = a01*b0-a00*b1;
    Real sqrDistance;

    if (s + t <= det)
    {
        if (s < 0)
        {
            if (t < 0)  // region 4
            {
                if (b0 < 0)
                {
                    t = 0;
                    if (-b0 >= a00)
                    {
                        s = 1;
                        sqrDistance = a00+2*b0+c;
                    }
                    else
                    {
                        s = -b0/a00;
                        sqrDistance = b0*s+c;
                    }
                }
                else
                {
                    s = 0;
                    if (b1 >= 0)
                    {
                        t = 0;
                        sqrDistance = c;
                    }
                    else if (-b1 >= a11)
                    {
                        t = 1;
                        sqrDistance = a11+2*b1+c;
                    }
                    else
                    {
                        t = -b1/a11;
                        sqrDistance = b1*t+c;
                    }
                }
            }
            else  // region 3
            {
                s = 0;
                if (b1 >= 0)
                {
                    t = 0;
                    sqrDistance = c;
                }
                else if (-b1 >= a11)
                {
                    t = 1;
                    sqrDistance = a11+2*b1+c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t+c;
                }
            }
        }
        else if (t < 0)  // region 5
        {
            t = 0;
            if (b0 >= 0)
            {
                s = 0;
                sqrDistance = c;
            }
            else if (-b0 >= a00)
            {
                s = 1;
                sqrDistance = a00+2*b0+c;
            }
            else
            {
                s = -b0/a00;
                sqrDistance = b0*s+c;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            Real invDet = 1/det;
            s *= invDet;
            t *= invDet;
            sqrDistance = s*(a00*s+a01*t+2*b0) +
                t*(a01*s+a11*t+2*b1)+c;
        }
    }
    else
    {
        Real tmp0, tmp1, numer, denom;

        if (s < 0)  // region 2
        {
            tmp0 = a01 + b0;
            tmp1 = a11 + b1;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00-2*a01+a11;
                if (numer >= denom)
                {
                    s = 1;
                    t = 0;
                    sqrDistance = a00+2*b0+c;
                }
                else
                {
                    s = numer/denom;
                    t = 1 - s;
                    sqrDistance = s*(a00*s+a01*t+2*b0) +
                        t*(a01*s+a11*t+2*b1)+c;
                }
            }
            else
            {
                s = 0;
                if (tmp1 <= 0)
                {
                    t = 1;
                    sqrDistance = a11+2*b1+c;
                }
                else if (b1 >= 0)
                {
                    t = 0;
                    sqrDistance = c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t+c;
                }
            }
        }
        else if (t < 0)  // region 6
        {
            tmp0 = a01 + b1;
            tmp1 = a00 + b0;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00-2*a01+a11;
                if (numer >= denom)
                {
                    t = 1;
                    s = 0;
                    sqrDistance = a11+2*b1+c;
                }
                else
                {
                    t = numer/denom;
                    s = 1 - t;
                    sqrDistance = s*(a00*s+a01*t+2*b0) +
                        t*(a01*s+a11*t+2*b1)+c;
                }
            }
            else
            {
                t = 0;
                if (tmp1 <= 0)
                {
                    s = 1;
                    sqrDistance = a00+2*b0+c;
                }
                else if (b0 >= 0)
                {
                    s = 0;
                    sqrDistance = c;
                }
                else
                {
                    s = -b0/a00;
                    sqrDistance = b0*s+c;
                }
            }
        }
        else  // region 1
        {
            numer = a11 + b1 - a01 - b0;
            if (numer <= 0)
            {
                s = 0;
                t = 1;
                sqrDistance = a11+2*b1+c;
            }
            else
            {
                denom = a00-2*a01+a11;
                if (numer >= denom)
                {
                    s = 1;
                    t = 0;
                    sqrDistance = a00+2*b0+c;
                }
                else
                {
                    s = numer/denom;
                    t = 1 - s;
                    sqrDistance = s*(a00*s+a01*t+2*b0) +
                        t*(a01*s+a11*t+2*b1)+c;
                }
            }
        }
    }

    // account for numerical round-off error
    if (sqrDistance < 0) sqrDistance = 0;
    if (triPoint) triPoint = tri.v0 + s*kEdge0 + t*kEdge1;
    return sqrDistance;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Line& line1, const Line& line2, option<Vec3&> linePoint1_, option<Vec3&> linePoint2_)
{
    const Vec3&   v0 = line1.v0;
    const Vec3&   v1 = line1.v1;
    const Vec3&   v2 = line2.v0;
    const Vec3&   v3 = line2.v1;

    Vec3    u = v1 - v0;
    Vec3    v = v3 - v2;
    Vec3    w = v0 - v2;
    Real    a = u.dot(u);        // always >= 0
    Real    b = u.dot(v);
    Real    c = v.dot(v);        // always >= 0
    Real    d = u.dot(w);
    Real    e = v.dot(w);
    Real    D = a*c - b*b;       // always >= 0
    Real    sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
    Real    tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (Alge::isNearZero(D))
    {
        // the lines are almost parallel
        sN = 0;       // force using point P0 on segment S1
        sD = 1;        // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else
    {
        // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0)
        {
            // sc < 0 => the s=0 edge is visible
            sN = 0;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            // sc > 1 => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0) {           // tc < 0 => the t=0 edge is visible
        tN = 0;
        // recompute sc for this edge
        if (-d < 0)
            sN = 0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1 => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d + b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = Alge::isNearZero(sN) ? 0 : sN / sD;
    tc = Alge::isNearZero(tN) ? 0 : tN / tD;

    Vec3 linePoint1 = v0 + sc * u;
    Vec3 linePoint2 = v2 + tc * v;

    if (linePoint1_) linePoint1_ = linePoint1;
    if (linePoint2_) linePoint2_ = linePoint2;
    return (linePoint2 - linePoint1).lengthSqr();   // return the closest distance
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Line& line, const Ray& ray, option<Vec3&> linePoint_, option<Vec3&> rayPoint)
{
    Real len;
    Vec3 dir = line.dir(len);
    Vec3 linePoint;

    Real distSqr = distanceSqr(Ray(line[0], dir), ray, linePoint, rayPoint);

    if (dir.dot(linePoint - line[0]) > len)
    {
        //Closest point is beyond line. Get distance from line end point
        if (linePoint_) linePoint_ = line[1];
        return distanceSqr(ray, line[1], rayPoint);
    }

    if (linePoint_) linePoint_ = linePoint;
    return distSqr;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Line& line, const Vec3& point, option<Vec3&> linePoint_)
{
    Vec3 dir = line.diff();
    Real t = 0;
    Real div = dir.lengthSqr();
    if (div != 0)
        t = (point - line[0]).dot(dir) / div;
    if (t < 0)
        t = 0;
    else if (t > 1)
        t = 1;

    Vec3 linePoint = line[0] + dir*t;
    if (linePoint_) linePoint_ = linePoint;
    return (linePoint - point).lengthSqr();
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Ray& ray1, const Ray& ray2, option<Vec3&> rayPoint1, option<Vec3&> rayPoint2)
{
    Vec3 diff = ray1.origin - ray2.origin;
    Real a01 = -ray1.dir.dot(ray2.dir);
    Real b0 = diff.dot(ray1.dir);
    Real c = diff.lengthSqr();
    Real det = Alge::abs(1 - a01*a01);
    Real b1, s0, s1, sqrDist;

    if (det >= Real_::zeroTol)
    {
        // Rays are not parallel.
        b1 = -diff.dot(ray2.dir);
        s0 = a01*b1 - b0;
        s1 = a01*b0 - b1;

        if (s0 >= 0)
        {
            if (s1 >= 0)  // region 0 (interior)
            {
                // Minimum at two interior points of rays.
                Real invDet = 1/det;
                s0 *= invDet;
                s1 *= invDet;
                sqrDist = s0*(s0 + a01*s1 + 2*b0) +
                    s1*(a01*s0 + s1 + 2*b1) + c;
            }
            else  // region 3 (side)
            {
                s1 = 0;
                if (b0 >= 0)
                {
                    s0 = 0;
                    sqrDist = c;
                }
                else
                {
                    s0 = -b0;
                    sqrDist = b0*s0 + c;
                }
            }
        }
        else
        {
            if (s1 >= 0)  // region 1 (side)
            {
                s0 = 0;
                if (b1 >= 0)
                {
                    s1 = 0;
                    sqrDist = c;
                }
                else
                {
                    s1 = -b1;
                    sqrDist = b1*s1 + c;
                }
            }
            else  // region 2 (corner)
            {
                if (b0 < 0)
                {
                    s0 = -b0;
                    s1 = 0;
                    sqrDist = b0*s0 + c;
                }
                else
                {
                    s0 = 0;
                    if (b1 >= 0)
                    {
                        s1 = 0;
                        sqrDist = c;
                    }
                    else
                    {
                        s1 = -b1;
                        sqrDist = b1*s1 + c;
                    }
                }
            }
        }
    }
    else
    {
        // Rays are parallel.
        if (a01 > 0)
        {
            // Opposite direction vectors.
            s1 = 0;
            if (b0 >= 0)
            {
                s0 = 0;
                sqrDist = c;
            }
            else
            {
                s0 = -b0;
                sqrDist = b0*s0 + c;
            }
        }
        else
        {
            // Same direction vectors.
            if (b0 >= 0)
            {
                b1 = -diff.dot(ray2.dir);
                s0 = 0;
                s1 = -b1;
                sqrDist = b1*s1 + c;
            }
            else
            {
                s0 = -b0;
                s1 = 0;
                sqrDist = b0*s0 + c;
            }
        }
    }

    if (rayPoint1) rayPoint1 = ray1.origin + s0*ray1.dir;
    if (rayPoint2) rayPoint2 = ray2.origin + s1*ray2.dir;
    // Account for numerical round-off errors.
    if (sqrDist < 0) sqrDist = 0;
    return sqrDist;
}

template<class Real>
Real Intersect_<Real>::distanceSqr(const Ray& ray, const Vec3& point, option<Vec3&> rayPoint_)
{
    Real t = 0;
    Real div = ray.dir.lengthSqr();
    if (div != 0)
        t = (point - ray.origin).dot(ray.dir) / div;
    if (t < 0)
        t = 0;

    Vec3 rayPoint = ray.origin + ray.dir*t;
    if (rayPoint_) rayPoint_ = rayPoint;
    return (rayPoint - point).lengthSqr();
}

template<class Real>
bool Intersect_<Real>::find(const OrientBox& box, const Triangle& tri, option<int&> pointCount, Vec3 intPoints[3])
{
    // Start with the triangle and clip it against each face of the box.
    Vec3 points[3];
    int quantity = 3;
    for (int i = 0; i < 3; ++i)
    {
        points[i] = tri[i];
    }

    for (int dir = -1; dir <= 1; dir += 2)
    {
        for (int side = 0; side < 3; ++side)
        {
            Vec3 innerNormal = ((Real)dir)*box.axis[side];
            Real constant = innerNormal.dot(box.center) - box.extent[side];
            OrientBoxTriangleInt::clipConvexPolygonAgainstPlane(innerNormal, constant, quantity, points);
        }
    }

    if (pointCount) pointCount = quantity;
    if (intPoints)
        for (int i = 0; i < quantity; ++i)
            intPoints[i] = points[i];

    return quantity > 0;
}

template<class Real>
void Intersect_<Real>::OrientBoxTriangleInt::
    clipConvexPolygonAgainstPlane(const Vec3& normal, Real constant, int& quantity, Vec3* P)
{
    // The input vertices are assumed to be in counterclockwise order.  The
    // ordering is an invariant of this function.  The size of array P is
    // assumed to be large enough to store the clipped polygon vertices.

    // test on which side of line are the vertices
    int positive = 0, negative = 0, index = -1;
    int currQuantity = quantity;

    Real test[8];
    int i;
    for (i = 0; i < quantity; ++i)
    {

        // An epsilon is used here because it is possible for the dot product
        // and 'constant' to be exactly equal to each other (in theory), but
        // differ slightly because of floating point problems.  Thus, add a
        // little to the test number to push actually equal numbers over the
        // edge towards the positive.

        // TODO: This should probably be a relative tolerance.  Multiplying
        // by the constant is probably not the best way to do this.
        test[i] = normal.dot(P[i]) - constant + Alge::abs(constant)*Real_::zeroTol;

        if (test[i] >= 0)
        {
            ++positive;
            if (index < 0)
            {
                index = i;
            }
        }
        else
        {
            ++negative;
        }
    }

    if (quantity == 2)
    {
        // Lines are a little different, in that clipping the segment
        // cannot create a new segment, as clipping a polygon would
        if (positive > 0)
        {
            if (negative > 0) 
            {
                int clip;

                if (index == 0)
                {
                    // vertex0 positive, vertex1 is clipped
                    clip = 1;
                }
                else // index == 1
                {
                    // vertex1 positive, vertex0 clipped
                    clip = 0;
                }

                Real t = test[index]/(test[index] - test[clip]);
                P[clip] = P[index] + t*(P[clip] - P[index]);
            }
            // otherwise both positive, no clipping
        }
        else
        {
            // Assert:  The entire line is clipped, but we should not
            // get here.
            quantity = 0;
        }
    }
    else
    {
        if (positive > 0)
        {
            if (negative > 0)
            {
                // plane transversely intersects polygon
                Vec3 CV[8];
                int cQuantity = 0, cur, prv;
                Real t;

                if (index > 0)
                {
                    // first clip vertex on line
                    cur = index;
                    prv = cur - 1;
                    t = test[cur]/(test[cur] - test[prv]);
                    CV[cQuantity++] = P[cur] + t*(P[prv] - P[cur]);

                    // vertices on positive side of line
                    while (cur < currQuantity && test[cur] >= 0)
                    {
                        CV[cQuantity++] = P[cur++];
                    }

                    // last clip vertex on line
                    if (cur < currQuantity)
                    {
                        prv = cur - 1;
                    }
                    else
                    {
                        cur = 0;
                        prv = currQuantity - 1;
                    }
                    t = test[cur]/(test[cur] - test[prv]);
                    CV[cQuantity++] = P[cur] + t*(P[prv] - P[cur]);
                }
                else  // index is 0
                {
                    // vertices on positive side of line
                    cur = 0;
                    while (cur < currQuantity && test[cur] >= 0)
                    {
                        CV[cQuantity++] = P[cur++];
                    }

                    // last clip vertex on line
                    prv = cur - 1;
                    t = test[cur]/(test[cur] - test[prv]);
                    CV[cQuantity++] = P[cur] + t*(P[prv] - P[cur]);

                    // skip vertices on negative side
                    while (cur < currQuantity && test[cur] < 0)
                    {
                        cur++;
                    }

                    // first clip vertex on line
                    if (cur < currQuantity)
                    {
                        prv = cur - 1;
                        t = test[cur]/(test[cur] - test[prv]);
                        CV[cQuantity++] = P[cur] + t*(P[prv] - P[cur]);

                        // vertices on positive side of line
                        while (cur < currQuantity && test[cur] >= 0)
                        {
                            CV[cQuantity++] = P[cur++];
                        }
                    }
                    else
                    {
                        // cur = 0
                        prv = currQuantity - 1;
                        t = test[0]/(test[0] - test[prv]);
                        CV[cQuantity++] = P[0] + t*(P[prv] - P[0]);
                    }
                }

                currQuantity = cQuantity;
                memcpy(P, CV, cQuantity*sizeof(Vec3));
            }
            // else polygon fully on positive side of plane, nothing to do

            quantity = currQuantity;
        }
        else
        {
            // Polygon does not intersect positive side of plane, clip all.
            // This should not ever happen if called by the findintersect
            // routines after an intersection has been determined.
            quantity = 0;
        }    
    }
}

template<class Real>
bool Intersect_<Real>::find(const OrientBox& box, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    int num = find(box, Ray(line[0], dir), t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(box, line[0]))
        t[0] = 0;
    if (test(box, line[1]))
        t[1] = len;

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const OrientBox& box, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real t[2];
    int num = find(box, ray, t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(box, ray.origin))
        t[0] = 0;

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    return true;
}

template<class Real>
int Intersect_<Real>::find(const OrientBox& box, const Ray& ray, Real t[2])
{
    // Convert linear component to box coordinates.
    Vec3 diff = ray.origin - box.center;
    Vec3 BOrigin(
        diff.dot(box.axis[0]),
        diff.dot(box.axis[1]),
        diff.dot(box.axis[2])
    );
    Vec3 BDirection(
        ray.dir.dot(box.axis[0]),
        ray.dir.dot(box.axis[1]),
        ray.dir.dot(box.axis[2])
    );

    t[0] = -Real_::max;
    t[1] = Real_::max;

    bool notAllClipped =
        OrientBoxRayInt::clip(+BDirection.x, -BOrigin.x-box.extent[0], t) &&
        OrientBoxRayInt::clip(-BDirection.x, +BOrigin.x-box.extent[0], t) &&
        OrientBoxRayInt::clip(+BDirection.y, -BOrigin.y-box.extent[1], t) &&
        OrientBoxRayInt::clip(-BDirection.y, +BOrigin.y-box.extent[1], t) &&
        OrientBoxRayInt::clip(+BDirection.z, -BOrigin.z-box.extent[2], t) &&
        OrientBoxRayInt::clip(-BDirection.z, +BOrigin.z-box.extent[2], t);

    if (notAllClipped)
    {
        if (t[1] > t[0])
            return 2;
        else
            return 1;
    }

    return 0;
}

template<class Real>
bool Intersect_<Real>::OrientBoxRayInt::
    clip(Real denom, Real numer, Real t[2])
{
    // Return value is 'true' if line segment intersects the current test
    // plane.  Otherwise 'false' is returned in which case the line segment
    // is entirely clipped.

    if (denom > 0)
    {
        if (numer > denom*t[1])
            return false;
        if (numer > denom*t[0])
            t[0] = numer/denom;
        return true;
    }
    else if (denom < 0)
    {
        if (numer > denom*t[0])
            return false;
        if (numer > denom*t[1])
            t[1] = numer/denom;
        return true;
    }
    else
        return numer <= 0;
}

template<class Real>
bool Intersect_<Real>::find(const Capsule& capsule, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    int num = find(capsule, Ray(line[0], dir), t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(capsule, line[0]))
        t[0] = 0;
    if (test(capsule, line[1]))
        t[1] = len;

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Capsule& capsule, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real t[2];
    int num = find(capsule, ray, t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(capsule, ray.origin))
        t[0] = 0;

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    return true;
}

template<class Real>
int Intersect_<Real>::find(const Capsule& capsule, const Ray& ray, Real t[2])
{
    // Create a coordinate system for the capsule.  In this system, the
    // capsule segment center C is the origin and the capsule axis direction
    // W is the z-axis.  U and V are the other coordinate axis directions.
    // If P = x*U+y*V+z*W, the cylinder containing the capsule wall is
    // x^2 + y^2 = r^2, where r is the capsule radius.  The finite cylinder
    // that makes up the capsule minus its hemispherical end caps has z-values
    // |z| <= e, where e is the extent of the capsule segment.  The top
    // hemisphere cap is x^2+y^2+(z-e)^2 = r^2 for z >= e, and the bottom
    // hemisphere cap is x^2+y^2+(z+e)^2 = r^2 for z <= -e.
    Real extent;
    Vec3 U, V, W = capsule.line.dir(extent);
    tie(U,V) = W.orthonormalBasis();
    extent /= 2;
    Real rSqr = capsule.radius*capsule.radius;

    // Convert incoming line origin to capsule coordinates.
    Vec3 diff = ray.origin - capsule.line.center();
    Vec3 P(U.dot(diff), V.dot(diff), W.dot(diff));

    // Get the z-value, in capsule coordinates, of the incoming line's
    // unit-length direction.
    Real dz = W.dot(ray.dir);
    if (Alge::abs(dz) >= 1 - Real_::zeroTol)
    {
        // The line is parallel to the capsule axis.  Determine whether the
        // line intersects the capsule hemispheres.
        Real radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0)
        {
            // Line outside the cylinder of the capsule, no intersection.
            return 0;
        }

        // line intersects the hemispherical caps
        Real zOffset = Alge::sqrt(radialSqrDist) + extent;
        if (dz > 0)
        {
            t[0] = -P.z - zOffset;
            t[1] = -P.z + zOffset;
        }
        else
        {
            t[0] = P.z - zOffset;
            t[1] = P.z + zOffset;
        }
        return 2;
    }

    // Convert incoming line unit-length direction to capsule coordinates.
    Vec3 D(U.dot(ray.dir), V.dot(ray.dir), dz);

    // Test intersection of line P+t*D with infinite cylinder x^2+y^2 = r^2.
    // This reduces to computing the roots of a quadratic equation.  If
    // P = (px,py,pz) and D = (dx,dy,dz), then the quadratic equation is
    //   (dx^2+dy^2)*t^2 + 2*(px*dx+py*dy)*t + (px^2+py^2-r^2) = 0
    Real a0 = P.x*P.x + P.y*P.y - rSqr;
    Real a1 = P.x*D.x + P.y*D.y;
    Real a2 = D.x*D.x + D.y*D.y;
    Real discr = a1*a1 - a0*a2;
    if (discr < 0)
    {
        // Line does not intersect infinite cylinder.
        return 0;
    }

    Real root, inv, tValue, zValue;
    int quantity = 0;
    if (discr > Real_::zeroTol)
    {
        // Line intersects infinite cylinder in two places.
        root = Alge::sqrt(discr);
        inv = 1/a2;
        tValue = (-a1 - root)*inv;
        zValue = P.z + tValue*D.z;
        if (Alge::abs(zValue) <= extent)
        {
            t[quantity++] = tValue;
        }

        tValue = (-a1 + root)*inv;
        zValue = P.z + tValue*D.z;
        if (Alge::abs(zValue) <= extent)
        {
            t[quantity++] = tValue;
        }

        if (quantity == 2)
        {
            // Line intersects capsule wall in two places.
            return 2;
        }
    }
    else
    {
        // Line is tangent to infinite cylinder.
        tValue = -a1/a2;
        zValue = P.z + tValue*D.z;
        if (Alge::abs(zValue) <= extent)
        {
            t[0] = tValue;
            return 1;
        }
    }

    // Test intersection with bottom hemisphere.  The quadratic equation is
    //   t^2 + 2*(px*dx+py*dy+(pz+e)*dz)*t + (px^2+py^2+(pz+e)^2-r^2) = 0
    // Use the fact that currently a1 = px*dx+py*dy and a0 = px^2+py^2-r^2.
    // The leading coefficient is a2 = 1, so no need to include in the
    // construction.
    Real PZpE = P.z + extent;
    a1 += PZpE*D.z;
    a0 += PZpE*PZpE;
    discr = a1*a1 - a0;
    if (discr > Real_::zeroTol)
    {
        root = Alge::sqrt(discr);
        tValue = -a1 - root;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }

        tValue = -a1 + root;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }
    else if (Alge::abs(discr) <= Real_::zeroTol)
    {
        tValue = -a1;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }

    // Test intersection with top hemisphere.  The quadratic equation is
    //   t^2 + 2*(px*dx+py*dy+(pz-e)*dz)*t + (px^2+py^2+(pz-e)^2-r^2) = 0
    // Use the fact that currently a1 = px*dx+py*dy+(pz+e)*dz and
    // a0 = px^2+py^2+(pz+e)^2-r^2.  The leading coefficient is a2 = 1, so
    // no need to include in the construction.
    a1 -= 2*extent*D.z;
    a0 -= 4*extent*P.z;
    discr = a1*a1 - a0;
    if (discr > Real_::zeroTol)
    {
        root = Alge::sqrt(discr);
        tValue = -a1 - root;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }

        tValue = -a1 + root;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }
    else if (Alge::abs(discr) <= Real_::zeroTol)
    {
        tValue = -a1;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent)
        {
            t[quantity++] = tValue;
            if (quantity == 2)
            {
                if (t[0] > t[1])
                {
                    Real save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }

    return quantity;
}


template<class Real>
bool Intersect_<Real>::find(const Cone& cone, const Sphere& sphere, option<Vec3&> intPoint)
{
    Real sinAngle = Trig::sin(cone.angle);
    Real cosAngle = 1 - sinAngle;

    // Test whether cone vertex is in sphere.
    Vec3 diff = sphere.center - cone.vertex;
    Real rSqr = sphere.radius*sphere.radius;
    Real lenSqr = diff.lengthSqr();
    if (lenSqr <= rSqr)
    {
        if (intPoint) intPoint = cone.vertex;
        return true;
    }

    // Test whether sphere center is in cone.
    Real dot = diff.dot(cone.axis);
    Real dotSqr = dot*dot;
    Real cosSqr = cosAngle*cosAngle;
    if (dotSqr >= lenSqr*cosSqr && dot > 0)
    {
        // Sphere center is inside cone, so sphere and infinite cone intersect.
        Vec3 point = sphere.center + (cone.vertex-sphere.center).normalize()*sphere.radius;

        // Check whether closest point to cone vertex is within finite cone
        if (Geom::side(cone.plane(1), point) == Plane::Side::neg)
        {
            if (intPoint) intPoint = point;
            return true;
        }
    }

    // Sphere center is outside cone.  Problem now reduces to looking for
    // an intersection between circle and ray in the plane containing
    // cone vertex and spanned by cone axis and vector from vertex to
    // sphere center.

    // Ray is t*D+V (t >= 0) where |D| = 1 and dot(A,D) = cos(angle).
    // Also, D = e*A+f*(C-V).  Plugging ray equation into sphere equation
    // yields R^2 = |t*D+V-C|^2, so the quadratic for intersections is
    // t^2 - 2*dot(D,C-V)*t + |C-V|^2 - R^2 = 0.  An intersection occurs
    // if and only if the discriminant is nonnegative.  This test becomes
    //
    //     dot(D,C-V)^2 >= dot(C-V,C-V) - R^2
    //
    // Note that if the right-hand side is nonpositive, then the inequality
    // is true (the sphere contains V).  I have already ruled this out in
    // the first block of code in this function.

    Real uLen = Alge::sqrt(Alge::abs(lenSqr-dotSqr));
    Real test = cosAngle*dot + sinAngle*uLen;
    Real discr = test*test - lenSqr + rSqr;

    if (discr >= 0 && test >= 0)
    {
        //Sphere and infinite cone intersect, get closest point to cone vertex
        Real t = test - Alge::sqrt(discr);
        Vec3 B = diff - dot*cone.axis;
        Real tmp = sinAngle/uLen;

        Vec3 point = t*(cosAngle*cone.axis + tmp*B);
        // Check whether closest point to cone vertex is within finite cone
        if (Geom::side(cone.plane(1), point) == Plane::Side::neg)
        {
            if (intPoint) intPoint = point;
            return true;
        }
    }

    return false;
}

template<class Real>
bool Intersect_<Real>::find(const Cone& cone, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    if (!find(cone, Ray(line[0], dir), t))
        return false;

    if (test(cone, line[0]))
        t[0] = 0;
    if (test(cone, line[1]))
        t[1] = len;

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Cone& cone, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real t[2];
    if (!find(cone, ray, t))
        return false;

    if (test(cone, ray.origin))
        t[0] = 0;

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Cone& cone, const Ray& ray, Real t[2])
{
    const Vec3& v1 = ray.origin;
    const Vec3& dir = ray.dir;
    const Real cos_angle = Trig::cos(cone.angle);
    const Vec3& pos = cone.vertex;
    const Vec3& axis = cone.axis;

    Vec3 pos_v1 = v1-pos;
    Vec3 vInt[2];

    //Solve quadratic equation
    Real cos_angle_square = Alge::sqr(cos_angle);
    Real dir_dot_axis = dir.dot(axis);
    Real dir_dot_dir = dir.dot(dir);
    Real pos_v1_dot_axis = pos_v1.dot(axis);
    Real pos_v1_dot_dir = pos_v1.dot(dir);
    Real pos_v1_dot_pos_v1 = pos_v1.dot(pos_v1);

    Real a = dir_dot_axis*dir_dot_axis - cos_angle_square*dir_dot_dir;
    Real b = dir_dot_axis*pos_v1_dot_axis - cos_angle_square*pos_v1_dot_dir;
    Real c = pos_v1_dot_axis*pos_v1_dot_axis - cos_angle_square*pos_v1_dot_pos_v1;

    Real root = Alge::sqr(b) - a*c;

    if (!Alge::isNearZero(a))
    {
        if (root < 0)
            return false;
        if (Alge::isNearZero(root))
        {
            t[0] = -b/a;
            vInt[0] = v1 + dir*t[0];
            if ((vInt[0]-pos).dot(axis) < 0)
                return false;

            t[1] = t[0];
            vInt[1] = vInt[0];
        }
        else
        {
            Real sqrtRoot = Alge::sqrt(root);

            t[0] = (-b + sqrtRoot)/a;
            vInt[0] = v1 + dir*t[0];
            if ((vInt[0]-pos).dot(axis) < 0)
                return false;

            t[1] = (-b - sqrtRoot)/a;
            vInt[1] = v1 + dir*t[1];
            if ((vInt[1]-pos).dot(axis) < 0)
                return false;
        }
    }
    else
    {
        if (Alge::isNearZero(b))
            t[0] = 0;
        else
            t[0] = -a/(2*b);

        vInt[0] = v1 + dir*t[0];
        if ((vInt[0]-pos).dot(axis) < 0)
            return false;

        t[1] = t[0];
        vInt[1] = vInt[0];
    }

    //Sort by distance along line
    if (t[0] > t[1])
    {
        std::swap(t[0], t[1]);
        std::swap(vInt[0], vInt[1]);
    }

    //Points are in cone angle span but we must must check to see if they also lie between vertex and base plane
    //Normal of base plane points away from vertex (positive = outside)
    Plane coneBase = cone.plane(1);
    typename Plane::Side side[2];
    side[0] = Geom::side(coneBase, vInt[0]);
    side[1] = Geom::side(coneBase, vInt[1]);

    if (side[0] == Plane::Side::pos && side[1] == Plane::Side::pos)
        //Both points are outside, no intersection
        return false;

    //If one point lies outside, clip with base plane
    if (side[0] == Plane::Side::pos)
        find(coneBase, Line(vInt[0], vInt[1]), vInt[0]);
    else if (side[1] == Plane::Side::pos)
        find(coneBase, Line(vInt[0], vInt[1]), vInt[1]);

    //Convert from point back to t
    t[0] = (vInt[0]-v1).dot(dir);
    t[1] = (vInt[1]-v1).dot(dir);

    return true;
}


template<class Real>
bool Intersect_<Real>::find(const Cylinder& cylinder, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    int num = find(cylinder, Ray(line[0], dir), t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(cylinder, line[0]))
        t[0] = 0;
    if (test(cylinder, line[1]))
        t[1] = len;

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Cylinder& cylinder, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real t[2];
    int num = find(cylinder, ray, t);
    if (num == 0)
        return false;
    if (num == 1)
        t[1] = t[0];
    if (test(cylinder, ray.origin))
        t[0] = 0;

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    return true;
}

template<class Real>
int Intersect_<Real>::find(const Cylinder& cylinder, const Ray& ray, Real t[2])
{
    // Create a coordinate system for the cylinder.  In this system, the
    // cylinder segment center C is the origin and the cylinder axis direction
    // W is the z-axis.  U and V are the other coordinate axis directions.
    // If P = x*U+y*V+z*W, the cylinder is x^2 + y^2 = r^2, where r is the
    // cylinder radius.  The end caps are |z| = h/2, where h is the cylinder
    // height.
    Vec3 U, V, W = cylinder.axis;
    tie(U,V) = W.orthonormalBasis();
    Real halfHeight = 0.5*cylinder.height;
    Real rSqr = cylinder.radius*cylinder.radius;

    // convert incoming line origin to cylinder coordinates
    Vec3 diff = ray.origin - cylinder.center;
    Vec3 P(U.dot(diff), V.dot(diff), W.dot(diff));

    // Get the z-value, in cylinder coordinates, of the incoming line's
    // unit-length direction.
    Real dz = W.dot(ray.dir);

    if (Alge::abs(dz) >= 1 - Real_::zeroTol)
    {
        // The line is parallel to the cylinder axis.  Determine if the line
        // intersects the cylinder end disks.
        Real radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0)
        {
            // Line outside the cylinder, no intersection.
            return 0;
        }

        // Line intersects the cylinder end disks.
        if (dz > 0)
        {
            t[0] = -P.z - halfHeight;
            t[1] = -P.z + halfHeight;
        }
        else
        {
            t[0] = P.z - halfHeight;
            t[1] = P.z + halfHeight;
        }
        return 2;
    }

    // convert incoming line unit-length direction to cylinder coordinates
    Vec3 D(U.dot(ray.dir),V.dot(ray.dir),dz);

    Real a0, a1, a2, discr, root, inv, tValue;

    if (Alge::abs(D.z) <= Real_::zeroTol)
    {
        // The line is perpendicular to the cylinder axis.
        if (Alge::abs(P.z) > halfHeight)
        {
            // Line is outside the planes of the cylinder end disks.
            return 0;
        }

        // Test intersection of line P+t*D with infinite cylinder
        // x^2+y^2 = r^2.  This reduces to computing the roots of a
        // quadratic equation.  If P = (px,py,pz) and D = (dx,dy,dz),
        // then the quadratic equation is
        //   (dx^2+dy^2)*t^2 + 2*(px*dx+py*dy)*t + (px^2+py^2-r^2) = 0
        a0 = P.x*P.x + P.y*P.y - rSqr;
        a1 = P.x*D.x + P.y*D.y;
        a2 = D.x*D.x + D.y*D.y;
        discr = a1*a1 - a0*a2;
        if (discr < 0)
        {
            // Line does not intersect cylinder.
            return 0;
        }
        else if (discr > Real_::zeroTol)
        {
            // Line intersects cylinder in two places.
            root = Alge::sqrt(discr);
            inv = 1/a2;
            t[0] = (-a1 - root)*inv;
            t[1] = (-a1 + root)*inv;
            return 2;
        }
        else
        {
            // Line is tangent to the cylinder.
            t[0] = -a1/a2;
            return 1;
        }
    }

    // Test plane intersections first.
    int quantity = 0;
    inv = 1 / D.z;

    Real t0 = (-halfHeight - P.z)*inv;
    Real xTmp = P.x + t0*D.x;
    Real yTmp = P.y + t0*D.y;
    if (xTmp*xTmp + yTmp*yTmp <= rSqr)
    {
        // Planar intersection inside the top cylinder end disk.
        t[quantity++] = t0;
    }

    Real t1 = (+halfHeight - P.z)*inv;
    xTmp = P.x + t1*D.x;
    yTmp = P.y + t1*D.y;
    if (xTmp*xTmp + yTmp*yTmp <= rSqr)
    {
        // Planar intersection inside the bottom cylinder end disk.
        t[quantity++] = t1;
    }

    if (quantity == 2)
    {
        // Line intersects both top and bottom cylinder end disks.
        if (t[0] > t[1])
        {
            Real save = t[0];
            t[0] = t[1];
            t[1] = save;
        }
        return 2;
    }

    // If quantity == 1, then the line must intersect cylinder wall in a
    // single point somewhere between the end disks.  This case is detected
    // in the following code that tests for intersection between line and
    // cylinder wall.
    a0 = P.x*P.x + P.y*P.y - rSqr;
    a1 = P.x*D.x + P.y*D.y;
    a2 = D.x*D.x + D.y*D.y;
    discr = a1*a1 - a0*a2;
    if (discr < 0)
    {
        // Line does not intersect cylinder wall.
        assert(quantity == 0, "Unexpected condition\n");
        return 0;
    }
    else if (discr > Real_::zeroTol)
    {
        root = Alge::sqrt(discr);
        inv = 1/a2;
        tValue = (-a1 - root)*inv;
        if (t0 <= t1)
        {
            if (t0 <= tValue && tValue <= t1)
            {
                t[quantity++] = tValue;
            }
        }
        else
        {
            if (t1 <= tValue && tValue <= t0)
            {
                t[quantity++] = tValue;
            }
        }

        if (quantity == 2)
        {
            // Line intersects one of the cylinder end disks and once on the
            // cylinder wall.
            if (t[0] > t[1])
            {
                Real save = t[0];
                t[0] = t[1];
                t[1] = save;
            }
            return 2;
        }

        tValue = (-a1 + root)*inv;
        if (t0 <= t1)
        {
            if (t0 <= tValue && tValue <= t1)
            {
                t[quantity++] = tValue;
            }
        }
        else
        {
            if (t1 <= tValue && tValue <= t0)
            {
                t[quantity++] = tValue;
            }
        }
    }
    else
    {
        tValue = -a1/a2;
        if (t0 <= t1)
        {
            if (t0 <= tValue && tValue <= t1)
            {
                t[quantity++] = tValue;
            }
        }
        else
        {
            if (t1 <= tValue && tValue <= t0)
            {
                t[quantity++] = tValue;
            }
        }
    }

    if (quantity == 2)
    {
        if (t[0] > t[1])
        {
            Real save = t[0];
            t[0] = t[1];
            t[1] = save;
        }
    }

    return quantity;
}

template<class Real>
bool Intersect_<Real>::find(const Box& box1, const Box& box2, option<Box&> intBox)
{
    if (!test(box1, box2))
        return false;

    Vec3 min;
    Vec3 max;

    const Vec3& min1 = box1.min;
    const Vec3& max1 = box1.max;
    const Vec3& min2 = box2.min;
    const Vec3& max2 = box2.max;

    for (int i = 0; i < 3; ++i)
    {
        if (max1[i] <= max2[i])
            max[i] = max1[i];
        else
            max[i] = max2[i];

        if (min1[i] <= min2[i])
            min[i] = min2[i];
        else
            min[i] = min1[i];
    }

    if (intBox) intBox->fromBounds(min, max);
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Box& box, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2, option<Vec3&> intNormal1, option<Vec3&> intNormal2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    Vec3 norm[2];

    if (!find(box, Ray(line[0], dir), t, norm))
        return false;

    if (test(box, line[0]))
    {
        t[0] = 0;
        norm[0] = Vec3::zero;
    }
    if (test(box, line[1]))
    {
        t[1] = len;
        norm[1] = Vec3::zero;
    }

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    if (intNormal1) intNormal1 = norm[0];
    if (intNormal2) intNormal2 = norm[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Box& box, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2, option<Vec3&> intNormal1, option<Vec3&> intNormal2)
{
    Real t[2];
    Vec3 norm[2];

    if (!find(box, ray, t, norm))
        return false;

    if (test(box, ray.origin))
    {
        t[0] = 0;
        norm[0] = -ray.dir;
    }

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    if (intNormal1) intNormal1 = norm[0];
    if (intNormal2) intNormal2 = norm[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Box& box, const Ray& ray, Real t[2], Vec3 norm[2])
{
    Vec3 vInt;
    Real tv;
    int num = 0;
    const Vec3& v1 = ray.origin;
    const Vec3& dir = ray.dir;
    const Vec3& ll = box.min;
    const Vec3& ur = box.max;
    Vec3 ll_test = ll.elemSub(Real_::zeroTol); //Accommodate for error
    Vec3 ur_test = ur.elemAdd(Real_::zeroTol);

    //Intersect X normal planes
    if (!Alge::isNearZero(dir.x))
    {
        if (num < 2)
        {
            tv = (ll.x - v1.x) / dir.x;
            vInt.y = v1.y + tv*dir.y;
            vInt.z = v1.z + tv*dir.z;
            if (vInt.y <= ur_test.y && vInt.y >= ll_test.y && vInt.z <= ur_test.z && vInt.z >= ll_test.z)
            {
                t[num] = tv;
                norm[num] = -Vec3::axisX;
                num++;
            }
        }

        if (num < 2)
        {
            tv = (ur.x - v1.x) / dir.x;
            vInt.y = v1.y + tv*dir.y;
            vInt.z = v1.z + tv*dir.z;
            if (vInt.y <= ur_test.y && vInt.y >= ll_test.y && vInt.z <= ur_test.z && vInt.z >= ll_test.z)
            {
                t[num] = tv;
                norm[num] = Vec3::axisX;
                num++;
            }
        }
    }
    //Intersect Y normal planes
    if (!Alge::isNearZero(dir.y))
    {
        if (num < 2)
        {
            tv = (ll.y - v1.y) / dir.y;
            vInt.x = v1.x + tv*dir.x;
            vInt.z = v1.z + tv*dir.z;
            if (vInt.x <= ur_test.x && vInt.x >= ll_test.x && vInt.z <= ur_test.z && vInt.z >= ll_test.z)
            {
                t[num] = tv;
                norm[num] = -Vec3::axisY;
                num++;
            }
        }

        if (num < 2)
        {
            tv = (ur.y - v1.y) / dir.y;
            vInt.x = v1.x + tv*dir.x;
            vInt.z = v1.z + tv*dir.z;
            if (vInt.x <= ur_test.x && vInt.x >= ll_test.x && vInt.z <= ur_test.z && vInt.z >= ll_test.z)
            {
                t[num] = tv;
                norm[num] = Vec3::axisY;
                num++;
            }
        }
    }
    //Intersect Z normal planes
    if (!Alge::isNearZero(dir.z))
    {
        if (num < 2)
        {
            tv = (ll.z - v1.z) / dir.z;
            vInt.x = v1.x + tv*dir.x;
            vInt.y = v1.y + tv*dir.y;
            if (vInt.x <= ur_test.x && vInt.x >= ll_test.x && vInt.y <= ur_test.y && vInt.y >= ll_test.y)
            {
                t[num] = tv;
                norm[num] = -Vec3::axisZ;
                num++;
            }
        }

        if (num < 2)
        {
            tv = (ur.z - v1.z) / dir.z;
            vInt.x = v1.x + tv*dir.x;
            vInt.y = v1.y + tv*dir.y;
            if (vInt.x <= ur_test.x && vInt.x >= ll_test.x && vInt.y <= ur_test.y && vInt.y >= ll_test.y)
            {
                t[num] = tv;
                norm[num] = Vec3::axisZ;
                num++;
            }
        }
    }

    if (num < 2)
        return false;

    if (t[1] < t[0])
    {
        std::swap(t[0], t[1]);
        std::swap(norm[0], norm[1]);
    }

    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Sphere& sphere, const Line& line, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real len;
    Vec3 dir = line.dir(len);

    Real t[2];
    if (!find(sphere, Ray(line[0], dir), t))
        return false;

    if (test(sphere, line[0]))
        t[0] = 0;
    if (test(sphere, line[1]))
        t[1] = len;

    if (t[0] < 0 || t[0] > len || t[1] < 0 || t[1] > len)
        return false;

    if (intPoint1) intPoint1 = line[0] + dir*t[0];
    if (intPoint2) intPoint2 = line[0] + dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Sphere& sphere, const Ray& ray, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    Real t[2];
    if (!find(sphere, ray, t))
        return false;

    if (test(sphere, ray.origin))
        t[0] = 0;

    if (t[0] < 0 || t[1] < 0)
        return false;

    if (intPoint1) intPoint1 = ray.origin + ray.dir*t[0];
    if (intPoint2) intPoint2 = ray.origin + ray.dir*t[1];
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Sphere& sphere, const Ray& ray, Real t[2])
{
    const Vec3& v1 = ray.origin;
    const Vec3& dir = ray.dir;
    const Vec3& o = sphere.center;
    const Real r = sphere.radius;

    //Solve quadratic equation
    Real a = Alge::sqr(dir.x) + Alge::sqr(dir.y) + Alge::sqr(dir.z); 
    Real b = 2*((dir.x)*(v1.x - o.x) + (dir.y)*(v1.y - o.y) + (dir.z)*(v1.z - o.z)); 
    Real c = Alge::sqr(o.x) + Alge::sqr(o.y) + Alge::sqr(o.z) + Alge::sqr(v1.x) + Alge::sqr(v1.y) + Alge::sqr(v1.z) 
                - 2*(o.x*v1.x + o.y*v1.y + o.z*v1.z) - Alge::sqr(r); 

    Real root = Alge::sqr(b) - 4*a*c;
    if (root < 0)
        return false;

    if (Alge::isNearZero(root))
    {
        t[0] = -b/(2*a);
        t[1] = t[0];
    }
    else
    {
        Real sqrtRoot = Alge::sqrt(root);
        t[0] = (-b + sqrtRoot)/(2*a);
        t[1] = (-b - sqrtRoot)/(2*a);
    }

    if (t[1] < t[0])
        std::swap(t[0], t[1]);

    return true;
}


//=======================================================================================================================


template<class Real>
bool Intersect_<Real>::find(const Triangle& tri1, const Triangle& tri2, option<Vec3&> intPoint1, option<Vec3&> intPoint2)
{
    // get the plane of triangle0
    Plane kPlane0(tri1.v0,tri1.v1,tri1.v2);

    // Compute the signed distances of triangle1 vertices to plane0.  Use
    // an ZeroTol-thick plane test.
    int pos1, neg1, zero1, aiSign1[3];
    Real afDist1[3];
    TriangleInt::trianglePlaneRelations(tri2,kPlane0,afDist1,aiSign1,pos1,neg1,zero1);

    if (pos1 == 3 || neg1 == 3)
    {
        // triangle1 is fully on one side of plane0
        return false;
    }

    if (zero1 == 3)
    {
        return false;
    }

    // get the plane of triangle1
    Plane kPlane1(tri2.v0,tri2.v1,tri2.v2);

    // Compute the signed distances of triangle0 vertices to plane1.  Use
    // an ZeroTol-thick plane test.
    int pos0, neg0, zero0, aiSign0[3];
    Real afDist0[3];
    TriangleInt::trianglePlaneRelations(tri1,kPlane1,afDist0,aiSign0,pos0,neg0,zero0);

    if (pos0 == 3 || neg0 == 3)
    {
        // triangle0 is fully on one side of plane1
        return false;
    }

    if (zero0 == 3)
    {
        return false;
    }

    // compute the line of intersection
    Real dot = kPlane0.normal.dot(kPlane1.normal);
    Real invDet = 1/(1 - dot*dot);
    Real c0 = (kPlane0.dist - dot*kPlane1.dist)*invDet;
    Real c1 = (kPlane1.dist - dot*kPlane0.dist)*invDet;
    Ray kLine(c0*kPlane0.normal + c1*kPlane1.normal,
        kPlane0.normal.crossUnit(kPlane1.normal));

    // compute the intervals of intersection of triangles and line
    Real afT0[2], afT1[2];
    TriangleInt::interval(tri1,kLine,afDist0,aiSign0,afT0);
    TriangleInt::interval(tri2,kLine,afDist1,aiSign1,afT1);

    // compute the intersection of intervals
    Real afOverlap[2];

    if (afT0[1] < afT1[0] || afT0[0] > afT1[1])
    {
        return false;
    }
    else if (afT0[1] > afT1[0])
    {
        if (afT0[0] < afT1[1])
        {
            afOverlap[0] = (afT0[0] < afT1[0] ? afT1[0] : afT0[0]);
            afOverlap[1] = (afT0[1] > afT1[1] ? afT1[1] : afT0[1]);
        }
        else  // afT0[0] == afT1[1]
        {
            afOverlap[0] = afT0[0];
            afOverlap[1] = afOverlap[0];
        }
    }
    else  // afT0[1] == afT1[0]
    {
        afOverlap[0] = afT0[1];
        afOverlap[1] = afOverlap[0];
    }

    if (intPoint1) intPoint1 = kLine.origin + afOverlap[0]*kLine.dir;
    if (intPoint2) intPoint2 = kLine.origin + afOverlap[1]*kLine.dir;
    return true;
}

template<class Real>
void Intersect_<Real>::TriangleInt::
    projectOntoAxis(const Triangle& tri, const Vec3& axis, Real& rfMin, Real& rfMax)
{
    Real dot0 = axis.dot(tri.v0);
    Real dot1 = axis.dot(tri.v1);
    Real dot2 = axis.dot(tri.v2);

    rfMin = dot0;
    rfMax = rfMin;

    if (dot1 < rfMin)
        rfMin = dot1;
    else if (dot1 > rfMax)
        rfMax = dot1;

    if (dot2 < rfMin)
        rfMin = dot2;
    else if (dot2 > rfMax)
        rfMax = dot2;
}

template<class Real>
void Intersect_<Real>::TriangleInt::
    trianglePlaneRelations(const Triangle& triangle, const Plane& plane, Real distances[3], int signs[3], int& positive, int& negative, int& zero)
{
    positive = 0;
    negative = 0;
    zero = 0;
    for (int i = 0; i < 3; ++i)
    {
        distances[i] = distance(plane, triangle[i]);
        if (distances[i] > Real_::zeroTol)
        {
            signs[i] = 1;
            positive++;
        }
        else if (distances[i] < -Real_::zeroTol)
        {
            signs[i] = -1;
            negative++;
        }
        else
        {
            distances[i] = 0;
            signs[i] = 0;
            zero++;
        }
    }
}

template<class Real>
void Intersect_<Real>::TriangleInt::
    interval(const Triangle& triangle, const Ray& line, const Real distances[3], const int signs[3], Real params[2])
{
    // project triangle onto line
    Real afProj[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        Vec3 kDiff = triangle[i] - line.origin;
        afProj[i] = line.dir.dot(kDiff);
    }

    // compute transverse intersections of triangle edges with line
    Real numer, denom;
    int i0, i1, i2;
    int quantity = 0;
    for (i0 = 2, i1 = 0; i1 < 3; i0 = ++i1)
    {
        if (signs[i0]*signs[i1] < 0)
        {
            assert(quantity < 2);
            numer = distances[i0]*afProj[i1] - distances[i1]*afProj[i0];
            denom = distances[i0] - distances[i1];
            params[quantity++] = numer/denom;
        }
    }

    // check for grazing contact
    if (quantity < 2)
    {
        for (i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2, ++i2)
        {
            if (signs[i2] == 0)
            {
                assert(quantity < 2);
                params[quantity++] = afProj[i2];
            }
        }
    }

    // sort
    assert(quantity == 1 || quantity == 2);
    if (quantity == 2)
    {
        if (params[0] > params[1])
        {
            Real save = params[0];
            params[0] = params[1];
            params[1] = save;
        }
    }
    else
    {
        params[1] = params[0];
    }
}


//=======================================================================================================================


template<class Real>
bool Intersect_<Real>::find(const Triangle& tri, const Plane& plane, option<int&> pointCount, Vec3 intPoints[3])
{
    enum
    {
        IT_EMPTY,
        IT_POINT,
        IT_SEGMENT,
        IT_POLYGON
    };

    int intersectionType;
    int quantity;
    Vec3 akPoint[3];

    // Compute the signed distances from the vertices to the plane.
    Real zero = 0;
    Real afSD[3];
    for (int i = 0; i < 3; ++i)
    {
        afSD[i] = distance(plane, tri[i]);
        if (Alge::isNearZero(afSD[i]))
        {
            afSD[i] = zero;
        }
    };

    const Vec3& rkV0 = tri.v0;
    const Vec3& rkV1 = tri.v1;
    const Vec3& rkV2 = tri.v2;

    if (afSD[0] > zero)
    {
        if (afSD[1] > zero)
        {
            if (afSD[2] > zero)
            {
                // ppp
                quantity = 0;
                intersectionType = IT_EMPTY;
            }
            else if (afSD[2] < zero)
            {
                // ppm
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                akPoint[1] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // ppz
                quantity = 1;
                akPoint[0] = rkV2;
                intersectionType = IT_POINT;
            }
        }
        else if (afSD[1] < zero)
        {
            if (afSD[2] > zero)
            {
                // pmp
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // pmm
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // pmz
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
        else
        {
            if (afSD[2] > zero)
            {
                // pzp
                quantity = 1;
                akPoint[0] = rkV1;
                intersectionType = IT_POINT;
            }
            else if (afSD[2] < zero)
            {
                // pzm
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                akPoint[1] = rkV1;
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // pzz
                quantity = 2;
                akPoint[0] = rkV1;
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
    }
    else if (afSD[0] < zero)
    {
        if (afSD[1] > zero)
        {
            if (afSD[2] > zero)
            {
                // mpp
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // mpm
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // mpz
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[1]))*(rkV1-rkV0);
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
        else if (afSD[1] < zero)
        {
            if (afSD[2] > zero)
            {
                // mmp
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                akPoint[1] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // mmm
                quantity = 0;
                intersectionType = IT_EMPTY;
            }
            else
            {
                // mmz
                quantity = 1;
                akPoint[0] = tri.v2;
                intersectionType = IT_POINT;
            }
        }
        else
        {
            if (afSD[2] > zero)
            {
                // mzp
                quantity = 2;
                akPoint[0] = rkV0+(afSD[0]/(afSD[0]-afSD[2]))*(rkV2-rkV0);
                akPoint[1] = rkV1;
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // mzm
                quantity = 1;
                akPoint[0] = rkV1;
                intersectionType = IT_POINT;
            }
            else
            {
                // mzz
                quantity = 2;
                akPoint[0] = rkV1;
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
    }
    else
    {
        if (afSD[1] > zero)
        {
            if (afSD[2] > zero)
            {
                // zpp
                quantity = 1;
                akPoint[0] = rkV0;
                intersectionType = IT_POINT;
            }
            else if (afSD[2] < zero)
            {
                // zpm
                quantity = 2;
                akPoint[0] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                akPoint[1] = rkV0;
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // zpz
                quantity = 2;
                akPoint[0] = rkV0;
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
        else if (afSD[1] < zero)
        {
            if (afSD[2] > zero)
            {
                // zmp
                quantity = 2;
                akPoint[0] = rkV1+(afSD[1]/(afSD[1]-afSD[2]))*(rkV2-rkV1);
                akPoint[1] = rkV0;
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // zmm
                quantity = 1;
                akPoint[0] = rkV0;
                intersectionType = IT_POINT;
            }
            else
            {
                // zmz
                quantity = 2;
                akPoint[0] = rkV0;
                akPoint[1] = rkV2;
                intersectionType = IT_SEGMENT;
            }
        }
        else
        {
            if (afSD[2] > zero)
            {
                // zzp
                quantity = 2;
                akPoint[0] = rkV0;
                akPoint[1] = rkV1;
                intersectionType = IT_SEGMENT;
            }
            else if (afSD[2] < zero)
            {
                // zzm
                quantity = 2;
                akPoint[0] = rkV0;
                akPoint[1] = rkV1;
                intersectionType = IT_SEGMENT;
            }
            else
            {
                // zzz
                quantity = 3;
                akPoint[0] = rkV0;
                akPoint[1] = rkV1;
                akPoint[2] = rkV2;
                intersectionType = IT_POLYGON;
            }
        }
    }

    if (pointCount) pointCount = quantity;
    if (intPoints)
        for (int i = 0; i < quantity; ++i)
            intPoints[i] = akPoint[i];

    return intersectionType != IT_EMPTY;
}

template<class Real>
bool Intersect_<Real>::find(const Triangle& tri, const Line& line, option<Vec3&> intPoint)
{
    Real lineExtent;
    Vec3 lineDir = line.dir(lineExtent);
    lineExtent *= 0.5;
    Vec3 lineCenter = line.center();

    // Compute the offset origin, edges, and normal.
    Vec3 diff = lineCenter - tri[0];
    Vec3 edge1 = tri[1] - tri[0];
    Vec3 edge2 = tri[2] - tri[0];
    Vec3 normal = edge1.cross(edge2);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = segment direction,
    // E1 = edge1, E2 = edge2, N = cross(E1,E2)) by
    //   |dot(D,N)|*b1 = sign(dot(D,N))*dot(D,cross(Q,E2))
    //   |dot(D,N)|*b2 = sign(dot(D,N))*dot(D,cross(E1,Q))
    //   |dot(D,N)|*t = -sign(dot(D,N))*dot(Q,N)
    Real DdN = lineDir.dot(normal);
    Real sign;
    if (DdN > Real_::zeroTol)
    {
        sign = 1;
    }
    else if (DdN < -Real_::zeroTol)
    {
        sign = -1;
        DdN = -DdN;
    }
    else
    {
        // Segment and triangle are parallel, call it a "no intersection"
        // even if the segment does intersect.
        return false;
    }

    Real DdQxE2 = sign*lineDir.dot(diff.cross(edge2));
    if (DdQxE2 >= 0)
    {
        Real DdE1xQ = sign*lineDir.dot(edge1.cross(diff));
        if (DdE1xQ >= 0)
        {
            if (DdQxE2 + DdE1xQ <= DdN)
            {
                // Line intersects triangle, check if segment does.
                Real QdN = -sign*diff.dot(normal);
                Real extDdN = lineExtent*DdN;
                if (-extDdN <= QdN && QdN <= extDdN)
                {
                    // Segment intersects triangle.
                    Real inv = 1/DdN;
                    if (intPoint) intPoint = lineCenter + lineDir*(QdN*inv);
                    return true;
                }
                // else: |t| > extent, no intersection
            }
            // else: b1+b2 > 1, no intersection
        }
        // else: b2 < 0, no intersection
    }
    // else: b1 < 0, no intersection

    return false;
}

template<class Real>
bool Intersect_<Real>::find(const Triangle& tri, const Ray& ray, option<Vec3&> intPoint)
{
    // Compute the offset origin, edges, and normal.
    Vec3 diff = ray.origin - tri[0];
    Vec3 edge1 = tri[1] - tri[0];
    Vec3 edge2 = tri[2] - tri[0];
    Vec3 normal = edge1.cross(edge2);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
    // E1 = kEdge1, E2 = kEdge2, N = cross(E1,E2)) by
    //   |dot(D,N)|*b1 = sign(dot(D,N))*dot(D,cross(Q,E2))
    //   |dot(D,N)|*b2 = sign(dot(D,N))*dot(D,cross(E1,Q))
    //   |dot(D,N)|*t = -sign(dot(D,N))*dot(Q,N)
    Real DdN = ray.dir.dot(normal);
    Real sign;
    if (DdN > Real_::zeroTol)
    {
        sign = 1;
    }
    else if (DdN < -Real_::zeroTol)
    {
        sign = -1;
        DdN = -DdN;
    }
    else
    {
        // Ray and triangle are parallel, call it a "no intersection"
        // even if the ray does intersect.
        return false;
    }

    Real DdQxE2 = sign*ray.dir.dot(diff.cross(edge2));
    if (DdQxE2 >= 0)
    {
        Real DdE1xQ = sign*ray.dir.dot(edge1.cross(diff));
        if (DdE1xQ >= 0)
        {
            if (DdQxE2 + DdE1xQ <= DdN)
            {
                // Line intersects triangle, check if ray does.
                Real QdN = -sign*diff.dot(normal);
                if (QdN >= 0)
                {
                    // Ray intersects triangle.
                    Real inv = 1/DdN;
                    if (intPoint) intPoint = ray.origin + ray.dir*(QdN*inv);
                    return true;
                }
                // else: t < 0, no intersection
            }
            // else: b1+b2 > 1, no intersection
        }
        // else: b2 < 0, no intersection
    }
    // else: b1 < 0, no intersection

    return false;
}

template<class Real>
bool Intersect_<Real>::find(const Plane& plane1, const Plane& plane2, option<Ray&> intRay)
{
    // If N0 and N1 are parallel, either the planes are parallel and separated
    // or the same plane.  In both cases, 'false' is returned.  Otherwise,
    // the intersection line is
    //   L(t) = t*cross(N0,N1)/|cross(N0,N1)| + c0*N0 + c1*N1
    // for some coefficients c0 and c1 and for t any real number (the line
    // parameter).  Taking dot products with the normals,
    //   d0 = dot(N0,L) = c0*dot(N0,N0) + c1*dot(N0,N1) = c0 + c1*d
    //   d1 = dot(N1,L) = c0*dot(N0,N1) + c1*dot(N1,N1) = c0*d + c1
    // where d = dot(N0,N1).  These are two equations in two unknowns.  The
    // solution is
    //   c0 = (d0 - d*d1)/det
    //   c1 = (d1 - d*d0)/det
    // where det = 1 - d^2.

    Real dot = plane1.normal.dot(plane2.normal);
    if (Alge::abs(dot) >= 1 - Real_::zeroTol)
    {
        // The planes are parallel.  Check if they are coplanar.
        Real cDiff;
        if (dot >= 0)
        {
            // Normals are in same direction, need to look at c0-c1.
            cDiff = plane1.dist - plane2.dist;
        }
        else
        {
            // Normals are in opposite directions, need to look at c0+c1.
            cDiff = plane1.dist + plane2.dist;
        }

        if (Alge::abs(cDiff) < Real_::zeroTol)
        {
            // Planes are coplanar.
            if (intRay)
            {
                intRay->origin = plane1.normal*plane1.dist;
                intRay->dir = Vec3::zero;
            }
            return true;
        }

        // Planes are parallel, but distinct.
        return false;
    }

    Real invDet = 1/(1 - dot*dot);
    Real c0 = (plane1.dist - dot*plane2.dist)*invDet;
    Real c1 = (plane2.dist - dot*plane1.dist)*invDet;

    if (intRay)
    {
        intRay->origin = c0*plane1.normal + c1*plane2.normal;
        intRay->dir = plane1.normal.crossUnit(plane2.normal);
    }
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Plane& plane, const Line& line, option<Vec3&> intPoint)
{
    Vec3 dir = line.diff();
    Real dot = dir.dot(plane.normal);

    // line is parallel to plane?
    if(Alge::isNearZero(dot))
    {
        // line lies in plane?
        if (Alge::isNearZero(distance(plane, line.v0)))
        {
            if(intPoint) intPoint = line.v0;
            return true;
        }
        return false;
    }

    //distance from line to intersection point
    Real t = ( plane.dist - line.v0.dot(plane.normal) ) / dot;   
    if (t < 0) return false; //Intersection behind line
    if (t > 1) return false; //Intersection in front of line
         
    if(intPoint) intPoint = line.v0 + dir*t;
    return true;
}

template<class Real>
bool Intersect_<Real>::find(const Plane& plane, const Ray& ray, option<Vec3&> intPoint)
{
    Real dot = ray.dir.dot(plane.normal);

    // ray is parallel to plane?
    if(Alge::isNearZero(dot))
    {
        // ray lies in plane?
        if (Alge::isNearZero(distance(plane, ray.origin))) 
        {
            if(intPoint) intPoint = ray.origin;
            return true;
        }
        return false;
    }

    //distance from ray to intersection point
    Real t = ( plane.dist - ray.origin.dot(plane.normal) ) / dot;

    if (t < 0) return false; //Intersection behind ray
    if(intPoint) intPoint = ray.origin + ray.dir*t;
    return true;
}



template class Intersect_<Float>;
template class Intersect_<Double>;


}
