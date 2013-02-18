// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Capsule.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename Capsule_<Real>::Sphere Capsule_<Real>::toSphere() const    { return Sphere(line.center(), line.extent()+radius); }

template class Capsule_<Float>;
template class Capsule_<Double>;

}