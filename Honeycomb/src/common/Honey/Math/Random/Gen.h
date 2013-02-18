// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Core.h"
#include "Honey/Math/Double.h"

namespace honey
{

/// Random number generator interface
class RandomGen
{
public:
    /// State of Gaussian generator
    struct GaussianState
    {
        GaussianState()                         : deviate(Double_::inf) {}
        Double deviate;
    };

    /// Base random state, must be inherited by generators
    struct State : GaussianState                {};

    /// Generate random number between 0 and 2^64-1 inclusive
    virtual uint64 next() = 0;

    virtual State& getState() = 0;
};

}
