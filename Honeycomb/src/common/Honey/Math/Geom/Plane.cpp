// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Plane.h"
#include "Honey/Math/Geom/Triangle.h"
#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Alge/Trig.h"

namespace honey
{

template<class Real>
Plane_<Real>::Plane_(const Triangle& tri)
{
    fromPoints(tri.v0, tri.v1, tri.v2);
}

template class Plane_<Float>;
template class Plane_<Double>;

}