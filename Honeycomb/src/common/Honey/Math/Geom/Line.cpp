// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Geom/Ray.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
Line_<Real>::Line_(const Ray& ray, Real length) : v0(ray.origin), v1(ray.origin + ray.dir*length) {}

template class Line_<Float>;
template class Line_<Double>;

}