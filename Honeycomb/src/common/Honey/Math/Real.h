// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Numeral.h"

namespace honey
{

/// Base class for real number operations
template<class Real>
class RealBase : mt::NoCopy
{
public:
    typedef Real Real;
};

/// \name Real types
/// @{

/// Real number type. See \ref RealT for real number operations and constants.
typedef float Real;
/// @}

/// Operations and constants for \ref Real type. See Float_, Double_.
typedef Numeral<Real>::RealT RealT;

}

