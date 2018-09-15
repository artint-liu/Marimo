#include "../clstd.h"
//#include "../FloatX.h"
//#include "Geometry.h"
//#include "GeoOp.h"

namespace clstd
{
  namespace geometry
  {
  bool SplitTriByPlane(SPLITTRIBYPLANE* pSplitTriByPlane)
  {
    float3 vClip[2][4];
    float aDelta[3];
    int i, n;
    int iNum[2] = {0, 0};
    const float3& vNormal = pSplitTriByPlane->pPlane->vNormal;
    const float&  fDist   = pSplitTriByPlane->pPlane->fDist;
    const float3* vFace   = pSplitTriByPlane->vTri;

    for(i = 0; i < 3; i++)
    {
      aDelta[i] = float3::dot(vNormal, vFace[i]) - fDist;
      if(fabs(aDelta[i]) < pSplitTriByPlane->fEpsilon)  
        aDelta[i] = 0;
    }
    for(i = 0; i < 3; i++)
    {
      n = (i == 2 ? 0 : (i + 1));
      if(aDelta[i] >= 0)
        vClip[0][iNum[0]++] = vFace[i];
      if(aDelta[i] <= 0)
        vClip[1][iNum[1]++] = vFace[i];

      if( (aDelta[i] > 0 && aDelta[n] < 0) ||
        (aDelta[i] < 0 && aDelta[n] > 0) )
      {
        float fDelta = aDelta[i] / (aDelta[i] - aDelta[n]);
        vClip[0][iNum[0]++] = 
          vClip[1][iNum[1]++] = (vFace[n] - vFace[i]) * fDelta + vFace[i];
      }
    }
    if(aDelta[0] == 0 && aDelta[1] == 0 && aDelta[2] == 0)
      iNum[0] = 0;

    ASSERT(iNum[1] >= 0 && iNum[1] <= 4);
    ASSERT(iNum[0] >= 0 && iNum[0] <= 4);
    //ASSERT(
    //  (iNum[0] == 3 && iNum[1] == 4) ||
    //  (iNum[0] == 4 && iNum[1] == 3));
    SPLITTRIBYPLANE::SPLITEDTRI* pSplitedTri = pSplitTriByPlane->SplitTri;
    for(i = 0; i < 2; i++)
    {
      pSplitedTri->nTri = 0;
      if(iNum[i] == 3 || iNum[i] == 4)
      {
        pSplitedTri->vTri[0] = vClip[i][0];
        pSplitedTri->vTri[1] = vClip[i][1];
        pSplitedTri->vTri[2] = vClip[i][2];
        pSplitedTri->nTri++;
        if(iNum[i] == 4)
        {
          pSplitedTri->vTri[3] = vClip[i][0];
          pSplitedTri->vTri[4] = vClip[i][2];
          pSplitedTri->vTri[5] = vClip[i][3];
          pSplitedTri->nTri++;
        }
      }
      pSplitedTri++;
    }
    return pSplitTriByPlane->SplitTri[0].nTri > 0 && pSplitTriByPlane->SplitTri[1].nTri > 0;
  }

  bool RayIntersectPlane2S(const Ray& ray, const Plane& plane, float* fDist)
  {
    const float fCosAngle = float3::dot(plane.vNormal, ray.vDirection);
    if(fCosAngle == 0) {
      return false;
    }
    const float fFactor = float3::dot(plane.vNormal, ray.vOrigin) ;
    if(fDist) {
      *fDist = (plane.fDist - fFactor) / fCosAngle;
    }
    return true;
  }

  bool RayIntersectPlane2S(const Ray& ray, const Plane& plane, float3* vHit)
  {
    float fDist;
    if(RayIntersectPlane2S(ray, plane, &fDist)) {
      *vHit = ray.vOrigin + ray.vDirection * fDist;
      return true;
    }
    return false;
  }

  int IntersectRayAABB(const Ray& ray, const AABB& aabb, float* fDist, float3* pNormal)
  {
    //const float fEpsilon = 1.0f;
    float fLen;
    
    if(aabb.IsPointIn(ray.vOrigin)) {
      if(fDist) *fDist = 0;
      if(pNormal) pNormal->set(0,0,0);
      return geometry::GOT_Inside;
    }

    if(ray.vDirection.y < 0 && ray.vOrigin.y > aabb.vMax.y &&
      RayIntersectPlane2S(ray, Plane( float3::AxisY, aabb.vMax.y), &fLen) != false)
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;
      
      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
         zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
        //vHit >= float3(aabb.vMin.x, aabb.vMax.y - fEpsilon, aabb.vMin.z) &&
        //vHit <= float3(aabb.vMax.x, aabb.vMax.y + fEpsilon, aabb.vMax.z))
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = float3::AxisY;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }
    if(ray.vDirection.y > 0 && ray.vOrigin.y < aabb.vMin.y &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisY,-aabb.vMin.y), &fLen) != false )
      //vHit >= float3(aabb.vMin.x, aabb.vMin.y - fEpsilon, aabb.vMin.z) &&
      //vHit <= float3(aabb.vMax.x, aabb.vMin.y + fEpsilon, aabb.vMax.z))
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
         zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = -float3::AxisY;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }

    if(ray.vDirection.x < 0 && ray.vOrigin.x > aabb.vMax.x &&
      RayIntersectPlane2S(ray, Plane( float3::AxisX, aabb.vMax.x), &fLen) != false )
      //vHit >= float3(aabb.vMax.x - fEpsilon, aabb.vMin.y, aabb.vMin.z) &&
      //vHit <= float3(aabb.vMax.x + fEpsilon, aabb.vMax.y, aabb.vMax.z))
    {
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(yHit >= aabb.vMin.y && yHit <= aabb.vMax.y && 
         zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = float3::AxisX;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }
    if(ray.vDirection.x > 0 && ray.vOrigin.x < aabb.vMin.x &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisX, -aabb.vMin.x), &fLen) != false )
      //vHit >= float3(aabb.vMin.x - fEpsilon, aabb.vMin.y, aabb.vMin.z) &&
      //vHit <= float3(aabb.vMin.x + fEpsilon, aabb.vMax.y, aabb.vMax.z))
    {
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(yHit >= aabb.vMin.y && yHit <= aabb.vMax.y && 
         zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = -float3::AxisX;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }

    if(ray.vDirection.z < 0 && ray.vOrigin.z > aabb.vMax.z &&
      RayIntersectPlane2S(ray, Plane( float3::AxisZ, aabb.vMax.z), &fLen) != false )
      //vHit >= float3(aabb.vMin.x, aabb.vMin.y, aabb.vMax.z - fEpsilon) &&
      //vHit <= float3(aabb.vMax.x, aabb.vMax.y, aabb.vMax.z + fEpsilon))
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
         yHit >= aabb.vMin.y && yHit <= aabb.vMax.y )
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = float3::AxisZ;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }
    if(ray.vDirection.z > 0 && ray.vOrigin.z < aabb.vMin.z &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisZ, -aabb.vMin.z), &fLen) != false )
      //vHit >= float3(aabb.vMin.x, aabb.vMin.y, aabb.vMin.z - fEpsilon) &&
      //vHit <= float3(aabb.vMax.x, aabb.vMax.y, aabb.vMin.z + fEpsilon))
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
         yHit >= aabb.vMin.y && yHit <= aabb.vMax.y )
      {
        ASSERT(fLen >= 0.0f);
        if(pNormal) *pNormal = -float3::AxisZ;
        if(fDist) *fDist = fLen;
        return geometry::GOT_Intersect;
      }
    }
    return geometry::GOT_Outside;
  }

  bool RayIntersectAABB_Out(const Ray& ray, const AABB& aabb, float* fDist, float3* pBackNormal)
  {
    //const float fEpsilon = 1.0f;
    float fLen;

    if(ray.vDirection.y > 0 &&
      RayIntersectPlane2S(ray, Plane( float3::AxisY, aabb.vMax.y), &fLen) != false)
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
        zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        if(pBackNormal) *pBackNormal = float3::AxisY;
        if(fDist) *fDist = fLen;
        return true;
      }
    }

    if(ray.vDirection.y < 0 &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisY,-aabb.vMin.y), &fLen) != false )
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
        zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        if(pBackNormal) *pBackNormal = -float3::AxisY;
        if(fDist) *fDist = fLen;
        return true;
      }
    }

    if(ray.vDirection.x > 0 &&
      RayIntersectPlane2S(ray, Plane( float3::AxisX, aabb.vMax.x), &fLen) != false )
    {
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(yHit >= aabb.vMin.y && yHit <= aabb.vMax.y && 
        zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        if(pBackNormal) *pBackNormal = float3::AxisX;
        if(fDist) *fDist = fLen;
        return true;
      }
    }

    if(ray.vDirection.x < 0 &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisX, -aabb.vMin.x), &fLen) != false )
    {
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;
      const float zHit = ray.vOrigin.z + fLen * ray.vDirection.z;

      if(yHit >= aabb.vMin.y && yHit <= aabb.vMax.y && 
        zHit >= aabb.vMin.z && zHit <= aabb.vMax.z )
      {
        if(pBackNormal) *pBackNormal = -float3::AxisX;
        if(fDist) *fDist = fLen;
        return true;
      }
    }

    if(ray.vDirection.z > 0 &&
      RayIntersectPlane2S(ray, Plane( float3::AxisZ, aabb.vMax.z), &fLen) != false )
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
        yHit >= aabb.vMin.y && yHit <= aabb.vMax.y )
      {
        if(pBackNormal) *pBackNormal = float3::AxisZ;
        if(fDist) *fDist = fLen;
        return true;
      }
    }
    if(ray.vDirection.z < 0 &&
      RayIntersectPlane2S(ray, Plane(-float3::AxisZ, -aabb.vMin.z), &fLen) != false )
    {
      const float xHit = ray.vOrigin.x + fLen * ray.vDirection.x;
      const float yHit = ray.vOrigin.y + fLen * ray.vDirection.y;

      if(xHit >= aabb.vMin.x && xHit <= aabb.vMax.x && 
        yHit >= aabb.vMin.y && yHit <= aabb.vMax.y )
      {
        if(pBackNormal) *pBackNormal = -float3::AxisZ;
        if(fDist) *fDist = fLen;
        return true;
      }
    }
    return false;
  }

  bool MovePointToPlane(const float3& vPoint, const Plane& plane, 
    const float3& vDir, const float fDeltaDist, float* pfMoveDelta)
  {
    return true;
  }

  bool MoveLineSegToPlane(const LineSegment& LineSeg, const Plane& plane, 
    const float3& vDir, const float fDeltaDist, float* pfMoveDelta)
  {
    return true;
  }

  bool MoveLineSegToLineSeg(const LineSegment& LineSeg, const LineSegment& RefLineSeg, 
    const float3& vDir, const float fDeltaDist, float* pfMoveDelta)
  {
    return true;
  }

  bool MovePointToPlane(
    const float3& vPoint, 
    const float3& vPlaneNormal, const float fPlaneDist, 
    const float3& vMoveDir, const float fMoveDist, 
    float3* vHit, float* pFraction)
  {
    const float fDistFront = float3::dot(vPoint, vPlaneNormal);
    if(fDistFront < fPlaneDist)
      return false;

    float3 vDestPos = vPoint + vMoveDir * fMoveDist;

    const float fDistBack = float3::dot(vDestPos, vPlaneNormal);
    if(fDistBack > fPlaneDist)
    {
      if(pFraction != NULL)
        *pFraction = fMoveDist;
      return false;
    }
    float fFraction = (fDistFront - fPlaneDist) / (fDistFront - fDistBack) * fMoveDist;
    if(pFraction != NULL)
      *pFraction = fFraction;
    if(vHit != NULL)
      *vHit = vPoint + fFraction * vMoveDir;

    return true;
  }

  bool MoveLineSegToLineSeg(const MOVE_LINESEG_TO_LINESEG& mls2ls,
    MOVINGCONTENT& mc)
  {
    const float3 vLineA = mls2ls.vLineSegA1 - mls2ls.vLineSegA0;
    const float3 vLineB = mls2ls.vLineSegB1 - mls2ls.vLineSegB0;

    float3 vPlaneDirA = float3::cross(vLineA, mc.vMoveDir);

    if(vPlaneDirA == 0.0f)
      return false;

    const float fPlaneDistA = float3::dot(vPlaneDirA, mls2ls.vLineSegA0);

    const float fDistFrontA = float3::dot(vPlaneDirA, mls2ls.vLineSegB0);
    const float fDistBackA = float3::dot(vPlaneDirA, mls2ls.vLineSegB1);

    if((fDistFrontA < fPlaneDistA || fDistBackA > fPlaneDistA) &&
      (fDistFrontA > fPlaneDistA || fDistBackA < fPlaneDistA))
      return false;

    float3 vPlaneDirB = float3::cross(mc.vMoveDir, vLineB);

    if(vPlaneDirB == 0.0f)
      return false;

    const float fPlaneDistB = float3::dot(vPlaneDirB, mls2ls.vLineSegB0);

    const float fDistFrontB = float3::dot(vPlaneDirB, mls2ls.vLineSegA0);
    const float fDistBackB = float3::dot(vPlaneDirB, mls2ls.vLineSegA1);

    if((fDistFrontB < fPlaneDistB || fDistBackB > fPlaneDistB) &&
      (fDistFrontB > fPlaneDistB || fDistBackB < fPlaneDistB) )
      return false;

    const float3 vIntersectA = ((fDistFrontA - fPlaneDistA) / (fDistFrontA - fDistBackA)) * vLineB + mls2ls.vLineSegB0;
    const float3 vIntersectB = ((fDistFrontB - fPlaneDistB) / (fDistFrontB - fDistBackB)) * vLineA + mls2ls.vLineSegA0;

    const float fDist = float3::dot(mc.vMoveDir, vIntersectA) - float3::dot(mc.vMoveDir, vIntersectB);
    if(fDist < 0 || fDist > mc.fMoveDist)
      return false;

    if(mc.pfFraction != NULL)
    {
      *mc.pfFraction = (vIntersectB - vIntersectA).length();
    }
    if(mc.pvHit != NULL)
    {
      *mc.pvHit = vIntersectA;
    }

    return true;
  }

  bool IsPointInWinding(const float3* vPos, const float3* vWinding, int nNumOfPoints)
  {
    float3 vDeltaA = vWinding[nNumOfPoints - 1] - *vPos;
    float3 vDeltaB = vWinding[0] - *vPos;
    bool bSign = float3::cross(vDeltaA, vDeltaB).z > 0;
    for(int i = 0; i < nNumOfPoints - 1; i++)
    {
      vDeltaA = vDeltaB;
      vDeltaB = vWinding[i + 1] - *vPos;
      if((float3::cross(vDeltaA, vDeltaB).z > 0) != bSign)
        return false;
    }
    return true;
  }

  bool _TriangleHitAABBPlaneP(const AABB& aabb, const AABB& aabbTri, const float3* aTriPoint, 
    const float3& vMoveDir, const float fMoveDist, 
    const int nCheckAxis, const int nRangeAxisA, const int nRangeAxisB, 
    float3& vHit, float& fFraction)
  {
    float3 vPlaneAxis(0.0f);
    vPlaneAxis.m[nCheckAxis] = 1.0f;
    const float fPlaneDist = (aabb.vMax.m[nCheckAxis]);

    for(int i = 0; i < 3; i++)
    {
      if(aabbTri.vMin.m[nCheckAxis] == aTriPoint[i].m[nCheckAxis])
      {
        if(MovePointToPlane(aTriPoint[i], vPlaneAxis, fPlaneDist, vMoveDir, fMoveDist, &vHit, &fFraction) && 
          vHit.m[nRangeAxisA] >= aabb.vMin.m[nRangeAxisA] && vHit.m[nRangeAxisA] <= aabb.vMax.m[nRangeAxisA] && 
          vHit.m[nRangeAxisB] >= aabb.vMin.m[nRangeAxisB] && vHit.m[nRangeAxisB] <= aabb.vMax.m[nRangeAxisB])
          return true;
      }
    }
    return false;
  }

  bool _TriangleHitAABBPlaneN(const AABB& aabb, const AABB& aabbTri, const float3* aTriPoint, 
    const float3& vMoveDir, const float fMoveDist, 
    const int nCheckAxis, const int nRangeAxisA, const int nRangeAxisB, 
    float3& vHit, float& fFraction)
  {
    float3 vPlaneAxis(0.0f);
    vPlaneAxis.m[nCheckAxis] = -1.0f;
    const float fPlaneDist = -(aabb.vMin.m[nCheckAxis]);

    for(int i = 0; i < 3; i++)
    {
      if(aabbTri.vMax.m[nCheckAxis] == aTriPoint[i].m[nCheckAxis])
      {
        if(MovePointToPlane(aTriPoint[i], vPlaneAxis, fPlaneDist, vMoveDir, fMoveDist, &vHit, &fFraction) && 
          vHit.m[nRangeAxisA] >= aabb.vMin.m[nRangeAxisA] && vHit.m[nRangeAxisA] <= aabb.vMax.m[nRangeAxisA] && 
          vHit.m[nRangeAxisB] >= aabb.vMin.m[nRangeAxisB] && vHit.m[nRangeAxisB] <= aabb.vMax.m[nRangeAxisB])
          return true;
      }
    }
    return false;
  }

  bool MoveAABBToTriangle(
    const AABB& aabb,
    const float3& vTri0, const float3& vTri1, const float3& vTri2,
    MOVINGCONTENT& mc, clvector<float3>& aDbgLineSeg)
  {
    const float3& vMoveDir = mc.vMoveDir;
    const float3 aTriPoint[3] = {vTri0, vTri1, vTri2};
    const float3 vPlaneNormal = float3::cross(vTri1 - vTri0, vTri2 - vTri1);
    const float fPlaneDist = float3::dot(vPlaneNormal, vTri0);

    float3 vHit;
    float fFraction, fMinFraction;
    int i, n;


    if(vMoveDir == 0.0f)
      return false;

    float3 aPoints[8];
    CLDWORD dwPointMask = 0;
    aPoints[0].set(aabb.vMin.x, aabb.vMin.y, aabb.vMin.z);
    aPoints[1].set(aabb.vMax.x, aabb.vMin.y, aabb.vMin.z);
    aPoints[2].set(aabb.vMax.x, aabb.vMin.y, aabb.vMax.z);
    aPoints[3].set(aabb.vMin.x, aabb.vMin.y, aabb.vMax.z);

    aPoints[4].set(aabb.vMin.x, aabb.vMax.y, aabb.vMin.z);
    aPoints[5].set(aabb.vMax.x, aabb.vMax.y, aabb.vMin.z);
    aPoints[6].set(aabb.vMax.x, aabb.vMax.y, aabb.vMax.z);
    aPoints[7].set(aabb.vMin.x, aabb.vMax.y, aabb.vMax.z);

    if(vPlaneNormal.y > 0)      dwPointMask |= 0xf000;
    else if(vPlaneNormal.y < 0) dwPointMask |= 0x0f00;

    if(vPlaneNormal.z > 0)      dwPointMask |= 0xCC00;
    else if(vPlaneNormal.z < 0) dwPointMask |= 0x3300;

    if(vPlaneNormal.x > 0)      dwPointMask |= 0x6600;
    else if(vPlaneNormal.x < 0) dwPointMask |= 0x9900;

    AABB aabbTri;
    //aabbTri.Clear();
    aabbTri.AppendVertex(aTriPoint[0]);
    aabbTri.AppendVertex(aTriPoint[1]);
    aabbTri.AppendVertex(aTriPoint[2]);

    for(i = 0; i < 8; i++)
    {
      if((dwPointMask & (0x100 << i)) != 0)
        continue;

      if(MovePointToPlane(aPoints[i], vPlaneNormal, fPlaneDist, mc.vMoveDir, mc.fMoveDist, &vHit, &fMinFraction) &&
        IsPointInWinding(&vHit, aTriPoint, 3))
        goto RET_HIT_CONTENT;
    }

    if(vMoveDir.y > 0)
    {
      dwPointMask |= 0xf0;
      if(_TriangleHitAABBPlaneP(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 1, 0, 2, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }
    else if(vMoveDir.y < 0)
    {
      dwPointMask |= 0x0f;
      if(_TriangleHitAABBPlaneN(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 1, 0, 2, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }

    if(vMoveDir.z > 0)
    {
      dwPointMask |= 0xCC;
      if(_TriangleHitAABBPlaneP(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 2, 0, 1, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }
    else if(vMoveDir.z < 0)
    {
      dwPointMask |= 0x33;
      if(_TriangleHitAABBPlaneN(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 2, 0, 1, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }

    if(vMoveDir.x > 0)
    {
      dwPointMask |= 0x66;
      if(_TriangleHitAABBPlaneP(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 0, 1, 2, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }
    else if(vMoveDir.x < 0)
    {
      dwPointMask |= 0x99;
      if(_TriangleHitAABBPlaneN(aabb, aabbTri, aTriPoint, -vMoveDir, mc.fMoveDist, 0, 1, 2, vHit, fMinFraction))
        goto RET_HIT_CONTENT;
    }


    static int aLineLink[24] = {
      0,1,1,2,2,3,3,0,
      4,5,5,6,6,7,7,4,
      0,4,1,5,2,6,3,7,
    };
    static int aTriLine[6] = {0,1,1,2,2,0};

    {
      MOVINGCONTENT mcEdge(vMoveDir, mc.fMoveDist, mc.pvHit, &fFraction);
      fMinFraction = 1e10f;

      for(i = 0; i < 12; i++)
      {
        const int nTip0 = aLineLink[i << 1];
        const int nTip1 = aLineLink[(i << 1) + 1];
        if((dwPointMask & (1 << nTip0)) && 
          (dwPointMask & (1 << nTip1)) )
        {
          for(n = 0; n < 3; n++)
          {
            if(MoveLineSegToLineSeg(
              MOVE_LINESEG_TO_LINESEG(
              aPoints[nTip0], aPoints[nTip1], 
              aTriPoint[aTriLine[n << 1]], aTriPoint[aTriLine[(n << 1) + 1]]), mcEdge))
            {
              if(fMinFraction > fFraction)
              {
                fMinFraction = fFraction;
                if(mc.pvHit != NULL)
                  vHit = *mc.pvHit;
              }
            }

          }
        }
      }
    }

    if(fMinFraction == 1e10f)
      return false;

RET_HIT_CONTENT:
    if(mc.pvHit != NULL)
      *mc.pvHit = vHit;
    if(mc.pfFraction != NULL)
      *mc.pfFraction = fMinFraction;

    return true;
  }

  //float3 XMVectorLess(const float3& V1, const float3& V2)
  //{
  //  float3 Control;
  //  Control.m[0] = (V1.m[0] < V2.m[0]) ? 0xFFFFFFFF : 0;
  //  Control.m[1] = (V1.m[1] < V2.m[1]) ? 0xFFFFFFFF : 0;
  //  Control.m[2] = (V1.m[2] < V2.m[2]) ? 0xFFFFFFFF : 0;
  //  //Control.m[3] = (V1.vector4_f32[3] < V2.vector4_f32[3]) ? 0xFFFFFFFF : 0;
  //  return Control;
  //}

  //float3 XMVectorOrInt(const float3& V1, const float3& V2)
  //{
  //  XMVECTOR Result;

  //  Result.vector4_u32[0] = V1.vector4_u32[0] | V2.vector4_u32[0];
  //  Result.vector4_u32[1] = V1.vector4_u32[1] | V2.vector4_u32[1];
  //  Result.vector4_u32[2] = V1.vector4_u32[2] | V2.vector4_u32[2];
  //  Result.vector4_u32[3] = V1.vector4_u32[3] | V2.vector4_u32[3];

  //  return Result;
  //}
//bool IntersectTriangle(const Vector3& orig, const Vector3& dir,
//9:                         Vector3& v0, Vector3& v1, Vector3& v2,
//10:                         float* t, float* u, float* v)
  bool IntersectRayWithTriangle(const NormalizedRay& ray, const float3& v0, const float3& v1, const float3& v2, float& t, float& u, float& v)
  {
    const float fEpsilon = 1e-5f;
    ASSERT(fabs(ray.vDirection.lengthsquare() - 1.0f) < fEpsilon);
    //float u, v;
    // E1
    float3 E1 = v1 - v0;

    // E2
    float3 E2 = v2 - v0;

    // P
    float3 P = ray.vDirection.cross(E2);

    // determinant
    float det = E1.dot(P);

    // keep det > 0, modify T accordingly
    float3 T;
    if( det > 0 )
    {
      T = ray.vOrigin - v0;
    }
    else
    {
      T = v0 - ray.vOrigin;
      det = -det;
    }

    // If determinant is near zero, ray lies in plane of triangle
    if( det < fEpsilon )
      return false;

    // Calculate u and make sure u <= 1
    u = T.dot(P);
    if( u < 0.0f || u > det )
        return false;

    // Q
    float3 Q = T.cross(E1);

    // Calculate v and make sure u + v <= 1
    v = ray.vDirection.dot(Q);
    if( v < 0.0f || u + v > det )
        return false;

    // Calculate t, scale parameters, ray intersects triangle
    t = E2.dot(Q);

    float fInvDet = 1.0f / det;
    t *= fInvDet;
    u *= fInvDet;
    v *= fInvDet;

    return true;
  }

  bool IntersectAABBAABB(const AABB& aabbFirst, const AABB& aabbSecond)
  {
    float3 vMin = Max(aabbFirst.vMin, aabbSecond.vMin);
    float3 vMax = Min(aabbFirst.vMax, aabbSecond.vMax);
    return vMin <= vMax;  // 这个"="以后修改要小心, 注意AABB是个薄片(vMin.x==vMax.x || vMin.y==vMax.y || vMin.z==vMax.z)的情况
  }

  /*void ComputeFrustumFromProjection( Frustum* pOut, float4x4* pProjection )
  {
    // TODO: 没测试
    ASSERT( pOut );
    ASSERT( pProjection );

    // Corners of the projection frustum in homogenous space.
    static float4 HomogenousPoints[6] =
    {
      float4(  1.0f,  0.0f, 1.0f, 1.0f ),   // right (at far plane)
      float4( -1.0f,  0.0f, 1.0f, 1.0f ),   // left
      float4(  0.0f,  1.0f, 1.0f, 1.0f ),   // top
      float4(  0.0f, -1.0f, 1.0f, 1.0f ),   // bottom

      float4( 0.0f, 0.0f, 0.0f, 1.0f   ),     // near
      float4( 0.0f, 0.0f, 1.0f, 1.0f   )      // far
    };

    float4x4 matInverse = float4x4::inverse(*pProjection);

    // Compute the frustum corners in world space.
    float4 Points[6];

    for( INT i = 0; i < 6; i++ )
    {
      // Transform point.
      Points[i] = HomogenousPoints[i].Transform(matInverse);
    }

    pOut->Origin = float3( 0.0f, 0.0f, 0.0f );
    pOut->Orientation = quaternion( 0.0f, 0.0f, 0.0f, 1.0f );

    // Compute the slopes.
    Points[0] = Points[0] * ( 1.0f / Points[0].z );
    Points[1] = Points[1] * ( 1.0f / Points[1].z );
    Points[2] = Points[2] * ( 1.0f / Points[2].z );
    Points[3] = Points[3] * ( 1.0f / Points[3].z );

    pOut->RightSlope  = Points[0].x;
    pOut->LeftSlope   = Points[1].x;
    pOut->TopSlope    = Points[2].y;
    pOut->BottomSlope = Points[3].y;

    // Compute near and far.
    Points[4] = Points[4] * ( 1.0f / Points[4].w );
    Points[5] = Points[5] * ( 1.0f / Points[5].w );

    pOut->Near = Points[4].z;
    pOut->Far  = Points[5].z;

    return;
  }//*/

  //-----------------------------------------------------------------------------
  // Exact axis alinged box vs frustum test.  Constructs an oriented box and uses
  // the oriented box vs frustum test.
  //
  // Return values: 0 = no intersection, 
  //                1 = intersection, 
  //                2 = box is completely inside frustum
  //-----------------------------------------------------------------------------
  //int IntersectAABBFrustum( const AABB* pVolumeA, const Frustum* pVolumeB )
  //{
  //  ASSERT( pVolumeA );
  //  ASSERT( pVolumeB );

  //  // Make the axis aligned box oriented and do an OBB vs frustum test.
  //  OBB BoxA;

  //  BoxA.Center = pVolumeA->GetCenter();
  //  BoxA.Extents = pVolumeA->GetExtent();
  //  BoxA.Orientation.x = 0.0f;
  //  BoxA.Orientation.y = 0.0f;
  //  BoxA.Orientation.z = 0.0f;
  //  BoxA.Orientation.w = 1.0f;

  //  return IntersectOrientedBoxFrustum( &BoxA, pVolumeB );
  //}
  
  int IntersectAABBFrustum(const AABB &aabb, const FrustumPlanes &f)
  {
    int result = geometry::GOT_Inside;

    for (int i = 0; i < 6; ++i) {
      const float4 &plane = f.planes[i];

      const float3 pv(
        plane.x > 0 ? aabb.vMax.x : aabb.vMin.x,
        plane.y > 0 ? aabb.vMax.y : aabb.vMin.y,
        plane.z > 0 ? aabb.vMax.z : aabb.vMin.z
        );

      const float3 nv(
        plane.x < 0 ? aabb.vMax.x : aabb.vMin.x,
        plane.y < 0 ? aabb.vMax.y : aabb.vMin.y,
        plane.z < 0 ? aabb.vMax.z : aabb.vMin.z
        );

      const float n = float4::dot(float4(pv, 1.0f), plane);
      if (n < 0) return geometry::GOT_Outside;

      const float m = float4::dot(float4(nv, 1.0f), plane);
      if (m < 0) result = geometry::GOT_Intersect;
    }

    return result;
  }
  /*
  //-----------------------------------------------------------------------------
  // Exact oriented box vs frustum test.
  // Return values: 0 = no intersection, 
  //                1 = intersection, 
  //                2 = box is completely inside frustum
  //-----------------------------------------------------------------------------
  int IntersectOrientedBoxFrustum( const OBB* pVolumeA, const Frustum* pVolumeB )
  {
    ASSERT( pVolumeA );
    ASSERT( pVolumeB );

    //static const XMVECTORI32 SelectY =
    //{
    //  XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0
    //};
    //static const XMVECTORI32 SelectZ =
    //{
    //  XM_SELECT_0, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0
    //};

    float4 Zero(0.0f);

    // Build the frustum planes.
    float4 Planes[6];
    Planes[0].set( 0.0f, 0.0f, -1.0f, pVolumeB->Near );
    Planes[1].set( 0.0f, 0.0f, 1.0f, -pVolumeB->Far );
    Planes[2].set( 1.0f, 0.0f, -pVolumeB->RightSlope, 0.0f );
    Planes[3].set( -1.0f, 0.0f, pVolumeB->LeftSlope, 0.0f );
    Planes[4].set( 0.0f, 1.0f, -pVolumeB->TopSlope, 0.0f );
    Planes[5].set( 0.0f, -1.0f, pVolumeB->BottomSlope, 0.0f );

    // Load origin and orientation of the frustum.
    float4 Origin = pVolumeB->Origin;
    quaternion FrustumOrientation = pVolumeB->Orientation;

    //XMASSERT( XMQuaternionIsUnit( FrustumOrientation ) );

    // Load the box.
    float3 Center = pVolumeA->Center;
    float3 Extents = pVolumeA->Extents;
    quaternion BoxOrientation = pVolumeA->Orientation;

    //XMASSERT( XMQuaternionIsUnit( BoxOrientation ) );
    
    // Transform the oriented box into the space of the frustum in order to 
    // minimize the number of transforms we have to do.
    Center = XMVector3InverseRotate( Center - Origin, FrustumOrientation );
    BoxOrientation = XMQuaternionMultiply( BoxOrientation, XMQuaternionConjugate( FrustumOrientation ) );

    // Set w of the center to one so we can dot4 with the plane.
    Center = XMVectorInsert( Center, XMVectorSplatOne(), 0, 0, 0, 0, 1);

    // Build the 3x3 rotation matrix that defines the box axes.
    XMMATRIX R = XMMatrixRotationQuaternion( BoxOrientation );

    // Check against each plane of the frustum.
    XMVECTOR Outside = XMVectorFalseInt();
    XMVECTOR InsideAll = XMVectorTrueInt();
    XMVECTOR CenterInsideAll = XMVectorTrueInt();

    for( INT i = 0; i < 6; i++ )
    {
      // Compute the distance to the center of the box.
      XMVECTOR Dist = XMVector4Dot( Center, Planes[i] );

      // Project the axes of the box onto the normal of the plane.  Half the
      // length of the projection (sometime called the "radius") is equal to
      // h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
      // where h(i) are extents of the box, n is the plane normal, and b(i) are the 
      // axes of the box.
      XMVECTOR Radius = XMVector3Dot( Planes[i], R.r[0] );
      Radius = XMVectorSelect( Radius, XMVector3Dot( Planes[i], R.r[1] ), SelectY );
      Radius = XMVectorSelect( Radius, XMVector3Dot( Planes[i], R.r[2] ), SelectZ );
      Radius = XMVector3Dot( Extents, XMVectorAbs( Radius ) );

      // Outside the plane?
      Outside = XMVectorOrInt( Outside, XMVectorGreater( Dist, Radius ) );

      // Fully inside the plane?
      InsideAll = XMVectorAndInt( InsideAll, XMVectorLessOrEqual( Dist, -Radius ) );

      // Check if the center is inside the plane.
      CenterInsideAll = XMVectorAndInt( CenterInsideAll, XMVectorLessOrEqual( Dist, Zero ) );
    }

    // If the box is outside any of the planes it is outside. 
    if ( XMVector4EqualInt( Outside, XMVectorTrueInt() ) )
      return 0;

    // If the box is inside all planes it is fully inside.
    if ( XMVector4EqualInt( InsideAll, XMVectorTrueInt() ) )
      return 2;

    // If the center of the box is inside all planes and the box intersects 
    // one or more planes then it must intersect.
    if ( XMVector4EqualInt( CenterInsideAll, XMVectorTrueInt() ) )
      return 1;

    // Build the corners of the frustum.
    XMVECTOR RightTop = XMVectorSet( pVolumeB->RightSlope, pVolumeB->TopSlope, 1.0f, 0.0f );
    XMVECTOR RightBottom = XMVectorSet( pVolumeB->RightSlope, pVolumeB->BottomSlope, 1.0f, 0.0f );
    XMVECTOR LeftTop = XMVectorSet( pVolumeB->LeftSlope, pVolumeB->TopSlope, 1.0f, 0.0f );
    XMVECTOR LeftBottom = XMVectorSet( pVolumeB->LeftSlope, pVolumeB->BottomSlope, 1.0f, 0.0f );
    XMVECTOR Near = XMVectorReplicatePtr( &pVolumeB->Near );
    XMVECTOR Far = XMVectorReplicatePtr( &pVolumeB->Far );

    XMVECTOR Corners[8];
    Corners[0] = RightTop * Near;
    Corners[1] = RightBottom * Near;
    Corners[2] = LeftTop * Near;
    Corners[3] = LeftBottom * Near;
    Corners[4] = RightTop * Far;
    Corners[5] = RightBottom * Far;
    Corners[6] = LeftTop * Far;
    Corners[7] = LeftBottom * Far;

    // Test against box axes (3)
    {
      // Find the min/max values of the projection of the frustum onto each axis.
      XMVECTOR FrustumMin, FrustumMax;

      FrustumMin = XMVector3Dot( Corners[0], R.r[0] );
      FrustumMin = XMVectorSelect( FrustumMin, XMVector3Dot( Corners[0], R.r[1] ), SelectY );
      FrustumMin = XMVectorSelect( FrustumMin, XMVector3Dot( Corners[0], R.r[2] ), SelectZ );
      FrustumMax = FrustumMin;

      for( INT i = 1; i < 8; i++ )
      {
        XMVECTOR Temp = XMVector3Dot( Corners[i], R.r[0] );
        Temp = XMVectorSelect( Temp, XMVector3Dot( Corners[i], R.r[1] ), SelectY );
        Temp = XMVectorSelect( Temp, XMVector3Dot( Corners[i], R.r[2] ), SelectZ );

        FrustumMin = XMVectorMin( FrustumMin, Temp );
        FrustumMax = XMVectorMax( FrustumMax, Temp );
      }

      // Project the center of the box onto the axes.
      XMVECTOR BoxDist = XMVector3Dot( Center, R.r[0] );
      BoxDist = XMVectorSelect( BoxDist, XMVector3Dot( Center, R.r[1] ), SelectY );
      BoxDist = XMVectorSelect( BoxDist, XMVector3Dot( Center, R.r[2] ), SelectZ );

      // The projection of the box onto the axis is just its Center and Extents.
      // if (min > box_max || max < box_min) reject;
      XMVECTOR Result = XMVectorOrInt( XMVectorGreater( FrustumMin, BoxDist + Extents ),
        XMVectorLess( FrustumMax, BoxDist - Extents ) );

      if( XMVector3AnyTrue( Result ) )
        return 0;
    }

    // Test against edge/edge axes (3*6).
    XMVECTOR FrustumEdgeAxis[6];

    FrustumEdgeAxis[0] = RightTop;
    FrustumEdgeAxis[1] = RightBottom;
    FrustumEdgeAxis[2] = LeftTop;
    FrustumEdgeAxis[3] = LeftBottom;
    FrustumEdgeAxis[4] = RightTop - LeftTop;
    FrustumEdgeAxis[5] = LeftBottom - LeftTop;

    for( INT i = 0; i < 3; i++ )
    {
      for( INT j = 0; j < 6; j++ )
      {
        // Compute the axis we are going to test.
        XMVECTOR Axis = XMVector3Cross( R.r[i], FrustumEdgeAxis[j] );

        // Find the min/max values of the projection of the frustum onto the axis.
        XMVECTOR FrustumMin, FrustumMax;

        FrustumMin = FrustumMax = XMVector3Dot( Axis, Corners[0] );

        for( INT k = 1; k < 8; k++ )
        {
          XMVECTOR Temp = XMVector3Dot( Axis, Corners[k] );
          FrustumMin = XMVectorMin( FrustumMin, Temp );
          FrustumMax = XMVectorMax( FrustumMax, Temp );
        }

        // Project the center of the box onto the axis.
        XMVECTOR Dist = XMVector3Dot( Center, Axis );

        // Project the axes of the box onto the axis to find the "radius" of the box.
        XMVECTOR Radius = XMVector3Dot( Axis, R.r[0] );
        Radius = XMVectorSelect( Radius, XMVector3Dot( Axis, R.r[1] ), SelectY );
        Radius = XMVectorSelect( Radius, XMVector3Dot( Axis, R.r[2] ), SelectZ );
        Radius = XMVector3Dot( Extents, XMVectorAbs( Radius ) );

        // if (center > max + radius || center < min - radius) reject;
        Outside = XMVectorOrInt( Outside, XMVectorGreater( Dist, FrustumMax + Radius ) );
        Outside = XMVectorOrInt( Outside, XMVectorLess( Dist, FrustumMin - Radius ) );
      }
    }

    if ( XMVector4EqualInt( Outside, XMVectorTrueInt() ) )
      return 0;
      
    // If we did not find a separating plane then the box must intersect the frustum.
    return 1;
  }
//*/
  } // namespace geometry
} // namespace clstd
