// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/CameraPersp.h"

namespace honey
{

/// An arcball camera looks at a point and rotates around it.
/**
  * The camera is controlled by rotation functions in the base camera class (rotate, pitch, setRot, etc.)
  */
class CameraArcBall : public CameraPersp
{
public:
    CameraArcBall() :
      _focalPoint(0),
      _focalDist(0),
      _focalOffset(0)
                                                            {}

    virtual ~CameraArcBall()                                {}

    /// Translate focal point in world space
    void moveFocalPoint(const Vec3& v)                      { setFocalPoint(_focalPoint + v); }

    /// Translate focal point in local space (relative to rotated camera axes)
    void moveFocalPointLocal(const Vec3& v)                 { setFocalPoint(_focalPoint + getRot()*v); }

    /// Set world space focal point
    void setFocalPoint(const Vec3& point)                   { _focalPoint = point; onTmChange(); }
    const Vec3& getFocalPoint() const                       { return _focalPoint; }

    /// Set fixed radial distance from the focal point
    void setFocalDist(Real dist)                            { _focalDist = Alge::max(dist, 0); onTmChange(); }
    Real getFocalDist() const                               { return _focalDist; }

    /// Set local space offset from the focal point
    void setFocalOffset(const Vec3& offset)                 { _focalOffset = offset; onTmChange(); }
    const Vec3& getFocalOffset() const                      { return _focalOffset; }
    
protected:

    virtual void onTmChange()
    {
        CameraPersp::onTmChange();
        _tm.setTrans(_focalPoint + getRot()*Vec3(0, 0, _focalDist) + getRot()*_focalOffset);
    }

    Vec3    _focalPoint;
    Real    _focalDist;
    Vec3    _focalOffset;
};

}
