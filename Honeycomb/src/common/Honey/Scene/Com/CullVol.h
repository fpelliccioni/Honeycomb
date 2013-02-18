// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Scene/Com/Tm.h"
#include "Honey/Math/Geom/BoundVol.h"

namespace honey
{

/// Cull volume base class
class CullVolBase : public SceneComponent
{
public:
    COMPONENT(CullVolBase)

    SIGNAL_DECL(CullVolBase)
    /// Called on shape change
    SIGNAL(sigShapeChange, (CullVolBase& src));

    CullVolBase()                                       : _cull(true) {}

    /// Get local space bounding volume
    virtual const BoundVolBase& bv() const = 0;
    /// Get world space bounding volume. \see Tm::world() for info about `ancestorsUpdated`.
    virtual const BoundVolBase& bvWorld(bool ancestorsUpdated = false) const = 0;

    /// Set whether culling using the bounding volume is enabled
    void setCullEnable(bool enable)                     { _cull = enable; }
    bool getCullEnable() const                          { return _cull; }

protected:
    static ComRegistry::DepNode createTypeDep()         { ComRegistry::DepNode node; node.add(Tm::s_comType()); return node; }

    virtual void onShapeChange()                        { listeners().dispatch<sigShapeChange>(*this); }

    bool _cull;
};
COMPONENT_REG(CullVolBase)


/// Generic volume for culling
template<class Shape>
class CullVol : public CullVolBase
{
public:
    typedef BoundVol<Shape> Bv;

    COMPONENT_SUB(CullVolBase, CullVol, StringStream() << "CullVol" << "<" << Bv::s_type() << ">")

    CullVol() :
        _bv(_shape),
        _bvWorld(bind_fill(&CullVol::bvWorldEval, this), bind(&CullVol::bvWorldDirty, this)),
        _worldState(0)
                                                        { _bvWorld.raw().setShape(_shapeWorld); }

    /// Set the shape that this cull volume uses
    void setShape(const Shape& shape)                   { _shape = shape; onShapeChange(); }
    const Shape& getShape() const                       { return _shape; }

    virtual const Bv& bv() const                        { return _bv; }
    virtual const Bv& bvWorld(bool ancestorsUpdated = false) const
                                                        { obj().com<Tm>().updateWorld(ancestorsUpdated); return _bvWorld; }

protected:
    virtual void onShapeChange()                        { _bvWorld.setDirty(true); CullVolBase::onShapeChange(); }

    bool bvWorldDirty()                                 { return _worldState != obj().com<Tm>().worldState(); }

    void bvWorldEval(Bv&)
    {
        const Transform& world = obj().com<Tm>().world(true);
        _shapeWorld = world * _shape;
        _worldState = obj().com<Tm>().worldState();
    }

    Shape               _shape;
    Bv                  _bv;
    Shape               _shapeWorld;
    lazy<Bv>            _bvWorld;
    atomic::Var<int>    _worldState;
};
COMPONENT_REG(CullVol<Sphere>)
COMPONENT_REG(CullVol<Box>)
COMPONENT_REG(CullVol<Cylinder>)
COMPONENT_REG(CullVol<Cone>)
COMPONENT_REG(CullVol<Capsule>)
COMPONENT_REG(CullVol<OrientBox>)
COMPONENT_REG(CullVol<FrustumOrtho>)
COMPONENT_REG(CullVol<FrustumPersp>)


/// Generated box for culling
class CullBoxGen : public CullVol<Box>
{
public:
    COMPONENT_SUB(CullVol<Box>, CullBoxGen)

    CullBoxGen()                                        { _cull = false; }

    /// Update the box to bound all descendants.  If updateTree is true then descendants that have a CullBoxGen will be updated first.
    /**
      * If updateTree is true then CullBoxGen's are created for descendants that don't have a cull volume.
      */
    void update(bool updateTree = false);

private:
    void update_priv();

};
COMPONENT_REG(CullBoxGen)

}
