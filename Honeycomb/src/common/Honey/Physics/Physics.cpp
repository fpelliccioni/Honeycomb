// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Physics/Physics.h"

namespace honey
{

template<class Real>
Real Physics_<Real>::travelTime(Real dist, Real accel, Real velMin, Real velMax)
{
    if (accel <= 0 || velMax <= 0)
    {
        if (velMin <= 0)
            return 0;
        else
            return dist / velMin;
    }
    
    //Calc time to accelerate and reach dest, starting at min speed, ignoring max speed
    //This is derived from the quadratic formula of d=vt+1/2at^2
    Real accelToDestTime = (-velMin + Alge::sqrt(Alge::sqr(velMin) + ((dist*accel)*2))) / accel;

    //Calc time to achieve max speed
    Real accelTime = (velMax-velMin) / accel;
    if (accelToDestTime > accelTime)
    {
        //Calc dist travelled from start until we hit max speed
        Real accelDist = velMin*accelTime + ((accel*Alge::sqr(accelTime))/2);
        //Calc dist we must travel at max speed
        Real maxSpeedDist = dist - accelDist;
        //Final time is the time it takes to accelerate to max speed plus time it takes to reach dest at max speed
        accelToDestTime = accelTime + (maxSpeedDist / velMax);
    }
    
    return accelToDestTime;
}

template class Physics_<Float>;
template class Physics_<Double>;

}
