// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Geom/Line.h"

namespace honey
{

/// 2D axis-aligned rectangle for left-handed screen coordinates.  The origin is at the top-left, with a rightward +X axis and downward +Y axis.
template<class Real>
class Rect_
{
    typedef Vec<2,Real> Vec2;
    typedef Vec<3,Real> Vec3;
    typedef Line_<Real> Line;

public:

    /// No initialization
    Rect_()                                                             {}

    /// Construct from origin and dimensions
    Rect_(const Vec2& origin, const Vec2& dim)                          { fromOrigin(origin, dim); }

    /// Construct from origin and dimensions
    Rect_(Real top, Real left, Real width, Real height)                 { fromOrigin(top,left,width,height); }

    /// Construct from origin and dimensions
    Rect_& fromOrigin(const Vec2& origin, const Vec2& dim)              { this->origin = origin; this->dim = dim; return *this; }

    /// Construct from origin and dimensions
    Rect_& fromOrigin(Real top, Real left, Real width, Real height)     { origin.x = left; origin.y = top; dim.x = width; dim.y = height; return *this; }

    /// Construct from minimum and maximum vertices
    Rect_& fromBounds(const Vec2& min, const Vec2& max)                 { fromOrigin(min, max-min); }

    /// Construct from center and extent (half rect dimensions)
    Rect_& fromCenter(const Vec2& center, const Vec2& extent)           { return fromOrigin(center-extent, extent*2); }

    /// Construct a rect that bounds a cloud of points
    Rect_& fromPoints(const Vec2* vs, int count)
    {
        assert(vs && count > 0);
        Vec2 min = vs[0], max = vs[0];

        for (int i = 1; i < count; ++i)
        {
            const Vec2& v = vs[i];

            if (v.x < min.x)
                min.x = v.x;
            else if (v.x > max.x)
                max.x = v.x;

            if (v.y < min.y)
                min.y = v.y;
            else if (v.y > max.y)
                max.y = v.y;
        }

        fromBounds(min, max);
        return *this;
    }

    ~Rect_()                                                            {}

    /// Get x coordinate of rect left side
    Real left() const                                                   { return origin.x; }
    /// Get y coordinate of rect top side
    Real top() const                                                    { return origin.y; }
    /// Get x coordinate of rect right side
    Real right() const                                                  { return origin.x + dim.x; }
    /// Get y coordinate of rect bottom side
    Real bottom() const                                                 { return origin.y + dim.y; }

    /// Get width of rect
    Real width() const                                                  { return dim.x; }
    /// Get height of rect
    Real height() const                                                 { return dim.y; }

    /// Extend the bounds to include another rect
    void extend(const Rect_& rect)                                      { fromBounds(min().elemMin(rect.min()), max().elemMax(rect.max())); }

    /// Extend the bounds to include a point
    void extend(const Vec2& v)                                          { fromBounds(min().elemMin(v), max().elemMax(v)); }

    /// Get center point of box
    Vec2 getCenter() const                                              { return origin + dim*0.5; }

    /// Move rect center to center point
    void setCenter(const Vec2& pos)                                     { Vec2 v = pos - getCenter(); fromOrigin(origin+v, dim); }

    /// Get extent from center of rect (half rect dimensions)
    Vec2 getExtent() const                                              { return dim*0.5; }
    
    /// Set extent from center of rect (half rect dimensions)
    void setExtent(const Vec2& extent)                                  { fromCenter(getCenter(), extent); }

    /// Get minimum point of box
    const Vec2& min() const                                             { return origin; }

    /// Get maximum point of box
    Vec2 max() const                                                    { return origin + dim; }

    /// Calc maximum radius from center
    Real radius() const                                                 { return dim.length()*0.5; }

    /// Calc rect surface area
    Real area() const                                                   { return dim.x*dim.y; }

    /// Number of vertices (constant)
    static const int vertexCount = 4;

    #define ENUM_LIST(e,_)  \
        e(_, topLeft)       \
        e(_, topRight)      \
        e(_, bottomRight)   \
        e(_, bottomLeft)    \

    /// Rect vertices, clockwise from origin
    ENUM(Rect, Vertex);
    #undef ENUM_LIST

    /// Access 4 corner vertices of the rect
    Vec2 operator[](int i) const                                        { return vertex(i); }

    /// Get one of 4 corner vertices of rect. Vertices are clockwise from origin. \see ENUM(Rect, Vertex).
    Vec2 vertex(int index) const
    {
        assert(index<vertexCount);
        switch (index)
        {
        case Vertex::topLeft:
            return origin;
        case Vertex::topRight:
            return Vec2(right(), top());
        case Vertex::bottomRight:
            return Vec2(right(), bottom());
        case Vertex::bottomLeft:
            return Vec2(left(), bottom());
        }
        return Vec2::zero;
    }

    /// Number of edges (constant)
    static const int edgeCount = 4;

    #define ENUM_LIST(e,_)  \
        e(_, top)           \
        e(_, right)         \
        e(_, bottom)        \
        e(_, left)          \

    /// Rect edges, clockwise from origin
    ENUM(Rect, Edge);
    #undef ENUM_LIST

    /// Get one of 4 edges of rect.  Edges are clockwise from origin. \see ENUM(Rect, Edge)
    Line edge(int index) const
    {
        assert(index<edgeCount);
        switch (index)
        {
        case Edge::top:
            return Line(Vec3(vertex(0)), Vec3(vertex(1)));
        case Edge::right:
            return Line(Vec3(vertex(1)), Vec3(vertex(2)));
        case Edge::bottom:
            return Line(Vec3(vertex(2)), Vec3(vertex(3)));
        case Edge::left:
            return Line(Vec3(vertex(3)), Vec3(vertex(0)));
        }
        return Line();
    }

    /// Transform operator
    friend Rect_ operator*(const Matrix<4,4,Real>& mat, const Rect_& rect)
    {
        //Transform vertices then find extremes
        Vec2 p = mat * rect[0];
        Vec2 min = p, max = p;
        for(int i = 1; i < vertexCount; ++i)
        {
            p = mat * rect[i];
            if(p.x > max.x) max.x = p.x;
            if(p.y > max.y) max.y = p.y;
            if(p.x < min.x) min.x = p.x;
            if(p.y < min.y) min.y = p.y;
        }
        Rect_ ret;
        return ret.fromBounds(min, max);
    }
    friend Rect_ operator*(const Transform_<Real>& tm, const Rect_& rect)
    {
        //Transform vertices then find extremes
        Vec2 p = tm * rect[0];
        Vec2 min = p, max = p;
        for(int i = 1; i < vertexCount; ++i)
        {
            p = tm * rect[i];
            if(p.x > max.x) max.x = p.x;
            if(p.y > max.y) max.y = p.y;
            if(p.x < min.x) min.x = p.x;
            if(p.y < min.y) min.y = p.y;
        }
        Rect_ ret;
        return ret.fromBounds(min, max);
    }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Rect_& val)
    {
        return os << "{ origin" << val.origin << ", dim" << val.dim << " }";
    }

public:
    Vec2 origin; ///< top-left
    Vec2 dim;    ///< width and height
};

typedef Rect_<Real>     Rect;

}
