// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Line.h"

namespace honey
{

template<class Real> class Box_;

/// 3D triangle geometry class
template<class Real>
class Triangle_ : public Shape<Real>
{
    typedef Alge_<Real>     Alge;
    typedef Vec<3,Real>     Vec3;
    typedef Line_<Real>     Line;
    typedef Box_<Real>      Box;

public:
    /// No initialization
    Triangle_()                                                             {}

    /// Construct from 3 vertices
    Triangle_(const Vec3& v0, const Vec3& v1, const Vec3& v2)               : v0(v0), v1(v1), v2(v2) {}

    /// Construct from array of 3 vertices
    Triangle_(const Vec3* v) : v0(v[0]), v1(v[1]), v2(v[2])                 {}

    ~Triangle_()                                                            {}

    /// Number of vertices (constant)
    static const int vertexCount = 3;

    /// Access vertices
    Vec3& operator[](int i)                                                 { return vertex(i); }
    const Vec3& operator[](int i) const                                     { return vertex(i); }

    /// Get vertex
    const Vec3& vertex(int index) const                                     { return *(&v0+index); }
    Vec3& vertex(int index)                                                 { return *(&v0+index); }

    /// Number of edges (constant)
    static const int edgeCount = 3;

    /// Get edge
    Line edge(int index) const
    {
        switch (index)
        {
        case 0:
            return Line(v0, v1);
        case 1:
            return Line(v1, v2);
        case 2:
            return Line(v2, v0);
        default:
            error("Invalid edge");
            return Line(Vec3::zero, Vec3::zero);
        }
    }

    Vec3 normal() const                                                     { return (v1-v0).crossUnit(v2-v0); }

    /// Get barycenter / centroid
    Vec3 center() const                                                     { return (v0+v1+v2) / 3; }

    /// Get point on triangle given barycentric parameters
    Vec3 point(const Vec3& bary) const                                      { return bary.x*v0 + bary.y*v1 + bary.z*v2; }

    /// Get point on triangle given barycentric parameters
    /**
      * Input -> Output:
      *
      *     (0,0)    -> v0          (1,0) -> v1         (0,1) -> v2
      *     1-f-g==0 -> line v1,v2  (f,0) -> line v0,v1 (0,g) -> line v0,v2
      */ 
    Vec3 point(Real f, Real g) const                                        { return (1-f-g)*v0 + f*v1 + g*v2; }

    /// Get barycentric parameters given any point
    Vec3 param(const Vec3& point) const
    {
        Vec3 bary,  v10 = v1-v0,    v20 = v2-v0,    v30 = point-v0;
        tie(ignore, bary.y, bary.z) = Alge::solve(v10.dot(v10), v20.dot(v10), v10.dot(v20), v20.dot(v20), v30.dot(v10), v30.dot(v20));
        bary.x = 1 - bary.y - bary.z;
        return bary;
    }

    /// Get bounding box
    Box toBox() const;

    /// Transform operator
    friend Triangle_ operator*(const Matrix<4,4,Real>& mat, const Triangle_& tri)   { return Triangle_(mat*tri.v0, mat*tri.v1, mat*tri.v2); }
    friend Triangle_ operator*(const Transform_<Real>& tm, const Triangle_& tri)    { return Triangle_(tm*tri.v0, tm*tri.v1, tm*tri.v2); }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Triangle_& val)
    {
        return os << "{ v0" << val.v0 << ", v1" << val.v1 << ", v2" << val.v2 << " }";
    }

public:
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
};

typedef Triangle_<Real>    Triangle;
typedef Triangle_<Float>   Triangle_f;
typedef Triangle_<Double>  Triangle_d;

}
