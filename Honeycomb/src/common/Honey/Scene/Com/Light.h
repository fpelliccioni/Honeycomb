// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Object.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

/// Base class for lights
class Light : public SceneComponent
{
public:
    COMPONENT(Light)

    #define ENUM_LIST(e,_)  \
        e(_, omni)          \
        e(_, dir)           \
        e(_, spot)          \

    ENUM(Light, LightType);
    #undef ENUM_LIST

    Light() :
        _ambient(Vec3::zero),
        _diffuse(Vec3::one),
        _specular(Vec3::one),
        _intensity(1) {}

    virtual LightType lightType() const = 0;

    /// Calculate light intensity at distance.  Returns intensity in range [0,1]
    virtual Real calcIntensity(Real distance) const = 0;

    /// Calculate distance from light at intensity. Pass in intensityZero to determine the distance where the light contribution is effectively zero.
    virtual Real calcDistance(Real intensity) const = 0;

    void setAmbient(const Vec3& ambient)                { _ambient = ambient; }
    const Vec3& getAmbient() const                      { return _ambient; }

    void setDiffuse(const Vec3& diffuse)                { _diffuse = diffuse; }
    const Vec3& getDiffuse() const                      { return _diffuse; }

    void setSpecular(const Vec3& specular)              { _specular = specular; }
    const Vec3& getSpecular() const                     { return _specular; }

    /// Brightness of light, default value is 1.
    void setIntensity(Real intensity)                   { _intensity = intensity; }
    Real getIntensity() const                           { return _intensity; }

protected:
    Vec3    _ambient;
    Vec3    _diffuse;
    Vec3    _specular;
    Real    _intensity;

public:
    /// Intensity when light contribution is effectively zero
    static const Real intensityZero;
};


/// Omni directional (spherical) light
class LightOmni : public Light
{
public:
    #define ENUM_LIST(e,_)  \
        e(_, none)          \
        e(_, inv)           \
        e(_, invSqr)        \

    ENUM(LightOmni, DecayType);
    #undef ENUM_LIST

    COMPONENT_SUB(Light, LightOmni)

    LightOmni() :
        _decayType(DecayType::invSqr),
        _radius(1) {}

    virtual LightType lightType() const                 { return LightType::omni; }

    /// Calculate light intensity at distance.  Returns intensity in range [0,1]
    virtual Real calcIntensity(Real distance) const
    {
        Real intensity = 0;
        switch (_decayType)
        {
        case DecayType::none:
            intensity = 1;
            break;
        case DecayType::inv:
            intensity = _radius/distance;
            break;
        case DecayType::invSqr:
            intensity = (_radius*_radius)/(distance*distance);
            break;
        }
        return Alge::min(intensity, 1);
    }

    /// Calculate distance from light at intensity. Pass in intensityZero to determine the distance where the light contribution is effectively zero.
    virtual Real calcDistance(Real intensity) const
    {
        intensity = Alge::clamp(intensity, 0, 1);
        switch (_decayType)
        {
        case DecayType::none:
            return intensity == 1 ? Real(0) : RealT::inf;
        case DecayType::inv:
            return _radius/intensity;
        case DecayType::invSqr:
            return Alge::sqrt((_radius / intensity) * _radius);
        }
        return 0;
    }

    void setDecayType(DecayType decayType)              { _decayType = decayType; }
    DecayType getDecayType() const                      { return _decayType; }

    /// Light intensity range is [0,1] within this radius
    void setRadius(Real radius)                         { _radius = radius; }
    Real getRadius() const                              { return _radius; }

protected:
    DecayType   _decayType;
    Real        _radius;
};
COMPONENT_REG(LightOmni)

/// Directional light (ex. sunlight)
class LightDir : public Light
{
public:
    COMPONENT_SUB(Light, LightDir)

    LightDir() :
        _dir(-Vec3::axisZ) {}

    virtual LightType lightType() const                 { return LightType::dir; }

    /// Calculate light intensity at distance.  Returns intensity in range [0,1]
    virtual Real calcIntensity(Real /*distance*/) const { return 1; }

    /// Calculate distance from light at intensity. Pass in intensityZero to determine the distance where the light contribution is effectively zero.
    virtual Real calcDistance(Real intensity) const     { return Alge::clamp(intensity, 0, 1) == 1 ? Real(0) : RealT::inf; }

    /// Normalized light direction
    void setDir(const Vec3& dir)                        { _dir = dir; }
    const Vec3& getDir() const                          { return _dir; }

protected:
    Vec3 _dir;
};
COMPONENT_REG(LightDir)

/// Spot light
class LightSpot : public LightOmni
{
public:
    COMPONENT_SUB(Light, LightSpot)

    LightSpot() :
        _dir(-Vec3::axisZ),
        _spotCutoff(RealT::piHalf),
        _spotExponent(0) {}

    virtual LightType lightType() const                 { return LightType::spot; }

    /// Normalized light direction
    void setDir(const Vec3& dir)                        { _dir = dir; }
    const Vec3& getDir() const                          { return _dir; }

    /// Cutoff angle in radians.  Range [0,90]
    void setSpotCutoff(Real spotCutoff)                 { assert(spotCutoff >= 0 && spotCutoff <= RealT::piHalf); _spotCutoff = spotCutoff; }
    Real getSpotCutoff() const                          { return _spotCutoff; }

    /// Exponent for falloff within spot cutoff angle.
    void setSpotExponent(Real spotExponent)             { _spotExponent = spotExponent; }
    Real getSpotExponent() const                        { return _spotExponent;  }

protected:
    Vec3    _dir;
    Real    _spotCutoff;
    Real    _spotExponent;
};
COMPONENT_REG(LightSpot)

}
