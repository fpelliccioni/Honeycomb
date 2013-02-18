// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Triangle.h"
#include "Honey/Math/Geom/Box.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename Triangle_<Real>::Box Triangle_<Real>::toBox() const     { return Box(v0.elemMin(v1.elemMin(v2)), v0.elemMax(v1.elemMax(v2))); }

template class Triangle_<Float>;
template class Triangle_<Double>;

}