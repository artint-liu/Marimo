#include "../clstd.h"
#include "clString.H"
#include "clUtility.H"
namespace clstd
{
  int _AABBF::IntersectAABB(const _AABBF& aabb) const
  {
    return IntersectAABBAABB(aabb, *this);
  }

  int _AABBF::IntersectFrustum(const _FrustumPlanes& frustum) const
  {
    return IntersectAABBFrustum(*this, frustum);
  }

  int _AABBF::IntersectRay(const _NormalizedRay& ray, float* fDist, float3* pNormal) const
  {
    return IntersectRayAABB(ray, *this, fDist, pNormal);
  }

  //////////////////////////////////////////////////////////////////////////
  _ray::_ray(const _NormalizedRay& nr)
    : vOrigin(nr.vOrigin)
    , vDirection(nr.vDirection)
  {
  }
  //////////////////////////////////////////////////////////////////////////
  _plane::_plane()
    : vNormal(0.0f)
    , fDist(0.0f)
  {  
  }

  _plane::_plane(const float3* aVert3)
  {
    set(aVert3[0], aVert3[1], aVert3[2]);
  }

  _plane::_plane(const float4& vNormalDist)
  {
    vNormal.set(vNormalDist.x, vNormalDist.y, vNormalDist.z);
    fDist = vNormalDist.w;
  }

  _plane::_plane(const float3& vVert0,const float3& vVert1,const float3& vVert2)
  {
    vNormal = float3::cross(vVert1 - vVert0, vVert2 - vVert1).normalize();
    fDist = vNormal.dot(vVert1);
  }

  _plane::_plane(const _plane& Plane)
    : vNormal(Plane.vNormal)
    , fDist(Plane.fDist)
  {
  }

  _plane::_plane(const float3& _vNormal, const float _fDist)
    : vNormal(_vNormal)
    , fDist(_fDist)
  {
    //if(fDist < 0)
    //{
    //  fDist = -fDist;
    //  vNormal = -vNormal;
    //}
  }

  _plane& _plane::normalize()
  {
    float fLength = vNormal.normalize();
    if(fLength > 0) {
      fDist /= fLength;
    }
    else {
      fDist = 0.0f;
    }
    return *this;
  }
  _plane _plane::normalize(const _plane& plane)
  {
    _plane p = plane;
    return p.normalize();
  }

  void _plane::set(const float3* aVert3)
  {
    set(aVert3[0], aVert3[1], aVert3[2]);
  }

  void _plane::set(const float3& vVert0,const float3& vVert1,const float3& vVert2)
  {
    vNormal = float3::cross(vVert1 - vVert0, vVert2 - vVert1).normalize();
    fDist = vNormal.dot(vVert1);
  }

  void _plane::set(const _plane& Plane)
  {
    vNormal = Plane.vNormal;
    fDist = Plane.fDist;
  }

  void _plane::set(const float3& _vNormal, const float _fDist)
  {
    vNormal = _vNormal;
    fDist = _fDist;
  }
  //////////////////////////////////////////////////////////////////////////
  _ray::_ray(const float3& _vOrigin, const float3& _vDirection)
    : vOrigin(_vOrigin)
    , vDirection(_vDirection)
  {
  }
  _NormalizedRay::_NormalizedRay(const float3& _vOrigin, const float3& _vDirection)
    : vOrigin   (_vOrigin)
    , vDirection(_vDirection)
  {
    vDirection.normalize();
  }

  //////////////////////////////////////////////////////////////////////////
  _LineSegment::_LineSegment(const float3& vPoint0, const float3& vPoint1)
  {
    vPoint[0] = vPoint0;
    vPoint[1] = vPoint1;
  }
  _LineSegment::_LineSegment(const float3* pvPoint)
  {
    vPoint[0] = pvPoint[0];
    vPoint[1] = pvPoint[1];
  }
  //////////////////////////////////////////////////////////////////////////
  _FrustumPlanes::_FrustumPlanes(const float4x4& m)
  {
    set(m);
  }

  _FrustumPlanes& _FrustumPlanes::set(const float4x4& m)
  {
    const float4 c1(m._11, m._21, m._31, m._41);
    const float4 c2(m._12, m._22, m._32, m._42);
    const float4 c3(m._13, m._23, m._33, m._43);
    const float4 c4(m._14, m._24, m._34, m._44);

    planes[0] = c4 + c1;  // Left
    planes[1] = c4 - c1;  // Right
    planes[2] = c4 + c2;  // Bottom
    planes[3] = c4 - c2;  // Top
    planes[4] = c3;       // Near
    planes[5] = c4 - c3;  // Far

    for(int i = 0; i < 6; i++) {
      planes[i].normalize();
    }
    return *this;
  }

  int _FrustumPlanes::IntersectAABB(const _AABBF& aabb) const
  {
    return IntersectAABBFrustum(aabb, *this);
  }

  _triangle::_triangle()
  {
  }

  _triangle::_triangle( const _float3* aVertices )
  {
		_float3* m = *this;
    m[0] = aVertices[0];
    m[1] = aVertices[1];
    m[2] = aVertices[2];
  }

  _triangle::_triangle( const _float3& _PA, const _float3& _PB, const _float3& _PC )
  {
		_float3* m = *this;
		m[0] = _PA;
    m[1] = _PB;
    m[2] = _PC;
  }

  _triangle::_triangle( const _float3* aVertices, int _PA, int _PB, int _PC )
  {
		_float3* m = *this;
    m[0] = aVertices[_PA];
    m[1] = aVertices[_PB];
    m[2] = aVertices[_PC];
  }

	_triangle::operator _float3*() const
	{
		return (_float3*)this;
	}

  _triangle& _triangle::set( const _float3* aVertices )
  {
		_float3* m = *this;
		m[0] = aVertices[0];
    m[1] = aVertices[1];
    m[2] = aVertices[2];
    return *this;
  }

  _triangle& _triangle::set(const _float3& _PA, const _float3& _PB, const _float3& _PC)
  {
		_float3* m = *this;
		m[0] = _PA;
    m[1] = _PB;
    m[2] = _PC;
    return *this;
  }

  _triangle& _triangle::set( const _float3* aVertices, int _PA, int _PB, int _PC )
  {
		_float3* m = *this;
		m[0] = aVertices[_PA];
    m[1] = aVertices[_PB];
    m[2] = aVertices[_PC];
    return *this;
  }

  _triangle& _triangle::flip()
  {
		_float3* m = *this;
    clSwap(m[0], m[2]);
    return *this;
  }

  clstd::_float3 _triangle::normal() const
  {
    return _float3::cross(B - A, C - B);
  }

  clstd::_AABBF _triangle::bounding() const
  {
    _AABBF aabb;
    aabb.Clear();
    aabb.AppendVertex(A);
    aabb.AppendVertex(B);
    aabb.AppendVertex(C);
    return aabb;
  }

} // namespace clstd
