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
#include "Engine/ScalingAxis.h"
#include "GrapX/gxUtility.h"
#include "Engine/Drag.h"

namespace EditorUtility
{
  static const float fExtentP = 0.5f; // plane
  static const float fExtentE = fExtentP * 0.618f * 0.5f;  // Entirety box

  GVNode* ScalingAxis::Hit(GXCanvas3D* pCanvas, GXLPCPOINT ptHit, NODERAYTRACE* pNRT)
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

  GXBOOL ScalingAxis::HitTest(GXCanvas3D* pCanvas, GXBOOL bHighlight, GXLPCPOINT ptHit)
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
      ASSERT(nrt.nFaceIndex >= 0 && nrt.nFaceIndex < 24);
      if(nrt.nFaceIndex < 8) { m_eHit = HT_PLANE_YZ; }
      else if(nrt.nFaceIndex < 16) { m_eHit = HT_PLANE_ZX; }
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
      case 'E': m_eHit = HT_ENTIRETY; break;
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

  GXBOOL ScalingAxis::InitializeRefPlane(GXGraphics* pGraphics)
  {
    RESET_FLAG(m_dwFlags, GVNF_CONTAINER);
    const float3 vOrigin(0.0f);
    GXVERTEX_P3F_C1D aVertices[24];
    const int NV = 12; // 一个正反平面需要的顶点数, 一个轴平面包含了正反4个矩形(TN * 2)
    const int NP = 6;  // 整个模型索引需要的面数
    static GXWORD aTriangles[NV] = {0,1,2, 0,3,1, 2,1,0, 1,3,0};
    GXWORD aIndices[72];
    static GXDWORD aColors[3] = { 0xffff2020, 0xff20ff20, 0xff2020ff };
    const float fExtent2E = fExtentE * 2.0f;

    for(int i = 0; i < NP; i++) {
      for(int n = 0; n < NV; n++) {
        aIndices[i * NV + n] = aTriangles[n] + 4 * i;
      }
    }

    // vOrigin.z => fExtent2E
    aVertices[ 0].pos.set(vOrigin.x, vOrigin.y, fExtent2E);
    aVertices[ 1].pos.set(vOrigin.x,  fExtentP,  fExtentP);
    aVertices[ 2].pos.set(vOrigin.x, vOrigin.y,  fExtentP);
    aVertices[ 3].pos.set(vOrigin.x,  fExtentP, fExtent2E);

    // vOrigin.y => fExtent2E
    // Z:fExtentP => fExtent2E
    aVertices[ 4].pos.set(vOrigin.x, fExtent2E, vOrigin.z);
    aVertices[ 5].pos.set(vOrigin.x,  fExtentP, fExtent2E);
    aVertices[ 6].pos.set(vOrigin.x, fExtent2E, fExtent2E);
    aVertices[ 7].pos.set(vOrigin.x,  fExtentP, vOrigin.z);

    // vOrigin.x => fExtent2E
    aVertices[ 8].pos.set(fExtent2E, vOrigin.y, vOrigin.z);
    aVertices[ 9].pos.set(fExtentP,  vOrigin.y,  fExtentP);
    aVertices[10].pos.set(fExtentP,  vOrigin.y, vOrigin.z);
    aVertices[11].pos.set(fExtent2E, vOrigin.y,  fExtentP);

    // vOrigin.z => fExtent2E
    // X:fExtentP => fExtent2E
    aVertices[12].pos.set(vOrigin.x, vOrigin.y, fExtent2E);
    aVertices[13].pos.set(fExtent2E, vOrigin.y,  fExtentP);
    aVertices[14].pos.set(fExtent2E, vOrigin.y, fExtent2E);
    aVertices[15].pos.set(vOrigin.x, vOrigin.y,  fExtentP);

    // vOrigin.y => fExtent2E
    aVertices[16].pos.set(vOrigin.x, fExtent2E, vOrigin.z);
    aVertices[17].pos.set(fExtentP,   fExtentP, vOrigin.z);
    aVertices[18].pos.set(vOrigin.x,  fExtentP, vOrigin.z);
    aVertices[19].pos.set(fExtentP,  fExtent2E, vOrigin.z);

    // vOrigin.x => fExtent2E
    // Y:fExtentP => fExtent2E
    aVertices[20].pos.set(fExtent2E, vOrigin.y, vOrigin.z);
    aVertices[21].pos.set(fExtentP,  fExtent2E, vOrigin.z);
    aVertices[22].pos.set(fExtent2E, fExtent2E, vOrigin.z);
    aVertices[23].pos.set(fExtentP,  vOrigin.y, vOrigin.z);

    for(int i = 0; i < 24; i++) {
      aVertices[i].color = aColors[i >> 3];
    }    

    m_eType = GXPT_TRIANGLELIST;
    return GVMesh::IntCreatePrimitive(pGraphics, 24, MOGetSysVertexDecl(GXVD_P3F_C1D), aVertices, 24, aIndices, 72);
  }

  GXHRESULT ScalingAxis::CreateAxis(GXGraphics* pGraphics, ScalingAxis** ppAxis)
  {
    GXHRESULT hval = GX_OK;
    auto* pAxis = new ScalingAxis();
    pAxis->AddRef();

    if(( ! pAxis->InitializeAsAxis(pGraphics, float3::Origin, 1.0f, 1)) ||
      ( ! pAxis->InitializeRefPlane(pGraphics))) {
      hval = GX_FAIL;
    }

    GVGeometry* pBox = NULL; // Entirety scaling
    
    if(GXSUCCEEDED(hval) && GXSUCCEEDED(GVGeometry::CreateBox(pGraphics, fExtentE, fExtentE, 0xFFE0E0E0, &pBox))) {
      pBox->SetName("Entirety");
      pBox->SetParent(pAxis);
    }
    else {
      hval = GX_FAIL;
    }

    if(GXSUCCEEDED(hval)) {
      pAxis->CombineFlags(GVNF_NORAYTRACE);
      *ppAxis = pAxis;
    }
    else {
      pAxis->Release();
      pAxis = NULL;
    }
    return hval;
  }


#define CMP_DIR(_AXIS1, _AXIS2)   fabs(float3::dot(RayLocal.vDirection, _AXIS1)) > fabs(float3::dot(RayLocal.vDirection, _AXIS2))
#define SEL_PLANE(_AXIS1, _AXIS2) if(CMP_DIR(_AXIS1, _AXIS2)) { DragAxis.m_plane.set(_AXIS1, 0); } else { DragAxis.m_plane.set(_AXIS2, 0); }

  GXBOOL ScalingAxis::Track( GXCanvas3D* pCanvas, GXLPCPOINT ptHit )
  {
    if(HitTest(pCanvas, TRUE, ptHit))
    {
      m_vTrackedScaling = m_pBind->GetTransform().scaling / GetTransform().scaling;
      m_bTracking = TRUE;

      class CDragAxis : public CDrag
      {
        friend class ScalingAxis;
      private:
        GXCanvas3D* m_pCanvas;
        ScalingAxis* m_pAxis;
        float3 m_vLimit;

      public:
        float3 m_vHitStart;
        Plane m_plane;
        float4x4 m_matInvAbs;
        float4x4 m_matAbs;
      public:
        CDragAxis(GXCanvas3D* pCanvas, ScalingAxis* pAxis, GXLPCPOINT point) 
          : m_pCanvas(pCanvas)
          , m_pAxis(pAxis)
          , m_vHitStart(0.0f)
          , m_vLimit(1.0f)
        {
        }

        GXBOOL OnDrag(GXLPCPOINT ptAbsoluteDelta, GXLPCPOINT ptOrigin)
        {
          GXPOINT pt = {ptOrigin->x + ptAbsoluteDelta->x, ptOrigin->y + ptAbsoluteDelta->y};
          float3 vScaling;
          Ray ray;

          m_pCanvas->RayFromScreen(&pt, &ray);
          NormalizedRay RayLocal(ray.vOrigin * m_matInvAbs, ray.vDirection.MulAsMatrix3x3(m_matInvAbs));

          float3 v2;
          if(clstd::RayIntersectPlane2S(RayLocal, m_plane, &v2))
          {
            const float3& S = m_pAxis->m_pBind->GetTransform().scaling;

            for(int i = 0; i < 3; i++) {
              if(m_vHitStart.m[i] != 0.0f && m_vLimit.m[i] != 0.0f) {
                vScaling.m[i] = v2.m[i] / m_vHitStart.m[i];
              }
              else {
                vScaling.m[i] = S.m[i];
              }
            }
            
            m_pAxis->InvokeScaling(vScaling);
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

      case HT_ENTIRETY:
        {
          float CX = fabs(float3::dot(RayLocal.vDirection, float3::AxisX));
          float CY = fabs(float3::dot(RayLocal.vDirection, float3::AxisY));
          float CZ = fabs(float3::dot(RayLocal.vDirection, float3::AxisZ));

          if(CX > CY && CX > CZ) {  DragAxis.m_plane.set(float3::AxisX, 0); }
          else if(CY > CZ) {        DragAxis.m_plane.set(float3::AxisY, 0); }
          else {                    DragAxis.m_plane.set(float3::AxisZ, 0); }
          DragAxis.m_vLimit.set(1, 1, 1);
        }
        break;
      }

      if( ! clstd::RayIntersectPlane2S(RayLocal, DragAxis.m_plane, &v1)) {
        m_bTracking = FALSE;
        return FALSE;
      }

      v1 = (v1 / m_pBind->GetTransform().scaling) * DragAxis.m_vLimit;
      DragAxis.m_matAbs = m_Transformation.GlobalMatrix;

      DragAxis.m_vHitStart = v1;
      DragAxis.Track(ptHit, ptHit);
      m_bTracking = FALSE;
      return TRUE;
    }

    m_bTracking = FALSE;
    return FALSE;
  }

  GVNode* ScalingAxis::BindNode( GVNode* pNode )
  {
    GVNode* pPrevNode = m_pBind;
    InlSetNewObjectT(m_pBind, pNode);
    if(m_pBind) {
      AlignBinder();
    }
    return pPrevNode;
  }

  void ScalingAxis::AlignBinder()
  {
    float3 vPos = m_pBind->GetPosition(GVNode::S_ABSOLUTE);
    GVNode* pParent = m_pBind->GetParent();
    const clstd::TRANSFORM& T = m_pBind->GetTransform();

    // 如果存在父对象，则使用自身的全局变换解算出全局旋转
    // 如果不存在父对象，它的全局变换就是自身的局部变换
    if(pParent) {
      float3 s;
      quaternion r;
      m_pBind->GetTransform().GlobalMatrix.Decompose(&s, &r);
      SetTransform(m_bTracking ? &(T.scaling / m_vTrackedScaling) : NULL, &r, &vPos);
    }
    else {
      SetTransform(m_bTracking ? &(T.scaling / m_vTrackedScaling) : NULL, &T.rotation, &vPos);
    }
  }

  ScalingAxis::~ScalingAxis()
  {
    SAFE_RELEASE(m_pBind);
  }

  GXBOOL ScalingAxis::Update( const GVSCENEUPDATE& sContext )
  {
    if( ! IsVisible() || m_bTracking) {
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

  void ScalingAxis::SetHighlight( GXBOOL bHighlight )
  {
    ASSERT(m_pHighlight);
    GXColor32 dwColor = 0xffffff00;
    if(m_pHighlight == this) {
      GXVERTEX_P3F_C1D* pVertices = (GXVERTEX_P3F_C1D*)m_pPrimitive->GetVerticesBuffer();
      switch(m_eHighlight) {
      case HT_PLANE_XY:
        pVertices += 16;
        dwColor = 0xff2020ff;
        break;
      case HT_PLANE_YZ:
        dwColor = 0xffff2020;
        break;
      case HT_PLANE_ZX:
        pVertices += 8;
        dwColor = 0xff20ff20;
        break;
      }

      if(bHighlight) {
        dwColor = 0xffffff00;
      }

      for(int i = 0; i < 8; i++)
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
        GXCHAR c = m_pHighlight->GetName()[0];
        if(c == 'E') {
          dwColor = 0xffE0E0E0;
        }
        else {
          dwColor = GetAxisColor(c);
        }
      }
      PrimitiveUtility::SetUnifiedDiffuse(rd.pPrimitive, dwColor, FALSE);
    }
  }

  GXVOID ScalingAxis::SetFunction( TransformAxisFunction* pFunction )
  {
    m_pCallback = pFunction;
  }

  void ScalingAxis::InvokeScaling( float3& vScaling )
  {
    if(m_eHit == HT_ENTIRETY) {
      vScaling.x = vScaling.y = vScaling.z = (vScaling.x + vScaling.y + vScaling.z) * 0.333f;
    }

    if(m_pBind) {
      if(m_pCallback) {
        m_pCallback->Scale(m_pBind, vScaling);
      }
      else {
        m_pBind->SetScaling(vScaling);
      }
      AlignBinder();
    }
  }

} // namespace EditorUtility