// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/FrustumOrtho.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
void FrustumOrtho_<Real>::fromOrthographic(Real left, Real right, Real bottom, Real top, Real near, Real far)
{
    switch (_orientation)
    {
    case Orientation::angle_0:
    case Orientation::angle_180:
        _extent[0].z = left;
        _extent[1].z = right;
        _extent[0].y = bottom;
        _extent[1].y = top;
        break;
    case Orientation::angle_90:
    case Orientation::angle_270:
        _extent[0].y = left;
        _extent[1].y = right;
        _extent[0].z = bottom;
        _extent[1].z = top;
        break;
    }

    _extent[0].x = near;
    _extent[1].x = far;

    assert( _extent[1].x != _extent[0].x &&
            _extent[1].y != _extent[0].y &&
            _extent[1].z != _extent[0].z);

    onProjChange();
}

template<class Real>
void FrustumOrtho_<Real>::onProjChange()
{
    onTmChange();
    updateProjection();
}

template<class Real>
void FrustumOrtho_<Real>::onTmChange()
{
    // left plane
    _planes[Planes::left].normal = _axis[2];
    _planes[Planes::left].dist = _origin.dot(_planes[Planes::left].normal) + _extent[0].z;

    // right plane
    _planes[Planes::right].normal = -_axis[2];
    _planes[Planes::right].dist = _origin.dot(_planes[Planes::right].normal) - _extent[1].z;

    // bottom plane
    _planes[Planes::bottom].normal = _axis[1];
    _planes[Planes::bottom].dist = _origin.dot(_planes[Planes::bottom].normal) + _extent[0].y;

    // top plane
    _planes[Planes::top].normal = -_axis[1];
    _planes[Planes::top].dist = _origin.dot(_planes[Planes::top].normal) - _extent[1].y;

    Real dDp = _origin.dot(_axis[0]);

    // near plane
    _planes[Planes::near].normal = _axis[0];
    _planes[Planes::near].dist = dDp + _extent[0].x;

    // far plane
    _planes[Planes::far].normal = -_axis[0];
    _planes[Planes::far].dist = -dDp - _extent[1].x;

    updateVertices();
    updateBox();
}

template<class Real>
void FrustumOrtho_<Real>::updateProjection()
{
    /* 
      *[ a   0   0   c  ]
      *[ 0   b   0   d  ]
      *[ 0   0   q   qn ]
      *[ 0   0   0   1  ]
      *
      * a = 2 * / (right - left)
      * b = 2 * / (top - bottom)
      * c = - (right + left) / (right - left)
      * d = - (top + bottom) / (top - bottom)
      * q = - 2 / (far - near)
      * qn = - (far + near) / (far - near)
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
            _proj[0][0] = 2 * l_inv;
            _proj[0][3] = -(_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][1] = 2 * u_inv;
            _proj[1][3] = -(_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -2 * d_inv;
            _proj[2][3] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[3][3] = 1;
            break;
        }
    case Orientation::angle_90:
        {
            _proj[0][1] = -2 * l_inv;
            _proj[0][3] = -(_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][0] = 2 * u_inv;
            _proj[1][3] = -(_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -2 * d_inv;
            _proj[2][3] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[3][3] = 1;
            break;
        }
    case Orientation::angle_180:
        {
            _proj[0][0] = -2 * l_inv;
            _proj[0][3] = -(_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][1] = -2 * u_inv;
            _proj[1][3] = -(_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -2 * d_inv;
            _proj[2][3] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[3][3] = 1;
            break;
        }
    case Orientation::angle_270:
        {
            _proj[0][1] = 2 * l_inv;
            _proj[0][3] = -(_extent[1].z + _extent[0].z) * l_inv;
            _proj[1][0] = -2 * u_inv;
            _proj[1][3] = -(_extent[1].y + _extent[0].y) * u_inv;
            _proj[2][2] = -2 * d_inv;
            _proj[2][3] = -(_extent[1].x + _extent[0].x) * d_inv;
            _proj[3][3] = 1;
            break;
        }
    }

    _proj = _clipDepth * _proj;
}

template<class Real>
void FrustumOrtho_<Real>::onOrientChange()
{
    fromOrthographic(_extent[0].z, _extent[1].z, _extent[0].y, _extent[1].y, _extent[0].x, _extent[1].x);
}

template<class Real>
void FrustumOrtho_<Real>::updateVertices()
{
    Vec3 d_min = _axis[0] * _extent[0].x;
    Vec3 d_max = _axis[0] * _extent[1].x;

    Vec3 u_min = _axis[1] * _extent[0].y;
    Vec3 u_max = _axis[1] * _extent[1].y;
    Vec3 r_min = _axis[2] * _extent[0].z;
    Vec3 r_max = _axis[2] * _extent[1].z;

    //Far face, starts at top left, clockwise order
    _verts[0] = _origin + d_max + u_max + r_min;
    _verts[1] = _origin + d_max + u_max + r_max;
    _verts[2] = _origin + d_max + u_min + r_max;
    _verts[3] = _origin + d_max + u_min + r_min;

    //Near face, starts at top left, clockwise order
    _verts[4] = _origin + d_min + u_max + r_min;
    _verts[5] = _origin + d_min + u_max + r_max;
    _verts[6] = _origin + d_min + u_min + r_max;
    _verts[7] = _origin + d_min + u_min + r_min;
}



template class FrustumOrtho_<Float>;
template class FrustumOrtho_<Double>;


}
