// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Frustum.h"

namespace honey
{

/// 3D perspective frustum geometry class.  
/**
  * A perspective frustum makes a pyramid with the tip at the near z plane and the base at the far z plane.
  * The perspective projection is right-handed.
  */
template<class Real>
class FrustumPersp_ : public Frustum_<Real>
{
public:
    /// No initialization
    FrustumPersp_() {}

    /// Create perspective frustum
    /**
      * \param fov      Field of view angle along the Y axis (up direction) in radians
      * \param aspect   Ratio (width / height) of viewport
      * \param near     near z plane distance
      * \param far      far z plane distance
      */
    FrustumPersp_(Real fov, Real aspect, Real near, Real far)               { fromPerspective(fov, aspect, near, far); }

    virtual ~FrustumPersp_() {}

    virtual FrustumType frustumType() const                                 { return FrustumType::persp; }

    void fromPerspective(Real fov, Real aspect, Real near, Real far);

    Real fov() const                                                        { return _fov; }
    Real aspect() const                                                     { return _aspect; }
    Real near() const                                                       { return _near; }
    Real far() const                                                        { return _far; }

    /// Transform operator
    friend FrustumPersp_ operator*(const Matrix4& mat, const FrustumPersp_& frustum)
    {
        FrustumPersp_ ret = frustum;
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

    friend FrustumPersp_ operator*(const Transform_<Real>& tm, const FrustumPersp_& frustum)
    {
        FrustumPersp_ ret = frustum;
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

    virtual void onProjChange();
    virtual void onTmChange();
    virtual void onOrientChange();

    void updateProjection();
    void updateVertices();

    /// Original persp values
    Real     _fov;
    Real     _aspect;
    Real     _near;
    Real     _far;

    /// Cached values for computing frustum planes
    Real     _coeffL[2];
    Real     _coeffR[2];
    Real     _coeffB[2];
    Real     _coeffT[2];
};

typedef FrustumPersp_<Real>     FrustumPersp;
typedef FrustumPersp_<Float>    FrustumPersp_f;
typedef FrustumPersp_<Double>   FrustumPersp_d;

}
