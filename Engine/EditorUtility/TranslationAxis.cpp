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
#include "Engine/TranslationAxis.h"
#include "GrapX/gxUtility.h"
#include "Engine/Drag.h"

namespace EditorUtility
{
  GXColor32 TransformAxis::GetAxisColor(char cAxis)
  {
    static GXColor32 cr[] = {0xffff0000, 0xff00ff00, 0xff0000ff};
    ASSERT(cAxis == 'X' || cAxis == 'Y' || cAxis == 'Z');
    return cr[cAxis - 'X'];
  }

  GVNode* TranslationAxis::Hit(GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT)
  {
    Ray ray;
    pNRT->eType = NRTT_NONE;
    pCanvas->RayFromScreen(ptHit, &ray);
    if(RayTrace(ray, pNRT) && pNRT->eType == NRTT_MESHFACE) {
      return this;
    }    

    GVNode* pNode = GetFirstChild();
    while(pNode)
    {
      if(pNode->RayTrace(ray, pNRT)) {
        break;
      }
      pNode = pNode->GetNext();
    }

    if(pNRT->eType == NRTT_NONE) {
      return NULL;
    }
    return pNode;
  }

  GXBOOL TranslationAxis::HitTest(GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit)
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
    else if(pNode == this) {
      ASSERT(nrt.nFaceIndex >= 0 && nrt.nFaceIndex < 12);
      if(nrt.nFaceIndex < 4) { m_eHit = HT_PLANE_YZ; }
      else if(nrt.nFaceIndex < 8) { m_eHit = HT_PLANE_ZX; }
      else { m_eHit = HT_PLANE_XY; }
      result = TRUE;
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

    if(bHighlight)
    {
      if(m_pHighlight) {
        SetHighlight(FALSE);
      }

      m_eHighlight = m_eHit;
      m_pHighlight = static_cast<GVGeometry*>(pNode);

      if(m_pHighlight) {
        SetHighlight(TRUE);
      }
    }

    return result;
  }

  GXBOOL TranslationAxis::InitializeRefPlane(GXGraphics* pGraphics)
  {
    RESET_FLAG(m_dwFlags, GVNF_CONTAINER);
    const float3 vOrigin(0.0f);
    const float fExtent = 0.3f;
    GXVERTEX_P3F_C1D aVertices[12];
    static GXWORD aIndices[36] = {
      0,1,2, 0,3,1, 2,1,0, 1,3,0, 
      4,5,6, 4,7,5, 6,5,4, 5,7,4, 
      8,9,10, 8,11,9, 10,9,8, 9,11,8,};
    static GXDWORD aColors[3] = {
      0xffff2020, 0xff20ff20, 0xff2020ff
    };

    aVertices[ 0].pos = vOrigin;
    aVertices[ 1].pos.set(vOrigin.x,   fExtent,   fExtent);
    aVertices[ 2].pos.set(vOrigin.x, vOrigin.y,   fExtent);
    aVertices[ 3].pos.set(vOrigin.x,   fExtent, vOrigin.z);

    aVertices[ 4].pos = vOrigin;
    aVertices[ 5].pos.set(fExtent,   vOrigin.y,   fExtent);
    aVertices[ 6].pos.set(fExtent,   vOrigin.y, vOrigin.z);
    aVertices[ 7].pos.set(vOrigin.x, vOrigin.y,   fExtent);

    aVertices[ 8].pos = vOrigin;
    aVertices[ 9].pos.set(fExtent,     fExtent, vOrigin.z);
    aVertices[10].pos.set(vOrigin.x,   fExtent, vOrigin.z);
    aVertices[11].pos.set(fExtent,   vOrigin.y, vOrigin.z);

    for(int i = 0; i < 12; i++) {
      aVertices[i].color = aColors[i >> 2];
    }    

    m_eType = GXPT_TRIANGLELIST;
    return GVMesh::IntCreatePrimitive(pGraphics, 12, MOGetSysVertexDecl(GXVD_P3F_C1D), aVertices, 12, aIndices, 36);
  }

  GXHRESULT TranslationAxis::CreateAxis(GXGraphics* pGraphics, TranslationAxis** ppAxis)
  {
    GXHRESULT hval = GX_OK;
    auto* pAxis = new TranslationAxis();
    pAxis->AddRef();

    if(( ! pAxis->InitializeAsAxis(pGraphics, float3::Origin, 1.0f, 1)) ||
       ( ! pAxis->InitializeRefPlane(pGraphics)))
    {
      pAxis->Release();
      pAxis = NULL;
      hval = GX_FAIL;
    }
    pAxis->CombineFlags(GVNF_NORAYTRACE);
    *ppAxis = pAxis;
    return hval;
  }


#define CMP_DIR(_AXIS1, _AXIS2)   fabs(float3::dot(RayLocal.vDirection, _AXIS1)) > fabs(float3::dot(RayLocal.vDirection, _AXIS2))
#define SEL_PLANE(_AXIS1, _AXIS2) if(CMP_DIR(_AXIS1, _AXIS2)) { DragAxis.m_plane.set(_AXIS1, 0); } else { DragAxis.m_plane.set(_AXIS2, 0); }

  GXBOOL TranslationAxis::Track( GXCanvas3D* pCanvas, GXLPCPOINT ptHit )
  {
    if(HitTest(pCanvas, TRUE, ptHit))
    {
      m_bTracking = TRUE;
      class CDragAxis : public CDrag
      {
        friend class TranslationAxis;
      private:
        GXCanvas3D* m_pCanvas;
        TranslationAxis* m_pAxis;
        float3 m_vLimit;
        float3 m_vStartPos;
        float4x4 m_matInvParent;

      public:
        float3 m_vHitStart;
        Plane m_plane;
        float4x4 m_matInvAbs;
        float4x4 m_matAbs;
      public:
        CDragAxis(GXCanvas3D* pCanvas, TranslationAxis* pAxis, GXLPCPOINT point) 
          : m_pCanvas(pCanvas)
          , m_pAxis(pAxis)
          , m_vHitStart(0.0f)
          , m_vLimit(1.0f)
        {
          GVNode* pParent = m_pAxis->m_pBind->GetParent();
          m_matInvParent = pParent ? float4x4::inverse(pParent->GetTransform().GlobalMatrix) : float4x4::Identity;
          m_vStartPos = m_pAxis->m_pBind->GetPosition(GVNode::S_ABSOLUTE);
        }

        GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin)
        {
          GXPOINT pt = {ptOrigin->x + ptAbsoluteDelta->x, ptOrigin->y + ptAbsoluteDelta->y};
          float3 vDist;
          Ray ray;

          m_pCanvas->RayFromScreen(&pt, &ray);
          NormalizedRay RayLocal(ray.vOrigin * m_matInvAbs, ray.vDirection.MulAsMatrix3x3(m_matInvAbs));

          float3 v2;
          if(clstd::RayIntersectPlane2S(RayLocal, m_plane, &v2))
          {
            vDist = v2 - m_vHitStart;
            vDist *= m_vLimit;
            vDist = vDist.MulAsMatrix3x3(m_matAbs);

            m_pAxis->InvokeTranslation((vDist + m_vStartPos) * m_matInvParent);
          }
          return TRUE;
        }
      } DragAxis(pCanvas, this, ptHit);

      Ray ray;
      float3 v1;
      pCanvas->RayFromScreen(ptHit, &ray);

      float4x4 matInvAbs = float4x4::inverse(m_Transformation.GlobalMatrix);
      NormalizedRay RayLocal(ray.vOrigin * matInvAbs, ray.vDirection.MulAsMatrix3x3(matInvAbs));
      DragAxis.m_matInvAbs = matInvAbs;

      switch(m_eHit)
      {
      case HT_AXIS_X:
        // SEL_PLANE 用来选择一个ViewDir和Plane比较垂直的平面，这样求出的交点比较精确
        SEL_PLANE(float3::AxisY, float3::AxisZ);
        DragAxis.m_vLimit.set(1, 0, 0);
        break;

      case HT_AXIS_Y:
        SEL_PLANE(float3::AxisZ, float3::AxisX);
        DragAxis.m_vLimit.set(0, 1, 0);
        break;

      case HT_AXIS_Z:
        SEL_PLANE(float3::AxisX, float3::AxisY);
        DragAxis.m_vLimit.set(0, 0, 1);
        break;

      case HT_PLANE_XY:
        DragAxis.m_plane.set(float3::AxisZ, 0);
        DragAxis.m_vLimit.set(1, 1, 0);
        break;

      case HT_PLANE_YZ:
        DragAxis.m_plane.set(float3::AxisX, 0);
        DragAxis.m_vLimit.set(0, 1, 1);
        break;

      case HT_PLANE_ZX:
        DragAxis.m_plane.set(float3::AxisY, 0);
        DragAxis.m_vLimit.set(1, 0, 1);
        break;
      }

      if( ! clstd::RayIntersectPlane2S(RayLocal, DragAxis.m_plane, &v1)) {
        m_bTracking = FALSE;
        return FALSE;
      }

      v1 *= DragAxis.m_vLimit;
      DragAxis.m_matAbs = m_Transformation.GlobalMatrix;

      DragAxis.m_vHitStart = v1;
      DragAxis.Track(ptHit, ptHit);
      m_bTracking = FALSE;
      return TRUE;
    }

    m_bTracking = FALSE;
    return FALSE;
  }

  GVNode* TranslationAxis::BindNode( GVNode* pNode )
  {
    GVNode* pPrevNode = m_pBind;
    InlSetNewObjectT(m_pBind, pNode);
    if(m_pBind) {
      AlignBinder();
    }
    return pPrevNode;
  }

  void TranslationAxis::AlignBinder()
  {
    float3 vPos = m_pBind->GetPosition(GVNode::S_ABSOLUTE);
    if(m_eSpace == GVNode::S_RELATIVE)
    {
      GVNode* pParent = m_pBind->GetParent();
      const clstd::TRANSFORM& T = m_pBind->GetTransform();

      // 如果存在父对象，则使用自身的全局变换解算出全局旋转
      // 如果不存在父对象，它的全局变换就是自身的局部变换
      if(pParent) {
        float3 s;
        quaternion r;
        m_pBind->GetTransform().GlobalMatrix.Decompose(&s, &r);
        SetTransform(NULL, &r, &vPos);
      }
      else {
        SetTransform(NULL, &T.rotation, &vPos);
      }
    }
    else {
      SetTransform(NULL, NULL, &vPos);
    }
  }


  TranslationAxis::~TranslationAxis()
  {
    SAFE_RELEASE(m_pBind);
  }

  GXBOOL TranslationAxis::Update( const GVSCENEUPDATE& sContext )
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

  void TranslationAxis::SetSpace( ESpace eSpace )
  {
    if(m_eSpace == eSpace) {
      return;
    }
    m_eSpace = eSpace;
    if(m_pBind) {
      const clstd::TRANSFORM& T = m_pBind->GetTransform();
      SetTransform(NULL, m_eSpace == GVNode::S_RELATIVE ? &T.rotation : &quaternion(0.0f), &T.translation);
    }
  }

  void TranslationAxis::SetHighlight( GXBOOL bHighlight )
  {
    ASSERT(m_pHighlight);
    GXColor32 dwColor = 0xffffff00;
    if(m_pHighlight == this) {
      GXVERTEX_P3F_C1D* pVertices = (GXVERTEX_P3F_C1D*)m_pPrimitive->GetVerticesBuffer();
      switch(m_eHighlight) {
      case HT_PLANE_XY:
        pVertices += 8;
        dwColor = 0xff2020ff;
        break;
      case HT_PLANE_YZ:
        dwColor = 0xffff2020;
        break;
      case HT_PLANE_ZX:
        pVertices += 4;
        dwColor = 0xff20ff20;
        break;
      }

      if(bHighlight) {
        dwColor = 0xffffff00;
      }

      for(int i = 0; i < 4; i++)
      {
        pVertices->color = dwColor;
        pVertices++;
      }
      m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);
    }
    else {
      GVRENDERDESC rd;
      m_pHighlight->GetRenderDesc(GVRT_Normal, &rd);
      if( ! bHighlight) {
        dwColor = GetAxisColor(m_pHighlight->GetName()[0]);
      }
      PrimitiveUtility::SetUnifiedDiffuse(rd.pPrimitive, dwColor, FALSE);
    }
  }

  GXVOID TranslationAxis::SetFunction( TransformAxisFunction* pFunction )
  {
    m_pCallback = pFunction;
  }

  void TranslationAxis::InvokeTranslation( const float3& vPos )
  {
    if(m_pBind)
    {
      if(m_pCallback) {
        m_pCallback->Translate(m_pBind, m_eSpace, vPos);
      }
      else {
        m_pBind->SetPosition(vPos);
        m_pBind->CombineFlags(GVNF_UPDATEWORLDMAT);
      }
      AlignBinder();
    }
  }

} // namespace EditorUtility