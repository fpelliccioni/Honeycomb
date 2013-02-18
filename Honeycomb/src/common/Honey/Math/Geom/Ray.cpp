// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/Ray.h"
#include "Honey/Math/Geom/Line.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
Ray_<Real>::Ray_(const Line& line) : origin(line.v0), dir(line.dir())   {}

template class Ray_<Float>;
template class Ray_<Double>;

}