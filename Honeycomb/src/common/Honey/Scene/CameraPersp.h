// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Camera.h"
#include "Honey/Math/Geom/FrustumPersp.h"

namespace honey
{

/// Perspective camera
class CameraPersp : public Camera
{
public:
    CameraPersp(Real fov = Real_::piQuarter, Real aspect = 4./3, Real near = Real_::quarter, Real far = 1000) :
        _frustum(fov, aspect, near, far),
        _frustumWorld([&](FrustumPersp& val) { val = getTm() * _frustum; }) {}

    /// Set up perspective camera
    /**
      * \param fov      Field of view angle in up direction in radians
      * \param aspect   Ratio (width / height) of viewport
      * \param near     near z plane distance
      * \param far      far z plane distance
      */
    void setPerspective(Real fov, Real aspect, Real near, Real far)     { _frustum.fromPerspective(fov, aspect, near, far); onProjChange(); }

    void setFov(Real fov)                                   { setPerspective(fov, getAspect(), getZNear(), getZFar()); }
    Real getFov() const                                     { return _frustum.fov(); }

    void setAspect(Real aspect)                             { setPerspective(getFov(), aspect, getZNear(), getZFar()); }
    Real getAspect() const                                  { return _frustum.aspect(); }

    void setZNear(Real near)                                { setPerspective(getFov(), getAspect(), near, getZFar()); }
    virtual Real getZNear() const                           { return _frustum.near(); }

    void setZFar(Real far)                                  { setPerspective(getFov(), getAspect(), getZNear(), far); }
    virtual Real getZFar() const                            { return _frustum.far(); }

    const FrustumPersp& frustum() const                     { return _frustum; }
    virtual const FrustumPersp& frustumWorld() const        { return _frustumWorld; }

protected:
    virtual FrustumPersp& frustumPriv()                     { return _frustum; }
    virtual void onTmChange()                               { Camera::onTmChange(); _frustumWorld.setDirty(true); }
    virtual void onProjChange()                             { Camera::onProjChange(); _frustumWorld.setDirty(true); }

    FrustumPersp        _frustum;
    lazy<FrustumPersp>  _frustumWorld;
};

}
