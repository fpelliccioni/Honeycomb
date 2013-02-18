// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Box.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename Box_<Real>::Sphere Box_<Real>::toSphere() const    { return Sphere(getCenter(), radius()); }

template class Box_<Float>;
template class Box_<Double>;

}