#ifndef _GEOMETRY_OPERATION_H_
#define _GEOMETRY_OPERATION_H_

namespace clstd
{
  namespace geometry
  {
    enum Test
    {
      GOT_Inside = -1,
      GOT_Outside = 0,
      GOT_Intersect = 1,
    };

    struct SPLITTRIBYPLANE
    {
      const Plane*  /*IN*/ pPlane;      // 切分平面
      const float3  /*IN*/ vTri[3];     // 源三角形的顶点
      float         /*IN*/ fEpsilon;    // 容差
      struct SPLITEDTRI
      {
        float3    vTri[6];      // 每份最多两个三角形
        int       nTri;
      }/*OUT*/ SplitTri[2];     // 分割成两份
    };
    struct MOVINGCONTENT
    {
      const float3&   vMoveDir;
      const float     fMoveDist;
      float3*          pvHit;
      float*          pfFraction;

      MOVINGCONTENT(const float3&  _vMoveDir, const float _fMoveDist,
        float3* _pvHit, float* _pfFraction)
        : vMoveDir(_vMoveDir)
        , fMoveDist(_fMoveDist)
        , pvHit(_pvHit)
        , pfFraction(_pfFraction)
      {
      }
    };

    struct MOVE_LINESEG_TO_LINESEG
    {
      const float3& vLineSegA0;
      const float3& vLineSegA1;
      const float3& vLineSegB0;
      const float3& vLineSegB1;

      MOVE_LINESEG_TO_LINESEG(const float3 *pLineSegA, const float3 *pLineSegB)
        : vLineSegA0(pLineSegA[0])
        , vLineSegA1(pLineSegA[1])
        , vLineSegB0(pLineSegB[0])
        , vLineSegB1(pLineSegB[1])
      {
      }
      MOVE_LINESEG_TO_LINESEG(const float3& _vLineSegA0, const float3& _vLineSegA1,
        const float3& _vLineSegB0, const float3& _vLineSegB1)
        : vLineSegA0(_vLineSegA0)
        , vLineSegA1(_vLineSegA1)
        , vLineSegB0(_vLineSegB0)
        , vLineSegB1(_vLineSegB1)
      {
      }
    };

    bool MovePointToPlane(
      const float3& vPoint,
      const float3& vPlaneNormal, const float fPlaneDist,
      const float3& vMoveDir, const float fMoveDist,
      float3* vHit, float* pFraction);

    bool MoveAABBToTriangle(
      const AABB& aabb,
      const float3& vTri0, const float3& vTri1, const float3& vTri2,
      MOVINGCONTENT& mc, clvector<float3>& aDbgLineSeg);

    bool MoveLineSegToLineSeg(const MOVE_LINESEG_TO_LINESEG& mls2ls,
      MOVINGCONTENT& mc);

    bool IntersectRayWithTriangle(const NormalizedRay& ray, const float3& v0, const float3& v1, const float3& v2, float& t, float& u, float& v);
    bool IntersectAABBAABB(const AABB& aabbFirst, const AABB& aabbSecond);

    bool IsPointInWinding(const float3* vPos, const float3* vWinding, int nNumOfPoints);
    bool SplitTriByPlane(SPLITTRIBYPLANE* pSplitTriByPlane);

    // 射线与面求交, 反正两面
    bool RayIntersectPlane2S(const Ray& ray, const Plane& plane, float* fDist);
    bool RayIntersectPlane2S(const Ray& ray, const Plane& plane, float3* vHit);

    // 射线与AABB求交
    int IntersectRayAABB(const Ray& ray, const AABB& aabb, float* fDist, float3* pNormal);  // 0: no intersect -1: inside, 1: Intersect

    // 射线与AABB求交, 返回射出面
    bool RayIntersectAABB_Out(const Ray& ray, const AABB& aabb, float* fDist, float3* pBackNormal);


    int IntersectAABBFrustum(const AABB &aabb, const FrustumPlanes &f);
    //int IntersectAABBFrustum        (const AABB* pVolumeA, const Frustum* pVolumeB);
    //int IntersectOrientedBoxFrustum (const OBB* pVolumeA, const Frustum* pVolumeB);
    //bool AABBOverlapAABB(const AABB& aabbFirst, const AABB& aabbSecond);

    bool MovePointToPlane    (const float3& vPoint, const Plane& Plane,
      const float3& vDir, const float fDeltaDist, float* pfMoveDelta);

    bool MoveLineSegToPlane    (const LineSegment& LineSeg, const Plane& Plane,
      const float3& vDir, const float fDeltaDist, float* pfMoveDelta);

    bool MoveLineSegToLineSeg  (const LineSegment& LineSeg, const LineSegment& RefLineSeg,
      const float3& vDir, const float fDeltaDist, float* pfMoveDelta);

  } // namespace geometry
} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GEOMETRY_OPERATION_H_