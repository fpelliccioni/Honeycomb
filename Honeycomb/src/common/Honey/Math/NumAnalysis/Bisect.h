// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"

namespace honey
{

/// Find the root of a function by the bisection method. ie. Finds where function returns 0.
/**
  * Bisection should only be used if the function to be evaluated is monotonic over the range [min,max],
  * ie. the function is either increasing or decreasing over the range.
  */ 
template<class Real>
class Bisect
{
    typedef Alge_<Real>             Alge;

public:
    typedef function<Real (Real)>   Func;

    /**
      * \param tol      find root to within tolerance of zero
      * \param iterMax  max bisections
      */
    Bisect(Real tol = RealT::zeroTol, int iterMax = 30)     : _tol(tol), _iterMax(iterMax) {}

    /// Find the lower and upper bounds of the root of a function. ie. Estimate bounds where somewhere within the function returns 0.
    /**
      * This method finds the bounds by expanding min/max each step
      *
      * \param func
      * \param min      root lower bound estimate
      * \param max      root upper bound estimate
      * \retval found   if not found a best guess is returned
      * \retval min
      * \retval max
      */
    tuple<bool,Real,Real> bracket(const Func& func, Real min, Real max);

    /// Find the root of a function within bounds [min,max]
    /**
      * If the bounds are unknown call Bracket to estimate the bounds
      *
      * \param func
      * \param min      root lower bound
      * \param max      root upper bound
      * \retval found   if not found a best guess is returned
      * \retval root
      */
    tuple<bool, Real> root(const Func& func, Real min, Real max);

private:
    Real _tol;
    int _iterMax;
};

}

