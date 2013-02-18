// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Plane.h"
#include "Honey/Math/Geom/Box.h"

namespace honey
{

template<class Real> class Sphere_;

/// 3D oriented box geometry class.  An oriented box is a box with axes that can freely rotate, they are not restricted to be aligned to the coordinate axes.
template<class Real>
class OrientBox_ : public Shape<Real>
{
    typedef Vec<3,Real>     Vec3;
    typedef Line_<Real>     Line;
    typedef Plane_<Real>    Plane;
    typedef Sphere_<Real>   Sphere;
    typedef Box_<Real>      Box;

public:
    #define ENUM_LIST(e,_)  \
        e(_, x)             \
        e(_, xNeg)          \
        e(_, y)             \
        e(_, yNeg)          \
        e(_, z)             \
        e(_, zNeg)          \

    /// Bounding planes
    ENUM(OrientBox, Planes);
    #undef ENUM_LIST

    ///No initialization
    OrientBox_()                                                            {}
    
    ///Construct from center, 3 unit axes and extent (half box dimensions)
    OrientBox_(const Vec3& center, const Vec3 axisX, const Vec3 axisY, const Vec3 axisZ, const Vec3& extent)
        : center(center), extent(extent)
    {
        axis[0] = axisX;    axis[1] = axisY;    axis[2] = axisZ;
    }

    ///Construct from center, 3 unit axes and extent (half box dimensions)
    OrientBox_(const Vec3& center, const Vec3* axes, const Vec3& extent)
        : center(center), extent(extent)
    {
        axis[0] = axes[0];  axis[1] = axes[1];  axis[2] = axes[2];
    }

    ///Construct from axis-aligned box
    OrientBox_(const Box& box)
        : center(box.getCenter()), extent(box.getExtent())
    {
        axis[0] = Vec3::axisX;  axis[1] = Vec3::axisY;  axis[2] = Vec3::axisZ;
    }
    
    ~OrientBox_()                                                           {}

    /// Number of vertices (constant)
    static const int vertexCount = 8;

    /// Access 8 corner vertices of the box
    Vec3 operator[](int i) const                                            { return vertex(i); }

    /// Get one of 8 corner vertices of box
    /**
      * Vertex order:
      *     - 0-3 = min Z face, counter-clockwise from min vertex
      *     - 4-7 = max Z face, counter-clockwise from max vertex
      */
    Vec3 vertex(int index) const
    {
        assert(index<vertexCount);

        Vec3 extX = axis[0]*extent.x;
        Vec3 extY = axis[1]*extent.y;
        Vec3 extZ = axis[2]*extent.z;

        switch (index)
        {
        case 0:
            return center - extX - extY - extZ;
        case 1:
            return center - extX + extY - extZ;
        case 2:
            return center + extX + extY - extZ;
        case 3:
            return center + extX - extY - extZ;
        case 4:
            return center + extX + extY + extZ;
        case 5:
            return center - extX + extY + extZ;
        case 6:
            return center - extX - extY + extZ;
        case 7:
            return center + extX - extY + extZ;
        }
        return Vec3::zero;
    }

    /// Number of vertices (constant)
    static const int edgeCount = 12;

    /// Get one of 12 edges of box
    /**
      * Edge order:
      *     - 0-3 = min Z face, counter-clockwise from min vertex
      *     - 4-7 = max Z face, counter-clockwise from max vertex
      *     - 8-11 = Z spokes, counter-clockwise from min vertex
      */
    Line edge(int index) const
    {
        assert(index<edgeCount);
        switch (index)
        {
        case 0:
            return Line(vertex(0), vertex(1));
        case 1:
            return Line(vertex(1), vertex(2));
        case 2:
            return Line(vertex(2), vertex(3));
        case 3:
            return Line(vertex(3), vertex(0));
        case 4:
            return Line(vertex(4), vertex(5));
        case 5:
            return Line(vertex(5), vertex(6));
        case 6:
            return Line(vertex(6), vertex(7));
        case 7:
            return Line(vertex(7), vertex(4));
        case 8:
            return Line(vertex(0), vertex(6));
        case 9:
            return Line(vertex(1), vertex(5));
        case 10:
            return Line(vertex(2), vertex(4));
        case 11:
            return Line(vertex(3), vertex(7));
        }
        return Line();
    }

    /// Number of planes (constant)
    static const int planeCount = 6;

    /// Get one of 6 box planes
    Plane plane(int index) const
    {
        assert(index<planeCount);
        switch (index)
        {
        case Planes::x:
            return Plane(axis[0], center.dot(axis[0]) + extent.x);
        case Planes::xNeg:
            return Plane(-axis[0], -center.dot(axis[0]) + extent.x);
        case Planes::y:
            return Plane(axis[1], center.dot(axis[1]) + extent.y);
        case Planes::yNeg:
            return Plane(-axis[1], -center.dot(axis[1]) + extent.y);
        case Planes::z:
            return Plane(axis[2], center.dot(axis[2]) + extent.z);
        case Planes::zNeg:
            return Plane(-axis[2], -center.dot(axis[2]) + extent.z);
        }
        return Plane();
    }

    /// Get bounding sphere
    Sphere toSphere() const;

    /// Transform operator
    friend OrientBox_ operator*(const Matrix<4,4,Real>& mat, const OrientBox_& box)
    {
        OrientBox_ ret;
        ret.center = mat*box.center;
        ret.axis[0] = mat.mulRotScale(box.axis[0]).normalize(ret.extent.x);
        ret.axis[1] = mat.mulRotScale(box.axis[1]).normalize(ret.extent.y);
        ret.axis[2] = mat.mulRotScale(box.axis[2]).normalize(ret.extent.z);
        ret.extent.elemMulEq(box.extent);
        return ret;
    }
    friend OrientBox_ operator*(const Transform_<Real>& tm, const OrientBox_& box)
    {
        if (tm.hasScale())
        {
            OrientBox_ ret;
            ret.center = tm*box.center;
            ret.axis[0] = tm.mulRotScale(box.axis[0]).normalize(ret.extent.x);
            ret.axis[1] = tm.mulRotScale(box.axis[1]).normalize(ret.extent.y);
            ret.axis[2] = tm.mulRotScale(box.axis[2]).normalize(ret.extent.z);
            ret.extent.elemMulEq(box.extent);
            return ret;
        }
        else
            return OrientBox_(tm*box.center, tm.mulRotScale(box.axis[0]), tm.mulRotScale(box.axis[1]), tm.mulRotScale(box.axis[2]), box.extent);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const OrientBox_& val)
    {
        return os << "{ center" << val.center << ", axis{ " << val.axis[0] << ", " << val.axis[1] << ", " << val.axis[2] << " }, extent" << val.extent << " }";
    }

public:
    Vec3 center;
    Vec3 axis[3];
    Vec3 extent;
};    

typedef OrientBox_<Real>   OrientBox;
typedef OrientBox_<Float>  OrientBox_f;
typedef OrientBox_<Double> OrientBox_d;

}
