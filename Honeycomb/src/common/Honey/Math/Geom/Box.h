// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Plane.h"

namespace honey
{

template<class Real> class Sphere_;

/// 3D axis-aligned box geometry class
template<class Real>
class Box_ : public Shape<Real>
{
    typedef Vec<3,Real>     Vec3;
    typedef Line_<Real>     Line;
    typedef Plane_<Real>    Plane;
    typedef Sphere_<Real>   Sphere;

public:

    /// No initialization
    Box_()                                                              {}

    /// Construct from minimum and maximum vertices
    Box_(const Vec3& min, const Vec3& max)                              { fromBounds(min, max); }

    /// Construct a box that bounds a cloud of points
    Box_(const Vec3* vs, int count)                                     { fromPoints(vs, count); }

    /// Construct from minimum and maximum vertices
    Box_& fromBounds(const Vec3& min, const Vec3& max)                  { assert(max >= min); this->min = min; this->max = max; return *this; }

    /// Construct from center and extent (half box dimensions)
    Box_& fromCenter(const Vec3& center, const Vec3& extent)            { return fromBounds(center-extent, center+extent); }

    /// Construct a box that bounds a cloud of points
    Box_& fromPoints(const Vec3* vs, int count)
    {
        assert(vs && count > 0);
        Vec3 min = vs[0], max = vs[0];

        for (int i = 1; i < count; ++i)
        {
            const Vec3& v = vs[i];

            if (v.x < min.x)
                min.x = v.x;
            else if (v.x > max.x)
                max.x = v.x;

            if (v.y < min.y)
                min.y = v.y;
            else if (v.y > max.y)
                max.y = v.y;

            if (v.z < min.z)
                min.z = v.z;
            else if (v.z > max.z)
                max.z = v.z;
        }

        fromBounds(min, max);
        return *this;
    }

    ~Box_()                                                             {}

    /// Extend the bounds to include another box
    void extend(const Box_& box)                                        { min = min.elemMin(box.min); max = max.elemMax(box.max); }

    /// Extend the bounds to include a point
    void extend(const Vec3& v)                                          { min = min.elemMin(v); max = max.elemMax(v); }

    /// Get center point of box
    Vec3 getCenter() const                                              { return (min + max)*0.5; }

    /// Move box center to center point
    void setCenter(const Vec3& pos)                                     { Vec3 v = pos - getCenter(); fromBounds(min+v, max+v); }

    /// Get extent from center of box (half box dimensions)
    Vec3 getExtent() const                                              { return dim()*0.5; }
    
    /// Set extent from center of box (half box dimensions)
    void setExtent(const Vec3& extent)                                  { fromCenter(getCenter(), extent); }

    /// Calc dimensions of box
    Vec3 dim() const                                                    { return max - min; }

    /// Calc maximum radius from center
    Real radius() const                                                 { return dim().length()*0.5; }

    /// Calc box surface area
    Real area() const                                                   { Vec3 vol = max - min; return (vol.x*vol.y + vol.x*vol.z + vol.y*vol.z) * 2; }

    /// Calc box volume
    Real volume() const                                                 { Vec3 vol = max - min; return vol.x*vol.y*vol.z; }

    /// Number of vertices (constant)
    static const int vertexCount = 8;

    /// Access 8 corner vertices of the box
    Vec3 operator[](int i) const                                        { return vertex(i); }

    /// Get one of 8 corner vertices of box
    /**
      * Vertex order:
      *     - 0-3 = min Z face, counter-clockwise from min vertex
      *     - 4-7 = max Z face, counter-clockwise from max vertex
      */
    Vec3 vertex(int index) const
    {
        assert(index<vertexCount);
        switch (index)
        {
        case 0:
            return min;
        case 1:
            return Vec3(min.x, max.y, min.z);
        case 2:
            return Vec3(max.x, max.y, min.z);
        case 3:
            return Vec3(max.x, min.y, min.z);
        case 4:
            return max;
        case 5:
            return Vec3(min.x, max.y, max.z);
        case 6:
            return Vec3(min.x, min.y, max.z);
        case 7:
            return Vec3(max.x, min.y, max.z);
        }
        return Vec3::zero;
    }

    /// Number of edges (constant)
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

    #define ENUM_LIST(e,_)  \
        e(_, x)             \
        e(_, xNeg)          \
        e(_, y)             \
        e(_, yNeg)          \
        e(_, z)             \
        e(_, zNeg)          \

    /// Bounding planes
    ENUM(Box, Planes);
    #undef ENUM_LIST

    /// Get one of 6 box planes
    Plane plane(int index) const
    {
        assert(index<planeCount);
        switch (index)
        {
        case Planes::x:
            return Plane(Vec3::axisX, max.x);
        case Planes::xNeg:
            return Plane(-Vec3::axisX, -min.x);
        case Planes::y:
            return Plane(Vec3::axisY, max.y);
        case Planes::yNeg:
            return Plane(-Vec3::axisY, -min.y);
        case Planes::z:
            return Plane(Vec3::axisZ, max.z);
        case Planes::zNeg:
            return Plane(-Vec3::axisZ, -min.z);
        }
        return Plane();
    }

    /// Get bounding sphere
    Sphere toSphere() const;

    /// Transform operator
    friend Box_ operator*(const Matrix<4,4,Real>& mat, const Box_& box)
    {
        //Transform vertices then find extremes
        Vec3 p = mat * box[0];
        Vec3 min = p, max = p;
        for(int i = 1; i < vertexCount; ++i)
        {
            p = mat * box[i];
            if(p.x > max.x) max.x = p.x;
            if(p.y > max.y) max.y = p.y;
            if(p.z > max.z) max.z = p.z;
            if(p.x < min.x) min.x = p.x;
            if(p.y < min.y) min.y = p.y;
            if(p.z < min.z) min.z = p.z;
        }
        return Box_(min, max);
    }
    friend Box_ operator*(const Transform_<Real>& tm, const Box_& box)
    {
        if (tm.hasRot())
        {
            //Transform vertices then find extremes
            Vec3 p = tm * box[0];
            Vec3 min = p, max = p;
            for(int i = 1; i < vertexCount; ++i)
            {
                p = tm * box[i];
                if(p.x > max.x) max.x = p.x;
                if(p.y > max.y) max.y = p.y;
                if(p.z > max.z) max.z = p.z;
                if(p.x < min.x) min.x = p.x;
                if(p.y < min.y) min.y = p.y;
                if(p.z < min.z) min.z = p.z;
            }
            return Box_(min, max);
        }

        //Fast transform without rotation, assumes that scale is positive
        return Box_(tm*box.min, tm*box.max);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Box_& val)
    {
        return os << "{ min" << val.min << ", max" << val.max << " }";
    }

public:
    Vec3 min;            ///< min vertex
    Vec3 max;            ///< max vertex
};

typedef Box_<Real>     Box;
typedef Box_<Float>    Box_f;
typedef Box_<Double>   Box_d;

}
