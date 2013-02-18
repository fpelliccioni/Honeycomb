// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Alge/Trig.h"
#include "Honey/Math/Alge/Vec/Vec3.h"

namespace honey
{

/// Physics methods
template<class Real>
class Physics_
{
    typedef typename Numeral<Real>::RealT RealT;
    typedef Vec<3,Real>     Vec3;

public:
    /// Calc time to travel, given distance, acceleration and a velocity range
    static Real travelTime(Real dist, Real accel, Real velMin, Real velMax);

private:
    Physics_() {}
};  

typedef Physics_<Real>         Physics;
typedef Physics_<Float>        Physicsf;
typedef Physics_<Double>       Physicsd;

}

