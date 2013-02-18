// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Cone.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename Cone_<Real>::Sphere Cone_<Real>::toSphere() const    { return Sphere(vertex, Vec2(height, radius()).length()); }

template class Cone_<Float>;
template class Cone_<Double>;

}