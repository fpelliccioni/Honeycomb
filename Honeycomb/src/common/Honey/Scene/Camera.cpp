// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Scene/Camera.h"

namespace honey
{

void Camera::setDir(const Vec3& dir_)
{
    Vec3 dir = dir_.normalize();
    if (dir.isNearZero()) return;

    if (_fixedYaw)
    {
        dir = -dir;
        Vec3 xAxis = _fixedYaw.cross(dir).normalize();
        setRot(Quat(xAxis, dir.cross(xAxis).normalize(), dir));
    }
    else
        rotate(Quat(getDir(), dir));
}

Vec3 Camera::project(const Vec3& point) const
{
    Vec3 pos = viewProj()*point;
    return Vec3(viewport().viewToScreen(viewport().clipToView(Vec2(pos))), pos.z);
}

Ray Camera::projectInv(const Vec2& screenPoint) const
{
    Vec3 pos = viewProjInv() * Vec3(viewport().viewToClip(viewport().screenToView(screenPoint)), Frustum::getClipDepthMax());
    return Ray(getPos(), (pos - getPos()).normalize());
}


}
