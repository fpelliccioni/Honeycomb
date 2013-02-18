// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Cylinder.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename Cylinder_<Real>::Sphere Cylinder_<Real>::toSphere() const    { return Sphere(center, Vec2(0.5*height, radius).length()); }

template class Cylinder_<Float>;
template class Cylinder_<Double>;

}