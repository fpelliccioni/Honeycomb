// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Camera.h"
#include "Honey/Math/Geom/FrustumOrtho.h"

namespace honey
{

/// Orthographic camera for 2D rendering on the XY plane
/**
  * Default setup: \n
  * The screen center is (x,y) = (0,0).  The bottom-left is (-1,-1), and the top-right is (1,1). \n
  * The camera looks down the -z axis, so the z range is (far,near) = (-1, 0)
  */
class CameraOrtho : public Camera
{
public:
    CameraOrtho(Real left = -1, Real right = 1, Real bottom = -1, Real top = 1, Real near = 0, Real far = 1) :
        _frustum(left, right, bottom, top, near, far),
        _frustumWorld([&](FrustumOrtho& val) { val = getTm() * _frustum; }) {}

    /// Set up orthographic camera
    /**
      * \param left     left plane distance
      * \param right    right plane distance
      * \param bottom   bottom plane distance
      * \param top      top plane distance
      * \param near     near plane distance
      * \param far      far plane distance
      */
    void setOrthographic(Real left, Real right, Real bottom, Real top, Real near, Real far)
    {
        _frustum.fromOrthographic(left, right, bottom, top, near, far);
        onProjChange();
    }

    void setOrthoLeft(Real left)                            { setOrthographic(left, getOrthoRight(), getOrthoBottom(), getOrthoTop(), getZNear(), getZFar()); }
    Real getOrthoLeft() const                               { return _frustum.extentMin().z; }
    
    void setOrthoRight(Real right)                          { setOrthographic(getOrthoLeft(), right, getOrthoBottom(), getOrthoTop(), getZNear(), getZFar()); }
    Real getOrthoRight() const                              { return _frustum.extentMax().z; }

    void setOrthoBottom(Real bottom)                        { setOrthographic(getOrthoLeft(), getOrthoRight(), bottom, getOrthoTop(), getZNear(), getZFar()); }
    Real getOrthoBottom() const                             { return _frustum.extentMin().y; }
    
    void setOrthoTop(Real top)                              { setOrthographic(getOrthoLeft(), getOrthoRight(), getOrthoBottom(), top, getZNear(), getZFar()); }
    Real getOrthoTop() const                                { return _frustum.extentMax().y; }
    
    void setZNear(Real near)                                { setOrthographic(getOrthoLeft(), getOrthoRight(), getOrthoBottom(), getOrthoTop(), near, getZFar()); }
    virtual Real getZNear() const                           { return _frustum.extentMin().x; }
    
    void setZFar(Real far)                                  { setOrthographic(getOrthoLeft(), getOrthoRight(), getOrthoBottom(), getOrthoTop(), getZNear(), far); }
    virtual Real getZFar() const                            { return _frustum.extentMax().x; }
    
    const FrustumOrtho& frustum() const                     { return _frustum; }
    virtual const FrustumOrtho& frustumWorld() const        { return _frustumWorld; }

protected:
    virtual FrustumOrtho& frustumPriv()                     { return _frustum; }
    virtual void onTmChange()                               { Camera::onTmChange(); _frustumWorld.setDirty(true); }
    virtual void onProjChange()                             { Camera::onProjChange(); _frustumWorld.setDirty(true); }

    FrustumOrtho        _frustum;
    lazy<FrustumOrtho>  _frustumWorld;
};

}
