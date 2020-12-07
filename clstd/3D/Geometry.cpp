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

    // 测试一个B面三点全部在A面一侧
    bool TestOneSide(float3 a, float3 b, float3 c, float3 a2, float3 b2, float3 c2)
    {
      float3 n = float3::cross(b - a, c - a);
      float check = float3::dot(n, a);
      float ca2 = float3::dot(n, a2);
      float cb2 = float3::dot(n, b2);
      float cc2 = float3::dot(n, c2);
      return ((ca2 > check && cb2 > check && cc2 > check) || (ca2 < check && cb2 < check && cc2 < check));
    }

    // 测试异面直线AB，A2B2中分平面，且两个三角形恰好在面两侧
    bool TestEachSide(float3 a, float3 b, float3 c, float3 a2, float3 b2, float3 c2)
    {
      float3 n = float3::cross(b - a, b2 - a2);
      float ca1 = float3::dot(n, a);
      float ca2 = float3::dot(n, a2);
      float mid = (ca1 + ca2) * 0.5f;
      float cc1 = float3::dot(n, c);
      float cc2 = float3::dot(n, c2);

      return (ca1 < mid && cc1 < ca1 && ca2 > mid && cc2 > ca2) || (ca1 > mid && cc1 > ca1 && ca2 < mid && cc2 < ca2);
    }

    bool TestSamePlane(float3 a, float3 b, float3 c, float3 a2, float3 b2, float3 c2)
    {
      float3 n = float3::cross(b - a, c - a);
      float3 n2 = float3::cross(b2 - a2, c2 - a2);
      float3 nt = float3::cross(n, n2);
      return (nt.x == 0 && nt.y == 0 && nt.z == 0 && float3::dot(n, a) == float3::dot(n2, a2));
    }


    bool TestSamePlaneOneSide(float3 n, float3 a, float3 b, float3 a2, float3 b2, float3 c2)
    {
      float3 nn = float3::cross(n, a - b); // 这个有方向要求，需要指向三角形外侧, TODO: 这个方向没测试过
      float check = float3::dot(nn, a);
      float ca2 = float3::dot(nn, a2);
      float cb2 = float3::dot(nn, b2);
      float cc2 = float3::dot(nn, c2);
      return ca2 > check && cb2 > check && cc2 > check;
    }

    b32 Triangle::intersect(const Triangle& t)
    {
      if (TestSamePlane(A, B, C, t.A, t.B, t.C))
      {
        float3 n = float3::cross(B - A, C - A);
        float3 n2 = float3::cross(t.B - t.A, t.C - t.A);

        bool bOutOfEdge = 
          TestSamePlaneOneSide(n, A, B, t.A, t.B, t.C) ||
          TestSamePlaneOneSide(n, B, C, t.A, t.B, t.C) ||
          TestSamePlaneOneSide(n, C, A, t.A, t.B, t.C) ||
          TestSamePlaneOneSide(n2, t.A, t.B, A, B, C) ||
          TestSamePlaneOneSide(n2, t.B, t.C, A, B, C) ||
          TestSamePlaneOneSide(n2, t.C, t.A, A, B, C);

        return !bOutOfEdge;
      }
      else
      {
        // 求存在一个面，一个三角形在面上，另一个三角形在面一侧
        // 或者存在一个面，两个三角形恰好在面的两侧
        bool bHasPlane = TestOneSide(A, B, C, t.A, t.B, t.C) || TestOneSide(t.A, t.B, t.C, A, B, C);
        bHasPlane = bHasPlane ||
          TestEachSide(A, B, C, t.A, t.B, t.C) ||
          TestEachSide(A, B, C, t.B, t.C, t.A) ||
          TestEachSide(A, B, C, t.C, t.A, t.B) ||
          TestEachSide(B, C, A, t.A, t.B, t.C) ||
          TestEachSide(B, C, A, t.B, t.C, t.A) ||
          TestEachSide(B, C, A, t.C, t.A, t.B) ||
          TestEachSide(C, A, B, t.A, t.B, t.C) ||
          TestEachSide(C, A, B, t.B, t.C, t.A) ||
          TestEachSide(C, A, B, t.C, t.A, t.B);
        return !bHasPlane;
      }
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
