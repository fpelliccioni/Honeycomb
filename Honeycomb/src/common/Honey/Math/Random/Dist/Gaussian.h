// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Random/Dist/Uniform.h"

namespace honey
{

/// Generate a normally (Gaussian) distributed random variate
/**
  * The gaussian distribution is a core component for most other distributions.
  *
  * Example: Toss a coin 1000 times and count number of heads
  *
  * Probability density function:
  *
  * \f$\displaystyle p(x,\mu,\sigma) = \frac{1}{\sigma \sqrt{2\pi}} e^{-\frac{(x-\mu)^2}{2\sigma^2}} \f$
  *
  * Default values provide the standard normal distribution around 0
  *
  * \param  mu      \f$\mu\f$ Mean
  * \param  sigma   \f$\sigma\f$ Standard deviation. Range > 0
  * \retval x       Random variate. Range [-inf, inf]
  */
template<class Real>
class Gaussian_ : public RandomDist<Real>
{
public:
    Gaussian_(Real mu = 0, Real sigma = 1)                  :                   mu(mu), sigma(sigma) { assert(sigma > 0); }
    Gaussian_(RandomGen& gen, Real mu = 0, Real sigma = 1)  : RandomDist(gen),  mu(mu), sigma(sigma) { assert(sigma > 0); }

    virtual Real next() const;
    virtual Real pdf(Real x) const;
    virtual Real cdf(Real x) const;
    virtual Real cdfInv(Real P) const;
    virtual Real mean() const               { return mu; }
    virtual Real variance() const           { return Alge::sqr(sigma); }
    
    /// Gauss Error Function, related to Cdf.  Input: [-inf, inf] -> Output: [-1, 1]
    static Real erf(Real x)                 { return 2*Gaussian().cdf(Double(x)*Double_::sqrtTwo) - 1; }
    /// Complement of Error Function.
    static Real erfComp(Real x)             { return 2*Gaussian().cdf(-Double(x)*Double_::sqrtTwo); }
    /// Inverse of Error Function, related to CdfInv.  Input: [-1, 1] -> Output: [-inf, inf]
    static Real erfInv(Real y)              { return Gaussian().cdfInv(Double(y+1)/2) / Double_::sqrtTwo; }
    /// Inverse of complement Error Function.
    static Real erfCompInv(Real y)          { return -Gaussian().cdfInv(Double(y)/2) / Double_::sqrtTwo; }

    Real mu;
    Real sigma;
};

typedef Gaussian_<Real>      Gaussian;
typedef Gaussian_<Float>     Gaussian_f;
typedef Gaussian_<Double>    Gaussian_d;

}
