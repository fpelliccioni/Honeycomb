// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Frustum.h"

namespace honey
{

/// 3D orthographic frustum geometry class.
/**
  * An orthographic frustum makes a box from the near plane to the far plane.
  * The orthographic projection is right-handed.
  */
template<class Real>
class FrustumOrtho_ : public Frustum_<Real>
{
    typedef Frustum_<Real> Super;
    typedef typename Super::FrustumType FrustumType;
public:
    typedef typename Super::Orientation Orientation;
    typedef typename Super::Planes Planes;
    
    /// No initialization
    FrustumOrtho_() {}

    /// Create orthographic frustum
    /**
      * \param left     left plane distance
      * \param right    right plane distance
      * \param bottom   bottom plane distance
      * \param top      top plane distance
      * \param near     near plane distance
      * \param far      far plane distance
      */
    FrustumOrtho_(Real left, Real right, Real bottom, Real top, Real near, Real far)
    {
        fromOrthographic(left, right, bottom, top, near, far);
    }

    virtual ~FrustumOrtho_() {}

    virtual FrustumType frustumType() const                                 { return FrustumType::ortho; }

    void fromOrthographic(Real left, Real right, Real bottom, Real top, Real near, Real far);

    /// Transform operator
    friend FrustumOrtho_ operator*(const Matrix4& mat, const FrustumOrtho_& frustum)
    {
        FrustumOrtho_ ret = frustum;
        ret._origin = mat*ret._origin;
        Vec3 scale;
        ret._axis[0] = mat.mulRotScale(ret._axis[0]).normalize(scale.x);
        ret._axis[1] = mat.mulRotScale(ret._axis[1]).normalize(scale.y);
        ret._axis[2] = mat.mulRotScale(ret._axis[2]).normalize(scale.z);
        ret._extent[0].elemMulEq(scale);
        ret._extent[1].elemMulEq(scale);
        ret.onProjChange();
        return ret;
    }

    friend FrustumOrtho_ operator*(const Transform_<Real>& tm, const FrustumOrtho_& frustum)
    {
        FrustumOrtho_ ret = frustum;
        ret._origin = tm*ret._origin;
        if (tm.hasScale())
        {
            Vec3 scale;
            ret._axis[0] = tm.mulRotScale(ret._axis[0]).normalize(scale.x);
            ret._axis[1] = tm.mulRotScale(ret._axis[1]).normalize(scale.y);
            ret._axis[2] = tm.mulRotScale(ret._axis[2]).normalize(scale.z);
            ret._extent[0].elemMulEq(scale);
            ret._extent[1].elemMulEq(scale);
            ret.onProjChange();
        }
        else
        {
            ret._axis[0] = tm.mulRotScale(ret._axis[0]);
            ret._axis[1] = tm.mulRotScale(ret._axis[1]);
            ret._axis[2] = tm.mulRotScale(ret._axis[2]);
            ret.onTmChange();
        }
        return ret;
    }

protected:
    using Super::_orientation;
    using Super::_extent;
    using Super::_axis;
    using Super::_origin;
    using Super::_planes;
    using Super::_proj;
    using Super::_clipDepth;
    using Super::_verts;
    
    virtual void onProjChange();
    virtual void onTmChange();
    virtual void onOrientChange();

    void updateProjection();
    void updateVertices();
};

typedef FrustumOrtho_<Real>     FrustumOrtho;
typedef FrustumOrtho_<Float>    FrustumOrtho_f;
typedef FrustumOrtho_<Double>   FrustumOrtho_d;

}
