// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Geom/Box.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
Sphere_<Real>& Sphere_<Real>::fromPoints(const Vec3* vs, int count)
{
    assert(vs && count > 0);

    //Get the min/max bounds

    Vec3 min = vs[0];
    Vec3 max = vs[0];

    for (int i = 1; i < count; i++)
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
    
    Vec3 dif = max - min;

    //Position the sphere in the center of the cloud
    center = min + dif / 2;
    //Set radius to encompass the cloud, account for error
    radius = dif.length() / 2 + Real_::zeroTol;

    return *this;
}

template<class Real>
Sphere_<Real> Sphere_<Real>::minSphere(const Sphere_& sphere)
{
    Sphere_ ret;
    Real length;
    Vec3 dir = (sphere.center - center).normalize(length);

    if (length + sphere.radius <= radius)
    {
        //sphere is completely contained in us
        return *this;
    }
    else if (length+radius <= sphere.radius)
    {
        //we are completely contained in sphere
        return sphere;
    }
    else
    {
        //Return minimum radius sphere that contains both
        ret.center = (sphere.center + center + dir*(sphere.radius - radius)) / 2;
        ret.radius = (length + sphere.radius + radius) / 2;
        return ret;
    }
}

template<class Real>
typename Sphere_<Real>::Box Sphere_<Real>::toBox() const  { Box box; return box.fromCenter(center, Vec3(radius)); }

template class Sphere_<Float>;
template class Sphere_<Double>;

}
