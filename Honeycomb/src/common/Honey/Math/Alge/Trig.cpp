// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Alge/Trig.h"
#include "Honey/Math/NumAnalysis/Interp.h"

namespace honey
{

template<class Real>
void SinTable<Real>::resize(int size)
{
    if (size == _size) return;

    _size = size;
    _sin.resize(_size);
    _asin.resize(_size);
    _radToSin = (_size-1) / RealT::piHalf;
    _xToAsin = _size-1;

    for (int i = 0; i < _size; ++i)
    {
        _sin[i] = RealT::sin(i*RealT::piHalf / (_size-1));
        _asin[i] = RealT::asin(Real(i) / (_size-1));
    }
}

template<class Real>
Real SinTable<Real>::sin(Real x) const
{
    //Take arbitrary angle and bound between [0, pi*2)
    Real xn = Alge::mod(x, RealT::piTwo);
    if (xn < 0) xn = RealT::piTwo + xn;
    //Sin table uses symmetry and defines one quarter [0,pi/2].  Apply transform for other quarters.
    if (xn < RealT::pi)
        return xn < RealT::piHalf ? linear(_sin, xn*_radToSin) : linear(_sin, (RealT::pi-xn)*_radToSin);
    else
        return xn < RealT::piAndHalf ? -linear(_sin, (xn-RealT::pi)*_radToSin) : -linear(_sin, (RealT::piTwo-xn)*_radToSin);
}

template<class Real>
Real SinTable<Real>::asin(Real x) const
{
    //Asin table uses symmetry and defines values for [0,1].  Input is [-1,1]
    if (x > 1 || x < -1) return RealT::nan;
    return x >= 0 ? linear(_asin, x*_xToAsin) : -linear(_asin, -x*_xToAsin);
}

template<class Real>
Real SinTable<Real>::atan2(Real y, Real x) const
{
    Real xAbs = Alge::abs(x);
    Real yAbs = Alge::abs(y);
        
    if (yAbs <= RealT::zeroTol)
        return (x >= 0) ? 0 : RealT::pi;
        
    if (xAbs <= RealT::zeroTol)
        return (y > 0) ? RealT::piHalf : -RealT::piHalf;
        
    //Switch the axes and use symmetry for atan if divisor is too small
    if (xAbs < yAbs)
    {
        Real z = atan(xAbs / yAbs);
        if (y > 0)
            return RealT::piHalf + ((x < 0) ? z : -z);
        else
            return -RealT::piHalf + ((x > 0) ? z : -z);
    }
        
    Real z = atan(yAbs / xAbs);
    if (x > 0)
        return (y > 0) ? z : -z;
    else
        return (y > 0) ? RealT::pi - z : z - RealT::pi;
}

template class SinTable<Float>;
template class SinTable<Double>;
template class SinTable<Quad>;


template<class Real>
void Trig_<Real>::enableSinTable(bool enable, int size)
{
    auto& t = inst();
    t._tableEnable = enable;
    if (!enable) return;
    if (size < 0) size = t._table.size() ? t._table.size() : tableSizeDefault;
    t._table.resize(size);
}

template class Trig_<Float>;
template class Trig_<Double>;
template class Trig_<Quad>;

}
