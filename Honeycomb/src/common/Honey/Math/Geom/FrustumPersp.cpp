// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/FrustumPersp.h"
#include "Honey/Math/Alge/Alge.h"
#include "Honey/Math/Alge/Trig.h"

namespace honey
{

template<class Real>
void FrustumPersp_<Real>::fromPerspective(Real fov, Real aspect, Real near, Real far)
{
    _fov = fov;
    _aspect = aspect;
    _near = near;
    _far = far;

    Real halfAngle = 0.5 * fov;

    switch (_orientation)
    {
    case Orientation::angle_0:
    case Orientation::angle_180:
        _extent[1].y = near * Trig::tan(halfAngle);
        _extent[1].z = aspect * _extent[1].y;
        break;
    case Orientation::angle_90:
    case Orientation::angle_270:
        _extent[1].y = near * Trig::tan(halfAngle);
        _extent[1].z = (1 / aspect) * _extent[1].y;
        break;
    }

    _extent[0].y = -_extent[1].y;
    _extent[0].z = -_extent[1].z;
    _extent[0].x = near;
    _extent[1].x = far;

    assert( _extent[1].x != _extent[0].x &&
            _extent[1].y != _extent[0].y &&
            _extent[1].z != _extent[0].z);

    onProjChange();
}

template<class Real>
void FrustumPersp_<Real>::onProjChange()
{
    //Cache coefficients (eliminates square root from the more common transform changes)

    Real dMinSqr = _extent[0].x*_extent[0].x;
    Real invLength;

    // left plane normal vector coefficients
    Real rMinSqr = _extent[0].z*_extent[0].z;
    invLength = 1 / Alge::sqrt(dMinSqr + rMinSqr);
    _coeffL[0] = -_extent[0].z*invLength;  // D component
    _coeffL[1] =  _extent[0].x*invLength;  // R component

    // right plane normal vector coefficients
    Real rMaxSqr = _extent[1].z*_extent[1].z;
    invLength = 1 / Alge::sqrt(dMinSqr + rMaxSqr);
    _coeffR[0] =  _extent[1].z*invLength;  // D component
    _coeffR[1] = -_extent[0].x*invLength;  // R component

    // bottom plane normal vector coefficients
    Real uMinSqr = _extent[0].y*_extent[0].y;
    invLength = 1 / Alge::sqrt(dMinSqr + uMinSqr);
    _coeffB[0] = -_extent[0].y*invLength;  // D component
    _coeffB[1] =  _extent[0].x*invLength;  // U component

    // top plane normal vector coefficients
    Real uMaxSqr = _extent[1].y*_extent[1].y;
    invLength = 1 / Alge::sqrt(dMinSqr + uMaxSqr);
    _coeffT[0] =  _extent[1].y*invLength;  // D component
    _coeffT[1] = -_extent[0].x*invLength;  // U component

    onTmChange();
    updateProjection();
}

template<class Real>
void FrustumPersp_<Real>::onTmChange()
{
    // left plane
    _planes[Planes::left].normal = _coeffL[0]*_axis[0] + _coeffL[1]*_axis[2];
    _planes[Planes::left].dist = _origin.dot(_planes[Planes::left].normal);

    // right plane
    _planes[Planes::right].normal = _coeffR[0]*_axis[0] + _coeffR[1]*_axis[2];
    _planes[Planes::right].dist = _origin.dot(_planes[Planes::right].normal);

    // bottom plane
    _planes[Planes::bottom].normal = _coeffB[0]*_axis[0] + _coeffB[1]*_axis[1];
    _planes[Planes::bottom].dist = _origin.dot(_planes[Planes::bottom].normal);

    // top plane
    _planes[Planes::top].normal = _coeffT[0]*_axis[0] + _coeffT[1]*_axis[1];
    _planes[Planes::top].dist = _origin.dot(_planes[Planes::top].normal);

    Real ddE = _axis[0].dot(_origin);

    // near plane
    _planes[Planes::near].normal = _axis[0];
    _planes[Planes::near].dist = ddE + _extent[0].x;

    // far plane
    _planes[Planes::far].normal = -_axis[0];
    _planes[Planes::far].dist = -ddE - _extent[1].x;

    updateVertices();
    this->updateBox();
}

template<class Real>
void FrustumPersp_<Real>::onOrientChange()
{
    fromPerspective(_fov, _aspect, _near, _far);
}

template<class Real>
void FrustumPersp_<Real>::updateProjection()
{
    /*
      *[a  0  c  0 ]
      *[0  b  d  0 ]
      *[0  0  q  qn]
      *[0  0  -1 0 ]
      *
      * a = 2 * near / (right - left)
      * b = 2 * near / (top - bottom)
      * c = (right + left) / (right - left)
      * d = (top + bottom) / (top - bottom)
      * q = - (far + near) / (far - near)
      * qn = - 2 * (far * near) / (far - near)
      *
      */

    Real l_inv = 1 / (_extent[1].z - _extent[0].z);
    Real u_inv = 1 / (_extent[1].y - _extent[0].y);
    Real d_inv = 1 / (_extent[1].x - _extent[0].x);

    _proj.fromZero();

    switch (_orientation)
    {
    case Orientation::angle_0:
        {
            _proj[0][0] = 2 * _extent[0].x * l_inv;
            _proj[0][2] = (_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][1] = 2 * _extent[0].x * u_inv;
            _proj[1][2] = (_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[2][3] = -2 * (_extent[1].x * _extent[0].x) * d_inv;
            _proj[3][2] = -1;
            break;
        }
    case Orientation::angle_90:
        {
            _proj[0][1] = -2 * _extent[0].x * u_inv; 
            _proj[0][2] = (_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][0] = 2 * _extent[0].x * l_inv;
            _proj[1][2] = (_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[2][3] = -2 * (_extent[1].x * _extent[0].x) * d_inv;
            _proj[3][2] = -1;
            break;
        }
    case Orientation::angle_180:
        {
            _proj[0][0] = -2 * _extent[0].x * l_inv;
            _proj[0][2] = (_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][1] = -2 * _extent[0].x * u_inv;
            _proj[1][2] = (_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[2][3] = -2 * (_extent[1].x * _extent[0].x) * d_inv;
            _proj[3][2] = -1;
            break;
        }
    case Orientation::angle_270:
        {
            _proj[0][1] = 2 * _extent[0].x * u_inv; 
            _proj[0][2] = (_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][0] = -2 * _extent[0].x * l_inv;
            _proj[1][2] = (_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[2][3] = -2 * (_extent[1].x * _extent[0].x) * d_inv;
            _proj[3][2] = -1;
            break;
        }
    }

    _proj = _clipDepth * _proj;
}

template<class Real>
void FrustumPersp_<Real>::updateVertices()
{
    Vec3 d_min = _axis[0] * _extent[0].x;
    Vec3 d_max = _axis[0] * _extent[1].x;

    Vec3 u_min = _axis[1] * _extent[0].y;
    Vec3 u_max = _axis[1] * _extent[1].y;
    Vec3 r_min = _axis[2] * _extent[0].z;
    Vec3 r_max = _axis[2] * _extent[1].z;

    Real d_scale = _extent[1].x / _extent[0].x;

    //Far face, starts at top left, clockwise order
    //P + (f/n)(+- zL +- yU + nD)

    _verts[0] = _origin + d_max + d_scale*(u_max + r_min);
    _verts[1] = _origin + d_max + d_scale*(u_max + r_max);
    _verts[2] = _origin + d_max + d_scale*(u_min + r_max);
    _verts[3] = _origin + d_max + d_scale*(u_min + r_min);

    //Near face, starts at top left, clockwise order
    //P + (+- zL +- yU + nD)

    _verts[4] = _origin + d_min + u_max + r_min;
    _verts[5] = _origin + d_min + u_max + r_max;
    _verts[6] = _origin + d_min + u_min + r_max;
    _verts[7] = _origin + d_min + u_min + r_min;
}



template class FrustumPersp_<Float>;
template class FrustumPersp_<Double>;

}
