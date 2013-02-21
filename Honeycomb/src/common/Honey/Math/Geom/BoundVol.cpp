// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/BoundVol.h"

namespace honey
{


template<> bool BoundVol<Sphere>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(bv.subc<Box>().getShape(), getShape());
    case Type::Cylinder:
        return Intersect::test(getShape(), bv.subc<Cylinder>().getShape().toSphere());
    case Type::Cone:
        return Intersect::test(bv.subc<Cone>().getShape(), getShape());
    case Type::Capsule:
        return Intersect::test(bv.subc<Capsule>().getShape(), getShape());
    case Type::OrientBox:
        return Intersect::test(bv.subc<OrientBox>().getShape(), getShape());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<Box>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(getShape(), bv.subc<Box>().getShape());
    case Type::Cylinder:
        return Intersect::test(getShape(), bv.subc<Cylinder>().getShape().toSphere());
    case Type::Cone:
        return Intersect::test(getShape(), bv.subc<Cone>().getShape().toSphere());
    case Type::Capsule:
        return Intersect::test(bv.subc<Capsule>().getShape(), getShape());
    case Type::OrientBox:
        return Intersect::test(bv.subc<OrientBox>().getShape(), getShape());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<Cylinder>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape().toSphere(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(bv.subc<Box>().getShape(), getShape().toSphere());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<Cone>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(bv.subc<Box>().getShape(), getShape().toSphere());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<Capsule>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(getShape(), bv.subc<Box>().getShape());
    case Type::Capsule:
        return Intersect::test(getShape(), bv.subc<Capsule>().getShape());
    case Type::OrientBox:
        return Intersect::test(bv.subc<OrientBox>().getShape(), getShape());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<OrientBox>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(getShape(), bv.subc<Box>().getShape());
    case Type::Capsule:
        return Intersect::test(getShape(), bv.subc<Capsule>().getShape());
    case Type::OrientBox:
        return Intersect::test(getShape(), bv.subc<OrientBox>().getShape());
    case Type::FrustumOrtho:
        return Intersect::test(bv.subc<FrustumOrtho>().getShape(), getShape());
    case Type::FrustumPersp:
        return Intersect::test(bv.subc<FrustumPersp>().getShape(), getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}


template<> bool BoundVol<FrustumOrtho>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(getShape(), bv.subc<Box>().getShape());
    case Type::Cylinder:
        return Intersect::test(getShape(), bv.subc<Cylinder>().getShape());
    case Type::Cone:
        return Intersect::test(getShape(), bv.subc<Cone>().getShape());
    case Type::Capsule:
        return Intersect::test(getShape(), bv.subc<Capsule>().getShape());
    case Type::OrientBox:
        return Intersect::test(getShape(), bv.subc<OrientBox>().getShape());
    case Type::FrustumOrtho:
        return Intersect::test(getShape(), bv.subc<FrustumOrtho>().getShape());
    case Type::FrustumPersp:
        return Intersect::test(getShape(), bv.subc<FrustumPersp>().getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}

template<> bool BoundVol<FrustumPersp>::test(const BoundVolBase& bv) const
{
    switch (bv.type())
    {
    case Type::Sphere:
        return Intersect::test(getShape(), bv.subc<Sphere>().getShape());
    case Type::Box:
        return Intersect::test(getShape(), bv.subc<Box>().getShape());
    case Type::Cylinder:
        return Intersect::test(getShape(), bv.subc<Cylinder>().getShape());
    case Type::Cone:
        return Intersect::test(getShape(), bv.subc<Cone>().getShape());
    case Type::Capsule:
        return Intersect::test(getShape(), bv.subc<Capsule>().getShape());
    case Type::OrientBox:
        return Intersect::test(getShape(), bv.subc<OrientBox>().getShape());
    case Type::FrustumOrtho:
        return Intersect::test(getShape(), bv.subc<FrustumOrtho>().getShape());
    case Type::FrustumPersp:
        return Intersect::test(getShape(), bv.subc<FrustumPersp>().getShape());
    default:
        error(sout() << "Unsupported intersection test. Bv 1: " << this->type() << " ; Bv 2: " << bv.type());
        return false;
    }
}

template class BoundVol<Sphere>;
template class BoundVol<Box>;
template class BoundVol<Cylinder>;
template class BoundVol<Cone>;
template class BoundVol<Capsule>;
template class BoundVol<OrientBox>;
template class BoundVol<FrustumOrtho>;
template class BoundVol<FrustumPersp>;

}