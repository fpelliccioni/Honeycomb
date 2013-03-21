// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Alge/Transform.h"
#include "Honey/Math/Geom/Plane.h"
#include "Honey/Math/Geom/Box.h"

namespace honey
{

template<class Real> class Alge_;
template<class Real> class Trig_;

/// 3D frustum geometry base class
/**    
  * A frustum is defined by 6 bounding planes with inward facing normals. \n
  * These planes are created from 3 unit axes (forward, up, right) and min/max extents along those axes. \n
  *
  * Forward axis = -Z axis  \n
  * Up axis = Y axis        \n
  * Right axis = X axis     \n
  *
  * minExtent.x and maxExtent.x define the near and far planes along the forward axis.  \n
  * minExtent.y and maxExtent.y define the bottom and top planes along the up axis.     \n
  * minExtent.z and maxExtent.z define the left and right planes along the right axis.  \n
  */
template<class Real>
class Frustum_ : public Shape<Real>
{
protected:
    typedef Alge_<Real>         Alge;
    typedef Trig_<Real>         Trig;
    typedef Vec<2,Real>         Vec2;
    typedef Vec<3,Real>         Vec3;
    typedef Quat_<Real>         Quat;
    typedef Matrix<4,4,Real>    Matrix4;
    typedef Transform_<Real>    Transform;
    typedef Box_<Real>          Box;
    typedef Plane_<Real>        Plane;

public:
    #define ENUM_LIST(e,_)  \
        e(_, ortho)         \
        e(_, persp)         \

    /// Frustum types
    ENUM(Frustum_, FrustumType);
    #undef ENUM_LIST

    #define ENUM_LIST(e,_)  \
        e(_, angle_0)       \
        e(_, angle_90)      \
        e(_, angle_180)     \
        e(_, angle_270)     \

    /// Orientation, projection will be rotated counter-clockwise
    ENUM(Frustum_, Orientation);
    #undef ENUM_LIST

    #define ENUM_LIST(e,_)  \
        e(_, left)          \
        e(_, right)         \
        e(_, bottom)        \
        e(_, top)           \
        e(_, near)          \
        e(_, far)           \

    /// Bounding planes
    ENUM(Frustum_, Planes);
    #undef ENUM_LIST

    virtual ~Frustum_()                                                     {}

    virtual FrustumType frustumType() const = 0;

    /// Number of planes (constant)
    static const int planeCount = 6;

    /// Get one of the 6 frustum bounding planes
    const Plane& operator[](int i) const                                    { return plane(i); }

    /// Get one of the 6 frustum bounding planes
    const Plane& plane(int index) const                                     { return _planes[index]; }

    /// Number of vertices (constant)
    static const int vertexCount = 8;

    /// Get one of 8 corner vertices of frustum volume
    /**
      * Vertex order:
      *     - 0-3 = far face, counter-clockwise from top left vertex
      *     - 4-7 = near face, counter-clockwise from top left vertex
      */
    const Vec3& vertex(int index) const                                     { return _verts[index]; }

    /// Get local space transform (origin trans + axis rot).  Brings a point into same space as frustum volume. 
    Transform tm() const                                                    { return Transform(_origin, Quat(_axis[2], _axis[1], -_axis[0])); }

    /// Get right-handed projection onto the near plane in local space (doesn't include any translation/rotation)
    const Matrix4& proj() const                                             { return _proj; }

    /// Get minimum extents
    const Vec3& extentMin() const                                           { return _extent[0]; }
    /// Get maximum extents
    const Vec3& extentMax() const                                           { return _extent[1]; }

    /// Set clip space z depth range for all frustum projections (ex. OpenGL is [-1,1] (default), DirectX is [0,1])
    static void setClipDepth(Real min, Real max)
    {
        _clipDepthMin = min; _clipDepthMax = max;
        //Set up scale and bias relative to default of [-1,1]
        _clipDepth.fromIdentity();
        _clipDepth[2][2] = (_clipDepthMax - _clipDepthMin) / 2;
        _clipDepth[2][3] = _clipDepthMin + _clipDepth[2][2];
    }
    /// Get min clip space z depth
    static Real getClipDepthMin()                                           { return _clipDepthMin; }
    /// Get max clip space z depth
    static Real getClipDepthMax()                                           { return _clipDepthMax; }
    
    /// Set orientation of projection
    void setOrientation(const Orientation& orientation)                     { _orientation = orientation; onOrientChange(); }
    const Orientation& getOrientation() const                               { return _orientation; }

    /// Bring a non-oriented 2D point (Angle_0) into the frustum's oriented space.  point must be in clip space [-1,1]
    Vec2 orient(const Vec2& point) const
    {
        switch (getOrientation())
        {
        case Orientation::angle_90:
            return Vec2(point.y, -point.x);
        case Orientation::angle_180:
            return Vec2(-point.x, -point.y);
        case Orientation::angle_270:
            return Vec2(-point.y, point.x);
        }
        return point;
    }

    /// Bring an oriented 2D point into non-oriented space (Angle_0). point must be in clip space [-1,1]
    Vec2 orientInv(const Vec2& point) const
    {
        switch (getOrientation())
        {
        case Orientation::angle_90:
            return Vec2(-point.y, point.x);
        case Orientation::angle_180:
            return Vec2(-point.x, -point.y);
        case Orientation::angle_270:
            return Vec2(point.y, -point.x);
        }
        return point;
    }

    /// Get a box that bounds this frustum volume
    const Box& box() const                                                  { return _box; }

    /// To string
    friend StringStream& operator<<(StringStream& os, const Frustum_& val)
    {
        return os   << "{ frustumType(" << val.frustumType() << "), orientation(" << val._orientation << ")"
                    << ", origin" << val._origin
                    << ", axis{ " << val._axis[0] << ", " << val._axis[1] << ", " << val._axis[2] << " }"
                    << ", extentMin" << val._extent[0] << ", extentMax" << val._extent[1];
    }

protected:

    Frustum_() :
        _orientation(Orientation::angle_0),
        _origin(Vec3::zero)
    {
        _axis[0] = -Vec3::axisZ;
        _axis[1] = Vec3::axisY;
        _axis[2] = Vec3::axisX;
    }

    virtual void onTmChange() = 0;
    virtual void onProjChange() = 0;
    virtual void onOrientChange() = 0;

    void updateBox()                                                        { _box.fromPoints(_verts, vertexCount); }

    Orientation     _orientation;
    Vec3            _origin;
    Vec3            _axis[3];               ///< Unit axes.  Forward, up, right
    Vec3            _extent[2];             ///< Min/Max extent along axes

    Matrix4         _proj;                  ///< Projection matrix
    Plane           _planes[planeCount];    ///< Bounding planes
    Box             _box;                   ///< Bounding box
    Vec3            _verts[vertexCount];    ///< Corner vertices

    static Real     _clipDepthMin;          ///< Platform dependent clip space depth min
    static Real     _clipDepthMax;          ///< Platform dependent clip space depth max
    static Matrix4  _clipDepth;             ///< Clip space scale/bias matrix generated from min/max
};

template<class Real> Real Frustum_<Real>::_clipDepthMin = -1;
template<class Real> Real Frustum_<Real>::_clipDepthMax = 1;
template<class Real> typename Frustum_<Real>::Matrix4 Frustum_<Real>::_clipDepth = Matrix4::identity;

typedef Frustum_<Real>      Frustum;
typedef Frustum_<Float>     Frustum_f;
typedef Frustum_<Double>    Frustum_d;

}
