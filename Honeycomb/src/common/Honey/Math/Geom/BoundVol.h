// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Misc/Enum.h"
#include "Honey/Math/Geom/Intersect.h"
#include "Honey/Math/Geom/FrustumOrtho.h"
#include "Honey/Math/Geom/FrustumPersp.h"


namespace honey
{

template<class Shape> class BoundVol;
class BoundVolAny;

/// Base class for all bounding volumes
class BoundVolBase
{
public:
    #define ENUM_LIST(e,_)  \
        e(_, Sphere)        \
        e(_, Box)           \
        e(_, Cylinder)      \
        e(_, Cone)          \
        e(_, Capsule)       \
        e(_, OrientBox)     \
        e(_, FrustumOrtho)  \
        e(_, FrustumPersp)  \

    ENUM(BoundVolBase, Type);
    #undef ENUM_LIST

    /// Get bounding volume type
    virtual Type type() const = 0;

    /// Get subclass
    template<class Shape>
    const BoundVol<Shape>& subc() const                                                 { assert(type() == BoundVol<Shape>::s_type()); return static_cast<const BoundVol<Shape>&>(*this); }
    template<class Shape>
    BoundVol<Shape>& subc()                                                             { assert(type() == BoundVol<Shape>::s_type()); return static_cast<BoundVol<Shape>&>(*this); }

    /// Create a clone of this bounding volume and store in `bv`
    virtual void clone(BoundVolAny& bv) const = 0;

    /// Apply transform to this bv
    virtual void mul(const Transform& tm) = 0;

    /// Check for intersection against `bv`
    virtual bool test(const BoundVolBase& bv) const = 0;

    /// Check for intersection against point
    virtual bool test(const Vec3& point) const = 0;

    /// Get shortest distance to point
    virtual Real distance(const Vec3& point, option<Vec3&> shapePoint = optnull) const = 0;

    /// Side of plane that this bv is on
    virtual Plane::Side side(const Plane& plane) const = 0;

    /// Convert this bv to a bounding box
    virtual Box toBox() const = 0;
};

/// Generic bounding volume. Wrapper around a shape that allows for generic intersection testing and transformations.
/**
  * All bounding volumes have guaranteed support for testing intersection against Sphere, Box, and Frustum.
  *
  * The following tests are inexact and give false positives:
  *     - Frustum vs Shape
  *     - Cylinder vs Box/Sphere
  *     - Cone vs Box
  */ 
template<class Shape>
class BoundVol : public BoundVolBase
{
    friend class BoundVol;
    friend class BoundVolAny;
public:
    BoundVol()                                                                          : _shape(nullptr) {}
    /// Construct with reference to shape
    BoundVol(Shape& shape)                                                              : _shape(&shape) {}

    static Type s_type();
    virtual Type type() const                                                           { return s_type(); }

    virtual void clone(BoundVolAny& bv) const;
    virtual void mul(const Transform& tm)                                               { getShape() = tm * getShape(); }
    virtual bool test(const BoundVolBase& bv) const;
    virtual bool test(const Vec3& point) const                                          { return Intersect::test(getShape(), point); }
    virtual Real distance(const Vec3& point, option<Vec3&> shapePoint = optnull) const  { return Intersect::distance(getShape(), point, shapePoint); }
    virtual Plane::Side side(const Plane& plane) const                                  { return Geom::side(plane, getShape()); }
    virtual Box toBox() const                                                           { return getShape().toBox(); }

    /// Set reference to shape
    void setShape(Shape& shape)                                                         { _shape = &shape; }
    const Shape& getShape() const                                                       { assert(_shape); return *_shape; }
    Shape& getShape()                                                                   { assert(_shape); return *_shape; }

private:
    Shape* _shape;
};

/// Allows a clone of any bounding volume to be stored on the stack, to which write ops like transforms can be applied.
class BoundVolAny : public BoundVolBase
{
    template<class> friend class BoundVol;

public:
    BoundVolAny()                                                                       : _shape(nullptr) {}
    template<class Shape>
    BoundVolAny(const BoundVol<Shape>& val)                                             { operator=(val); }

    template<class Shape>
    BoundVolAny& operator=(const BoundVol<Shape>& rhs)
    {
        assert(sizeof(Shape) <= sizeof(_shapeData));
        //Copy entire class including virtual table pointer
        memcpy(this, &rhs, sizeof(BoundVol<Shape>));
        //Copy shape object
        memcpy(_shapeData, &rhs.getShape(), sizeof(Shape));
        //Point our shape to our shape data
        _shape = _shapeData;
        return *this;
    }

    virtual Type type() const                                                                   { return subc().type(); }
    virtual void clone(BoundVolAny& bv) const                                                   { subc().clone(bv); }
    virtual void mul(const Transform& tm)                                                       { subc().mul(tm); }
    virtual bool test(const BoundVolBase& bv) const                                             { return subc().test(bv); }
    virtual bool test(const Vec3& point) const                                                  { return subc().test(point); }
    virtual Real distance(const Vec3& point, option<Vec3&> shapePoint = optnull) const          { return subc().distance(point, shapePoint); } 
    virtual Plane::Side side(const Plane& plane) const                                          { return subc().side(plane); }
    virtual Box toBox() const                                                                   { return subc().toBox(); }

private:
    /// Upcast to base to use virtual table pointer copied during assignment
    BoundVolBase& subc()                                                                        { return *this; }
    const BoundVolBase& subc() const                                                            { return *this; }

    uint8* _shape;                              ///< Mimics layout of BoundVol<Shape> class, pointer to our shape data
    uint8 _shapeData[sizeof(FrustumPersp)];     ///< Data for any shape type
};


template<class Shape>
inline void BoundVol<Shape>::clone(BoundVolAny& bv) const                                       { bv = *this; }


inline BoundVolBase::Type BoundVol<Sphere>::s_type()                                            { return Type::Sphere; }
bool BoundVol<Sphere>::test(const BoundVolBase& bv) const;

inline BoundVolBase::Type BoundVol<Box>::s_type()                                               { return Type::Box; }
bool BoundVol<Box>::test(const BoundVolBase& bv) const;
inline Box BoundVol<Box>::toBox() const                                                         { return getShape(); }

inline BoundVolBase::Type BoundVol<Cylinder>::s_type()                                          { return Type::Cylinder; }
bool BoundVol<Cylinder>::test(const BoundVolBase& bv) const;
inline Real BoundVol<Cylinder>::distance(const Vec3& point, option<Vec3&> shapePoint) const     { return Intersect::distance(getShape().toSphere(), point, shapePoint); }
inline Box BoundVol<Cylinder>::toBox() const                                                    { return getShape().toSphere().toBox(); }

inline BoundVolBase::Type BoundVol<Cone>::s_type()                                              { return Type::Cone; }
bool BoundVol<Cone>::test(const BoundVolBase& bv) const;
inline Real BoundVol<Cone>::distance(const Vec3& point, option<Vec3&> shapePoint) const         { return Intersect::distance(getShape().toSphere(), point, shapePoint); }
inline Box BoundVol<Cone>::toBox() const                                                        { return getShape().toSphere().toBox(); }

inline BoundVolBase::Type BoundVol<Capsule>::s_type()                                           { return Type::Capsule; }
bool BoundVol<Capsule>::test(const BoundVolBase& bv) const;
inline Box BoundVol<Capsule>::toBox() const                                                     { return getShape().toSphere().toBox(); }

inline BoundVolBase::Type BoundVol<OrientBox>::s_type()                                         { return Type::OrientBox; }
bool BoundVol<OrientBox>::test(const BoundVolBase& bv) const;
inline Box BoundVol<OrientBox>::toBox() const                                                   { return getShape().toSphere().toBox(); }

inline BoundVolBase::Type BoundVol<FrustumOrtho>::s_type()                                      { return Type::FrustumOrtho; }
bool BoundVol<FrustumOrtho>::test(const BoundVolBase& bv) const;
inline Box BoundVol<FrustumOrtho>::toBox() const                                                { return getShape().box(); }

inline BoundVolBase::Type BoundVol<FrustumPersp>::s_type()                                      { return Type::FrustumPersp; }
bool BoundVol<FrustumPersp>::test(const BoundVolBase& bv) const;
inline Box BoundVol<FrustumPersp>::toBox() const                                                { return getShape().box(); }

}
