#include "GrapX.H"

#include <GrapX/GXUser.H>

//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GShader.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXCanvas3D.h"
#include "GrapX/GCamera.H"

#include "GrapX/GrapVR.H"
#include "Engine.h"
#include "Engine/TransformAxis.h"
#include "Engine/RotationAxis.h"
#include "GrapX/gxUtility.h"
#include "Engine/Drag.h"

namespace EditorUtility
{
  GXHRESULT RotationAxis::CreateAxis(GXGraphics* pGraphics, RotationAxis** ppAxis)
  {
    GXHRESULT hval = GX_OK;
    auto* pAxis = new RotationAxis();
    if( ! InlCheckNewAndIncReference(pAxis)) {
      return GX_FAIL;
    }

    const float fRadius1 = 1.0f;
    const float fRadius2 = 0.05f;
    const int nSegments = 24;
    const int nSides = 6;

    if( ! pAxis->InitializeRefPlane(pGraphics, nSegments)) {
      pAxis->Release();
      return GX_FAIL;
    }

    GVGeometry* pTorus = NULL;


    GVGeometry::CreateTorus(pGraphics, fRadius1, fRadius2, nSegments, nSides, 0xff00ff00, &pTorus, NULL, GXVF_NORMAL|GXVF_COLOR);
    pTorus->SetName("Y");
    pTorus->SetParent(pAxis);

    float4x4 matTransform;
    matTransform.identity();
    matTransform._11 = 0.0f;
    matTransform._22 = 0.0f;
    matTransform._12 = -1.0f;
    matTransform._21 = 1.0f;

    GVGeometry::CreateTorus(pGraphics, fRadius1, fRadius2, nSegments, nSides, 0xffff0000, &pTorus, &matTransform, GXVF_NORMAL|GXVF_COLOR);
    pTorus->SetName("X");
    pTorus->SetParent(pAxis);

    matTransform.identity();
    matTransform._22 = 0.0f;
    matTransform._33 = 0.0f;
    matTransform._23 = 1.0f;
    matTransform._32 = -1.0f;

    GVGeometry::CreateTorus(pGraphics, fRadius1, fRadius2, nSegments, nSides, 0xff0000ff, &pTorus, &matTransform, GXVF_NORMAL|GXVF_COLOR);
    pTorus->SetName("Z");
    pTorus->SetParent(pAxis);

    pAxis->CombineFlags(GVNF_NORAYTRACE);
    *ppAxis = pAxis;
    return hval;
  }

  GVNode* RotationAxis::Hit(GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT)
  {
    Ray ray;
    pNRT->eType = NRTT_NONE;
    pCanvas->RayFromScreen(ptHit, &ray);
    if(RayTrace(ray, pNRT) && pNRT->eType == NRTT_MESHFACE) {
      return this;
    }    

    GVNode* pNode = GetFirstChild();
    GVNode* pBestNode = NULL;
    float fDist = FLT_MAX;
    while(pNode)
    {
      // FIXME: 应该选择最近的
      if(pNode->RayTrace(ray, pNRT) && pNRT->eType == NRTT_MESHFACE && fDist > pNRT->fSquareDist) {
        pBestNode = pNode;
        fDist = pNRT->fSquareDist;
      }
      pNode = pNode->GetNext();
    }

    if( ! pBestNode) {
      return NULL;
    }
    return pBestNode;
  }

  GXBOOL RotationAxis::HitTest(GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit)
  {
    if(m_bTracking) {
      return FALSE;
    }

    NODERAYTRACE nrt;
    GXBOOL result = FALSE;
    GVNode* pNode = Hit(pCanvas, ptHit, &nrt);
    if(pNode == NULL) {
      m_eHit = HT_NONE;
    }
    else
    {
      GXLPCSTR szName = pNode->GetName();
      switch(szName[0])
      {
      case 'X': m_eHit = HT_AXIS_X; break;
      case 'Y': m_eHit = HT_AXIS_Y; break;
      case 'Z': m_eHit = HT_AXIS_Z; break;
      }
      result = TRUE;
    }

    if(bHighlight) {
      if(m_pHighlight) {
        SetHighlight(FALSE);
      }
      m_pHighlight = static_cast<GVGeometry*>(pNode);
      if(m_pHighlight) {
        SetHighlight(TRUE);
      }
    }

    return result;
  }

  float RotationAxis::ProjectLength(GXCanvas3D* pCanvas, const float3& vPos, const float3& vDir, GXLPCPOINT ptBegin, GXLPCPOINT ptEnd)
  {
    // 给定一个空间中的参考点和参考方向,根据屏幕上两点的移动计算出空间中在这个方向上的投影距离(移动距离)
    ASSERT(fabs(1.0f - vDir.lengthsquare()) <= 1.0f - 1e-9f);

    GXPOINT ptRefPos;
    GXPOINT ptUnitEnd;
    float3 vUnitPos = vPos + vDir;  // 空间中单位向量的终点
    pCanvas->PositionToScreen(&vUnitPos, &ptUnitEnd);
    pCanvas->PositionToScreen(&vPos, &ptRefPos);

    float2 vScreen((float)(ptEnd->x - ptBegin->x), (float)(ptEnd->y - ptBegin->y));
    float2 vWorldRef((float)(ptUnitEnd.x - ptRefPos.x), (float)(ptUnitEnd.y - ptRefPos.y));

    const float fDist = float2::dot(vScreen, float2::normalize(vWorldRef)) / vWorldRef.length();
    return fDist;
  }
  
  GXBOOL RotationAxis::InitializeRefPlane(GXGraphics* pGraphics, int nSegments)
  {
    RESET_FLAG(m_dwFlags, GVNF_CONTAINER);
    const float3 vOrigin(0.0f);
    const float fExtent = 0.3f;
    GXVERTEX_P3F_C1D* aVertices = new GXVERTEX_P3F_C1D[nSegments + 2];
    GXWORD* aIndices = new GXWORD[nSegments + 2];

    m_nSegments = nSegments;
    aIndices[0] = 0;
    aVertices[0].pos.set(0, 0, 0);
    aVertices[0].color = 0xffe0e000;
    
    for(int i = 1; i < (nSegments + 2); i++)
    {
      aIndices[i] = i;
      aVertices[i].pos.set(0, 0, 0);
      aVertices[i].color = 0xffe0e000;
    }

    m_eType = GXPT_TRIANGLEFAN;
    GXBOOL result = GVMesh::IntCreatePrimitive(pGraphics, nSegments, MOGetSysVertexDecl(GXVD_P3F_C1D), aVertices, nSegments + 2, aIndices, nSegments + 2);
    delete aIndices;
    delete aVertices;
    return result;
  }

  GXBOOL RotationAxis::Track( GXCanvas3D* pCanvas, GXLPCPOINT ptHit )
  {
    //NODERAYTRACE nrt;
    if(HitTest(pCanvas, TRUE, ptHit))
    {
      m_bTracking = TRUE;
      class CDragAxis : public CDrag
      {
        friend class RotationAxis;
      private:
        GXCanvas3D* m_pCanvas;
        RotationAxis* m_pAxis;
        GXPOINT m_point;
        //float3 m_vLimit;

      public:
        float m_fStart;
        float m_fPrevArc;
        Plane m_plane;
        float4x4 m_matInvAbs;
      public:
        CDragAxis(GXCanvas3D* pCanvas, RotationAxis* pAxis, GXLPCPOINT point) 
          : m_pCanvas(pCanvas)
          , m_pAxis(pAxis)
          , m_point(*point)
          , m_fStart(0)
          //, m_vLimit(1.0f)
          , m_fPrevArc(0) {}

        GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin)
        {
          GXPOINT pt = {ptOrigin->x + ptAbsoluteDelta->x, ptOrigin->y + ptAbsoluteDelta->y};
          float3 vRotation;
          Ray ray;

          m_pCanvas->RayFromScreen(&pt, &ray);
          NormalizedRay RayLocal(ray.vOrigin * m_matInvAbs, ray.vDirection.MulAsMatrix3x3(m_matInvAbs));

          //NormalizedRay RayLocal = ray;

          float3 v2;
          if(clstd::RayIntersectPlane2S(RayLocal, m_plane, &v2))
          {
            //v2 -= m_pAxis->GetPosition();
            float arc = 0;

            AxisHitTest& eHit = m_pAxis->m_eHit;

            if(eHit == HT_AXIS_X) {
              v2.x = 0;
              v2.normalize();
              arc = atan2(v2.z, v2.y);
              vRotation.set(arc - m_fPrevArc, 0, 0);
            }
            else if(eHit == HT_AXIS_Y) {
              v2.y = 0;
              v2.normalize();
              arc = atan2(v2.x, v2.z);
              vRotation.set(0, arc - m_fPrevArc, 0);
              m_pAxis->SetArcY(m_fStart, arc);
            }
            else if(eHit == HT_AXIS_Z) {
              v2.z = 0;
              v2.normalize();
              arc = atan2(v2.y, v2.x);
              vRotation.set(0, 0, arc - m_fPrevArc);
            }
            
            m_pAxis->InvokeRotation(vRotation);
            m_fPrevArc = arc;
          }

          m_point = pt;
          return TRUE;
        }
      } DragAxis(pCanvas, this, ptHit);

      Ray ray;
      float3 v1;// = nrt.vLocalHit - GetPosition();
      pCanvas->RayFromScreen(ptHit, &ray);
      float4x4 matInvAbs = float4x4::inverse(m_Transformation.GlobalMatrix);
      NormalizedRay RayLocal(ray.vOrigin * matInvAbs, ray.vDirection.MulAsMatrix3x3(matInvAbs));
      //NormalizedRay RayLocal = ray;
      DragAxis.m_matInvAbs = matInvAbs;

      if(m_eHit == HT_AXIS_X) {
        DragAxis.m_plane.set(float3::AxisX, 0);
        if( ! clstd::RayIntersectPlane2S(RayLocal, DragAxis.m_plane, &v1)) {
          m_bTracking = FALSE;
          return FALSE;
        }
        v1.x = 0;
        v1.normalize();
        DragAxis.m_fStart = atan2(v1.z, v1.y);
      }
      else if(m_eHit == HT_AXIS_Y) {
        DragAxis.m_plane.set(float3::AxisY, 0);
        if( ! clstd::RayIntersectPlane2S(RayLocal, DragAxis.m_plane, &v1)) {
          m_bTracking = FALSE;
          return FALSE;
        }
        v1.y = 0;
        v1.normalize();
        DragAxis.m_fStart = atan2(v1.x, v1.z);
      }
      else if(m_eHit == HT_AXIS_Z) {
        DragAxis.m_plane.set(float3::AxisZ, 0);
        if( ! clstd::RayIntersectPlane2S(RayLocal, DragAxis.m_plane, &v1)) {
          m_bTracking = FALSE;
          return FALSE;
        }
        v1.z = 0;
        v1.normalize();
        DragAxis.m_fStart = atan2(v1.y, v1.x);
      }

      DragAxis.m_fPrevArc = DragAxis.m_fStart;
      DragAxis.Track(ptHit, ptHit);
      m_bTracking = FALSE;
      return TRUE;
    }
    return FALSE;
  }

  GVNode* RotationAxis::BindNode( GVNode* pNode )
  {
    GVNode* pPrevNode = m_pBind;
    InlSetNewObjectT(m_pBind, pNode);
    if(m_pBind) {
      AlignBinder();
    }
    return pPrevNode;
  }

  void RotationAxis::AlignBinder()
  {
    const clstd::TRANSFORM& T = m_pBind->GetTransform();
    GVNode* pParent = m_pBind->GetParent();
    float3 vPos = m_pBind->GetPosition(GVNode::S_ABSOLUTE);
    if(pParent)
    {
      float3 s;
      quaternion r;
      m_pBind->GetTransform().GlobalMatrix.Decompose(&s, &r);
      SetTransform(NULL, &r, &vPos);
    }
    else {
      SetTransform(NULL, &T.rotation, &vPos);
    }
  }

  RotationAxis::~RotationAxis()
  {
    SAFE_RELEASE(m_pBind);
  }

  GXBOOL RotationAxis::Update( const GVSCENEUPDATE& sContext )
  {
    if( ! IsVisible()) {
      return TRUE;
    }

    float3 vLen = sContext.pCamera->GetPos() - GetPosition();
    const float3& vScaling = GetTransform().scaling;
    float fLength = vLen.length() * 0.15f;
    if(vScaling.x != fLength || vScaling.y != fLength || vScaling.z != fLength)
    {
      SetScaling(fLength);
    }
    return TRUE;
  }

  void RotationAxis::SetArcY( float fStart, float fStop )
  {
    float fDelta = (fStop - fStart);
    fStop = fStart - fDelta;

    GXVERTEX_P3F_C1D* pVertices = (GXVERTEX_P3F_C1D*)m_pPrimitive->GetVerticesBuffer();
    const int nLoop = m_nSegments + 2;
    for(int i = 1; i < nLoop; i++)
    {
      float a = ((float)(i - 1) / (float)m_nSegments) * (fStop - fStart) + fStart;
      pVertices[i].pos.set(sin(a), 0, cos(a));
      pVertices[i].color = 0xffe0e000;
    }
    m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
  }

  void RotationAxis::SetHighlight( GXBOOL bHighlight )
  {
    ASSERT(m_pHighlight);
    GVRENDERDESC rd;
    m_pHighlight->GetRenderDesc(GVRT_Normal, &rd);

    //GXLPBYTE lpColors = (GXLPBYTE)rd.pPrimitive->GetVerticesBuffer() + rd.pPrimitive->GetElementOffset(GXDECLUSAGE_COLOR, 0);
    //int nNumVertices = rd.pPrimitive->GetVerticesCount();
    //int nStride = rd.pPrimitive->GetVertexStride();
    GXColor32 dwColor = 0xffffff00;

    if( ! bHighlight) {
      dwColor = GetAxisColor(m_pHighlight->GetName()[0]);
    }

    //for(int i = 0; i < nNumVertices; i++)
    //{
    //  *(GXDWORD*)lpColors = dwColor;
    //  lpColors += nStride;
    //}

    PrimitiveUtility::SetUnifiedDiffuse(rd.pPrimitive, dwColor, FALSE);

    rd.pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
  }

  GXVOID RotationAxis::SetFunction( TransformAxisFunction* pFunction )
  {
    m_pCallback = pFunction;
  }

  void RotationAxis::InvokeRotation( const float3& vRotation )
  {
    if(m_pBind)
    {
      if(m_pCallback) {
        m_pCallback->RotateR(m_pBind, vRotation);
      }
      else {
        m_pBind->RotateR(vRotation);
        m_pBind->CombineFlags(GVNF_UPDATEWORLDMAT);
      }
      AlignBinder();
    }
  }

} // namespace EditorUtility