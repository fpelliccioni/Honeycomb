// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Alge/Trig.h"
#include "Honey/Math/Random/Gen.h"

namespace honey
{

template<class Real> class Random_;
template<class Real> class Uniform_;
template<class Real> class Gaussian_;
template<class Real> class Gamma_;
template<class Real> class GammaFunc_;
template<class Real> class Beta_;

/// Base class for all random distributions
template<class Real>
class RandomDist
{
protected:
    typedef typename Numeral<Real>::RealT   RealT;
    typedef typename RealT::DoubleType      Double_;
    typedef typename Double_::Real          Double;
    typedef Alge_<Real>                     Alge;
    typedef Alge_<Double>                   Alge_d;
    typedef Trig_<Real>                     Trig;
    typedef Trig_<Double>                   Trig_d;
    typedef Random_<Real>                   Random;
    typedef Uniform_<Double>                Uniform;
    typedef Gaussian_<Double>               Gaussian;
    typedef Gamma_<Double>                  Gamma;
    typedef GammaFunc_<Double>              GammaFunc;
    typedef Beta_<Double>                   Beta;

public:
    RandomDist()                                        : _gen(nullptr) {}
    /// Construct with a random generator to use for next()
    RandomDist(RandomGen& gen)                          : _gen(&gen) {}

    virtual ~RandomDist()                               {}

    /// Get next randomly distributed variate. Requires a random generator (see ctor or setGen())
    virtual Real next() const                           { return 0; }

    /// Probability Density Function
    /**
      * The PDF integrates to 1 over the entire range of possible values of x.
      *
      * \param  x   value, range depends on distribution.
      * \retval p   a relative likelihood in range [0,inf] that a random variate X will equal x.
      */ 
    virtual Real pdf(Real x) const                      { mt_unused(x); return 0; }
    
    /// Cumulative Distribution Function
    /** 
      * The integral of the PDF from -inf to x.
      *
      * \param  x   value, range depends on distribution.
      * \retval P   a probability in range [0,1] that a random variate will be <= x.
      */
    virtual Real cdf(Real x) const                      { mt_unused(x); return 0; }

    /// Complement of the CDF
    /**
      * The integral of the PDF from x to inf.
      *
      * \param  x   value, range depends on distribution.
      * \retval Q   a probability in range [0,1] that a random variate will be > x.
      */ 
    virtual Real cdfComp(Real x) const                  { return 1 - cdf(x); }

    /// Inverse of the CDF
    /**
      * \param  P   cumulative probability
      * \retval x   A value that has probability P of being >= a random variate X. Also, x satisfies: cdf(x) = P.
      */ 
    virtual Real cdfInv(Real P) const                   { mt_unused(P); return 0; }
    /// Calc mean
    virtual Real mean() const                           { return 0; }
    /// Calc variance
    virtual Real variance() const                       { return 0; }
    /// Calc standard deviation
    Real stdDev() const                                 { return Alge::sqrt(variance()); }

    /// Set random generator to use for next()
    void setGen(RandomGen& gen)                         { _gen = &gen; }
    /// Get random generator
    RandomGen& getGen() const                           { assert(_gen, "Must set a random generator"); return *_gen; }

protected:
    /// Generic binary search algorithm to find Cdf
    Real cdfInvFind(Real P, Real min, Real max, bool discrete = false) const;

private:
    RandomGen* _gen;
};

}
