// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Geom/Frustum.h"
#include "Honey/Math/Geom/Ray.h"
#include "Honey/Scene/Viewport.h"
#include "Honey/Misc/Lazy.h"

namespace honey
{

/// Base camera class.  In the identity local space the camera axes are oriented so that the coordinate axes (X,Y,-Z) = (Right,Up,Forward)
class Camera
{
public:

    Camera() :
        _fixedYawEnable(true),
        _fixedYaw(Vec3::axisY),
        _view([&](Matrix4& val) { val = getTm().inverse(); }),
        _viewInv([&](Matrix4& val) { val = getTm(); }),
        _projInv([&](Matrix4& val) { val = proj().inverse(); }),
        _viewProj([&](Matrix4& val) { val = proj()*view(); }),
        _viewProjInv([&](Matrix4& val) { val = viewInv()*projInv(); }) {}

    virtual ~Camera()                                               {}

    /// Translate in world space
    void move(const Vec3& v)                                        { setPos(getPos()+v); }

    /// Translate in local space relative to rotated local axes. (x,y,z) = (Right, Up, Forward)
    void moveLocal(const Vec3& v)                                   { setPos(getPos()+getRot()*Vec3(v.x,v.y,-v.z)); }

    /// Rotate in world space
    void rotate(const Quat& q)                                      { setRot((q*getRot()).normalize_fast()); }

    /// Rotate in local space around Right (X) axis. Tip: Right-hand rule, thumb points right, +ve camera rotation is finger curl (counter-clockwise).
    void pitch(Real radians)                                        { rotate(Quat(getRot()*Vec3::axisX, radians)); }
    
    /// Enable/disable using a fixed yaw axis for yaw rotations
    void enableFixedYaw(bool fixedYaw, const Vec3& axis = Vec3::axisY)   { _fixedYawEnable = fixedYaw; _fixedYaw = axis; }

    /// Rotate in local space around Up (Y) axis, or in world space around the fixed yaw axis. Tip: Right-hand rule, thumb points up, +ve camera rotation is finger curl (counter-clockwise).
    void yaw(Real radians)                                          { rotate(Quat(_fixedYawEnable ? _fixedYaw : getRot()*Vec3::axisY, radians)); }

    /// Rotate in local space around -Forward (+Z) axis.  Tip: Right-hand rule, thumb points away from screen (+Z), +ve camera rotation is finger curl (counter-clockwise).
    void roll(Real radians)                                         { rotate(Quat(getRot()*Vec3::axisZ, radians)); }

    /// Look at position in world space
    void lookAt(const Vec3& pos)                                    { setDir(pos - getPos()); }

    /// Set forward direction that camera is looking towards. Uses fixed yaw axis if available to avoid unintended roll.
    void setDir(const Vec3& dir);
    Vec3 getDir() const                                             { return -getRot().axisZ(); }

    /// Set camera's up direction
    void setUp(const Vec3& up)                                      { rotate(Quat(getUp(), up.normalize())); }
    Vec3 getUp() const                                              { return getRot().axisY(); } 

    /// Set camera's right direction
    void setRight(const Vec3& right)                                { rotate(Quat(getRight(), right.normalize())); }
    Vec3 getRight() const                                           { return getRot().axisX(); }

    /// Set world space position
    void setPos(const Vec3& pos)                                    { _tm.setTrans(pos); onTmChange(); }
    const Vec3& getPos() const                                      { return _tm.getTrans(); }

    /// Set world space rotation
    void setRot(const Quat& rot)                                    { _tm.setRot(rot); onTmChange(); }
    /// Get world space rotation
    const Quat& getRot() const                                      { return _tm.getRot(); }

    /// Set world space transform
    void setTm(const Transform& tm)                                 { assert(!tm.hasScale()); _tm = tm; onTmChange(); }
    const Transform& getTm() const                                  { return _tm; }

    /// Get the camera's view matrix
    const Matrix4& view() const                                     { return _view; }
    /// Get the camera's inverse view matrix
    const Matrix4& viewInv() const                                  { return _viewInv; }

    /// Get the camera's projection matrix
    const Matrix4& proj() const                                     { return frustum().proj(); }
    /// Get the camera's inverse projection matrix
    const Matrix4& projInv() const                                  { return _projInv; }

    /// Get the camera's view-projection matrix
    const Matrix4& viewProj() const                                 { return _viewProj; }
    /// Get the camera's inverse view-projection matrix
    const Matrix4& viewProjInv() const                              { return _viewProjInv; }

    /// Project a 3D world space point, returns a 2D screen space point.
    /**
      * Screen space range is (0,0) top-left to (1,1) bottom-right. \n
      * Projected Z range is (Frustum::ClipDepthMin, Frustum::ClipDepthMax)
      */
    Vec3 project(const Vec3& point) const;

    /// Project a 2D screen space point, returns a 3D world space ray.
    /** 
      * Screen space range is (0,0) top-left to (1,1) bottom-right. \n
      * Ray origin is the camera position. Ray direction is towards the projected point on the far z plane.
      */ 
    Ray projectInv(const Vec2& screenPoint) const;

    /// Set viewport rectangle. Full-screen range is (0,0,1,1)
    void setViewportRect(const Rect& rect)                          { _viewport.setRect(rect); onViewportChange(); }

    /// Get viewport
    const Viewport& viewport() const                                { return _viewport; }

    /// Get near z plane distance
    virtual Real getZNear() const = 0;
    /// Get far z plane distance
    virtual Real getZFar() const = 0;

    /// Get frustum in local space
    virtual const Frustum& frustum() const                          { return const_cast<Camera*>(this)->frustumPriv(); }
    /// Get frustum in world space
    virtual const Frustum& frustumWorld() const = 0;

    /// Set orientation of projection
    void setOrientation(Frustum::Orientation orientation)           { frustumPriv().setOrientation(orientation); onProjChange(); }
    Frustum::Orientation getOrientation() const                     { return frustum().getOrientation(); }

protected:
    /// Get non-const frustum in local space
    virtual Frustum& frustumPriv() = 0;

    virtual void onTmChange()
    {
        _view.setDirty(true);
        _viewInv.setDirty(true);
        _viewProj.setDirty(true);
        _viewProjInv.setDirty(true);
    }

    virtual void onProjChange()
    {
        _projInv.setDirty(true);
        _viewProj.setDirty(true);
        _viewProjInv.setDirty(true);
    }

    virtual void onViewportChange()                                 {}

    Transform           _tm;        ///< World transform
    Viewport            _viewport;

    bool                _fixedYawEnable;
    Vec3                _fixedYaw;

    lazy<Matrix4>       _view;
    lazy<Matrix4>       _viewInv;
    lazy<Matrix4>       _projInv;
    lazy<Matrix4>       _viewProj;
    lazy<Matrix4>       _viewProjInv;
};

}
