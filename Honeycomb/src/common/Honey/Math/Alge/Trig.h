// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real> class Interp_;

/// Speeds up all trig functions at the cost of precision.  Precision is roughly 1 / size.
template<class Real>
class SinTable
{
    typedef typename Numeral<Real>::RealT RealT;
    typedef Alge_<Real>     Alge;
    typedef Interp_<Real>   Interp;

public:
    SinTable()                                                      : _size(0) {}

    void resize(int size);
    int size() const                                                { return _size; }

    Real sin(Real x) const;
    Real asin(Real x) const;
    Real cos(Real x) const                                          { return sin(RealT::piHalf - x); }
    Real acos(Real x) const                                         { return RealT::piHalf - asin(x); }
    Real tan(Real x) const                                          { return sin(x) / cos(x); }
    Real atan(Real x) const                                         { return asin(x / Alge::sqrt(1 + x*x)); }
    Real atan2(Real y, Real x) const;

private:
    typedef vector<Real> List;

    Real linear(const List& list, Real idx) const                   { int cur = idx; return Interp::linear(idx-cur, list[cur], list[(cur+1)%_size]); }

    List _sin;
    List _asin;
    int  _size;
    int  _radToSin;
    int  _xToAsin;
};

/// Trigonometry
template<class Real>
class Trig_
{
    typedef typename Numeral<Real>::RealT RealT;
    typedef Alge_<Real> Alge;

public:
    /// Enable or disable sin table.  If no size is specified then the table will not be resized, or if uninitialized it will be set to tableSizeDefault.
    static void enableSinTable(bool enable, int size = -1);

    /// Sin of radian angle
    static Real sin(Real x)                                         { auto& t = inst(); return t._tableEnable ? t._table.sin(x) : RealT::sin(x); }
    /// Convert sine ratio [-1, 1] to radian angle [-pi/2, pi/2]
    static Real asin(Real x)                                        { auto& t = inst(); return t._tableEnable ? t._table.asin(x) : RealT::asin(x); }

    /// Cos of radian angle
    static Real cos(Real x)                                         { auto& t = inst(); return t._tableEnable ? t._table.cos(x) : RealT::cos(x); }
    /// Convert cosine ratio [-1, 1] to radian angle [0, pi]
    static Real acos(Real x)                                        { auto& t = inst(); return t._tableEnable ? t._table.acos(x) : RealT::acos(x); }

    /// Tan of radian angle
    static Real tan(Real x)                                         { auto& t = inst(); return t._tableEnable ? t._table.tan(x) : RealT::tan(x); }
    /// Convert tangent ratio [-inf, inf] to radian angle [-pi/2, pi/2]
    static Real atan(Real x)                                        { auto& t = inst(); return t._tableEnable ? t._table.atan(x) : RealT::atan(x); }
    /// Converts Cartesian(x,y) to Polar(r, theta), and returns radian angle theta [-pi, pi].
    static Real atan2(Real y, Real x)                               { auto& t = inst(); return t._tableEnable ? t._table.atan2(y,x) : RealT::atan2(y,x); }

    /// Convert angle in degrees to angle in radians.
    static Real radian(Real degree)                                 { return degree * RealT::pi / 180; }
    /// Convert angle in radians to angle in degrees.
    static Real degree(Real radian)                                 { return radian * 180 / RealT::pi; }

    /// Get an equivalent angle in the normalized range [-pi, pi]
    static Real normalizeAngle(Real angle)                          { return Alge::modNormalize(RealT::pi, angle); }
    /// Calc smallest angle to align angleFrom with angleTo. Angles must be normalized. Result is in range [-pi, pi].
    static Real alignAngle(Real angleFrom, Real angleTo)            { return Alge::modDistSigned(RealT::pi, angleFrom, angleTo); }
    /// Calc shortest angular distance between angle and angle2. Angles must be normalized. Result is in range [0, pi].
    static Real distanceAngle(Real angle, Real angle2)              { return Alge::abs(alignAngle(angle, angle2)); }

private:
    Trig_()                                                         : _tableEnable(false) {}

    /// Get singleton
    mt_staticObj(Trig_, inst,);

    SinTable<Real> _table;
    bool _tableEnable;

    static const int tableSizeDefault = 1 << 13;    ///< 8K elem table = 32KB ; 1/8K =~ 1e-5 precision
};

typedef Trig_<Real>         Trig;
typedef Trig_<Float>        Trig_f;
typedef Trig_<Double>       Trig_d;
typedef Trig_<Quad>         Trig_q;

}

