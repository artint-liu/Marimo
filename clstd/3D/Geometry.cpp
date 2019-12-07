#include "../clstd.h"
#include "clString.h"
#include "clUtility.h"
namespace clstd
{
  namespace geometry
  {
    int AABB::IntersectAABB(const AABB& aabb) const
    {
      return IntersectAABBAABB(aabb, *this);
    }

    int AABB::IntersectFrustum(const FrustumPlanes& frustum) const
    {
      return IntersectAABBFrustum(*this, frustum);
    }

    int AABB::IntersectRay(const NormalizedRay& ray, float* fDist, float3* pNormal) const
    {
      return IntersectRayAABB(ray, *this, fDist, pNormal);
    }

    //////////////////////////////////////////////////////////////////////////
    Ray::Ray(const NormalizedRay& nr)
      : vOrigin(nr.vOrigin)
      , vDirection(nr.vDirection)
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Plane::Plane()
      : vNormal(0.0f)
      , fDist(0.0f)
    {
    }

    Plane::Plane(const float3* aVert3)
    {
      set(aVert3[0], aVert3[1], aVert3[2]);
    }

    Plane::Plane(const float4& vNormalDist)
    {
      vNormal.set(vNormalDist.x, vNormalDist.y, vNormalDist.z);
      fDist = vNormalDist.w;
    }

    Plane::Plane(const float3& vVert0, const float3& vVert1, const float3& vVert2)
    {
      vNormal = float3::cross(vVert1 - vVert0, vVert2 - vVert1);
      vNormal.normalize();
      fDist = vNormal.dot(vVert1);
    }

    Plane::Plane(const Plane& plane)
      : vNormal(plane.vNormal)
      , fDist(plane.fDist)
    {
    }

    Plane::Plane(const float3& _vNormal, const float _fDist)
      : vNormal(_vNormal)
      , fDist(_fDist)
    {
      //if(fDist < 0)
      //{
      //  fDist = -fDist;
      //  vNormal = -vNormal;
      //}
    }

    Plane& Plane::normalize()
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
    Plane Plane::normalize(const Plane& plane)
    {
      Plane p = plane;
      return p.normalize();
    }

    void Plane::set(const float3* aVert3)
    {
      set(aVert3[0], aVert3[1], aVert3[2]);
    }

    void Plane::set(const float3& vVert0, const float3& vVert1, const float3& vVert2)
    {
      vNormal = float3::cross(vVert1 - vVert0, vVert2 - vVert1).normalize();
      fDist = vNormal.dot(vVert1);
    }

    void Plane::set(const Plane& plane)
    {
      vNormal = plane.vNormal;
      fDist   = plane.fDist;
    }

    void Plane::set(const float3& _vNormal, const float _fDist)
    {
      vNormal = _vNormal;
      fDist = _fDist;
    }
    //////////////////////////////////////////////////////////////////////////
    Ray::Ray(const float3& _vOrigin, const float3& _vDirection)
      : vOrigin(_vOrigin)
      , vDirection(_vDirection)
    {
    }

    NormalizedRay::NormalizedRay(const float3& _vOrigin, const float3& _vDirection)
      : vOrigin   (_vOrigin)
      , vDirection(_vDirection)
    {
      vDirection.normalize();
    }

    //////////////////////////////////////////////////////////////////////////
    LineSegment::LineSegment(const float3& vPoint0, const float3& vPoint1)
    {
      vPoint[0] = vPoint0;
      vPoint[1] = vPoint1;
    }
    LineSegment::LineSegment(const float3* pvPoint)
    {
      vPoint[0] = pvPoint[0];
      vPoint[1] = pvPoint[1];
    }
    //////////////////////////////////////////////////////////////////////////
    FrustumPlanes::FrustumPlanes(const float4x4& m)
    {
      set(m);
    }

    FrustumPlanes& FrustumPlanes::set(const float4x4& m)
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

    int FrustumPlanes::IntersectAABB(const AABB& aabb) const
    {
      return IntersectAABBFrustum(aabb, *this);
    }

    Triangle::Triangle()
    {
    }

    Triangle::Triangle(const _float3* aVertices)
    {
      _float3* m = *this;
      m[0] = aVertices[0];
      m[1] = aVertices[1];
      m[2] = aVertices[2];
    }

    Triangle::Triangle(const _float3& _PA, const _float3& _PB, const _float3& _PC)
    {
      _float3* m = *this;
      m[0] = _PA;
      m[1] = _PB;
      m[2] = _PC;
    }

    Triangle::Triangle(const _float3* aVertices, int _PA, int _PB, int _PC)
    {
      _float3* m = *this;
      m[0] = aVertices[_PA];
      m[1] = aVertices[_PB];
      m[2] = aVertices[_PC];
    }

    Triangle::operator _float3*() const
    {
      return (_float3*)this;
    }

    Triangle& Triangle::set(const _float3* aVertices)
    {
      _float3* m = *this;
      m[0] = aVertices[0];
      m[1] = aVertices[1];
      m[2] = aVertices[2];
      return *this;
    }

    Triangle& Triangle::set(const _float3& _PA, const _float3& _PB, const _float3& _PC)
    {
      _float3* m = *this;
      m[0] = _PA;
      m[1] = _PB;
      m[2] = _PC;
      return *this;
    }

    Triangle& Triangle::set(const _float3* aVertices, int _PA, int _PB, int _PC)
    {
      _float3* m = *this;
      m[0] = aVertices[_PA];
      m[1] = aVertices[_PB];
      m[2] = aVertices[_PC];
      return *this;
    }

    Triangle& Triangle::flip()
    {
      _float3* m = *this;
      clSwap(m[0], m[2]);
      return *this;
    }

    _float3 Triangle::normal() const
    {
      return _float3::cross(B - A, C - B);
    }

    AABB Triangle::bounding() const
    {
      AABB aabb;
      aabb.Clear();
      aabb.AppendVertex(A);
      aabb.AppendVertex(B);
      aabb.AppendVertex(C);
      return aabb;
    }

  } // namespace geomerty
} // namespace clstd
