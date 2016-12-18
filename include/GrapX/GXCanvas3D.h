// GrapX 3D 绘图对象
// 与GXCanvas不同的是, GXCanvas在绘图时锁定, 用完释放. GXCanvas3D 需要长期持有, 完全不用时才能释放
// GetXXX 返回指针为 Canvas3D 内部数据
#ifndef _GRAP_X_CANVAS_3D_INTERFACE_H_
#define _GRAP_X_CANVAS_3D_INTERFACE_H_

class GVSequence;
struct STANDARDMTLUNIFORMTABLE;

class GXCanvas3D : public GXCanvasCore
{
public:
  GXCanvas3D(GXUINT nPrioruty, GXDWORD dwType) : GXCanvasCore(nPrioruty, dwType) {}

  GXSTDINTERFACE(GXHRESULT      Invoke                (GRESCRIPTDESC* pDesc));
  GXSTDINTERFACE(GXHRESULT      Activate              ());
  GXSTDINTERFACE(GXHRESULT      Clear                 (GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil));
  GXSTDINTERFACE(GXHRESULT      Draw                  (GVSequence* pSequence));

  GXSTDINTERFACE(GXHRESULT      SetMaterialInst       (GXMaterialInst* pMtlInst));
  GXSTDINTERFACE(GXHRESULT      SetPrimitive          (GPrimitive* pPrimitive));
  GXSTDINTERFACE(GXHRESULT      SetCamera             (GCamera* pCamera));
  GXSTDINTERFACE(GCamera*       GetCameraUnsafe       ());
  GXSTDINTERFACE(GXHRESULT      GetDepthStencil       (GTexture** ppDepthStencil) const);
  GXSTDINTERFACE(void           SetWorldMatrix        (const float4x4& matWorld));

#ifdef REFACTOR_SHADER
  GXSTDINTERFACE(GXDWORD        GetGlobalHandle       (GXLPCSTR szName));

  GXSTDINTERFACE(GXHRESULT      SetCanvasFloat        (GXDWORD dwGlobalHandle, float fValue));
  GXSTDINTERFACE(GXHRESULT      SetCanvasVector       (GXDWORD dwGlobalHandle, const float4& rVector));
  GXSTDINTERFACE(GXHRESULT      SetCanvasMatrix       (GXDWORD dwGlobalHandle, const float4x4& rMatrix));

  GXSTDINTERFACE(GXHRESULT      SetCanvasFloat        (GXLPCSTR szName, float fValue));
  GXSTDINTERFACE(GXHRESULT      SetCanvasVector       (GXLPCSTR szName, const float4& rVector));
  GXSTDINTERFACE(GXHRESULT      SetCanvasMatrix       (GXLPCSTR szName, const float4x4& rMatrix));
#endif // #ifdef REFACTOR_SHADER

  GXSTDINTERFACE(GXHRESULT      TransformPosition     (const float3* pPos, GXOUT float4* pView)); // Transform world position to screen
  GXSTDINTERFACE(GXHRESULT      PositionToView        (const float3* pPos, GXOUT float3* pView)); // like transform but it is float3 pos
  GXSTDINTERFACE(GXHRESULT      PositionToScreen      (const float3* pPos, GXOUT GXPOINT* ptScreen));
  GXSTDINTERFACE(GXHRESULT      PositionFromScreen    (const GXPOINT* pScreen, float fDepth, GXOUT float3* pWorldPos));
  GXSTDINTERFACE(GXHRESULT      PositionFromView      (const float3* pView, GXOUT float3* pWorldPos));
  GXSTDINTERFACE(GXHRESULT      RayFromScreen         (const GXPOINT* pScreen, GXOUT Ray* pRay));

  GXSTDINTERFACE(void           SetViewport           (GXVIEWPORT* pViewport));
  GXSTDINTERFACE(GXLPCVIEWPORT  GetViewport           () const);

  GXSTDINTERFACE(const FrustumPlanes*     GetViewFrustum        () const);
#ifdef REFACTOR_SHADER
#else
  GXSTDINTERFACE(STANDARDMTLUNIFORMTABLE* GetStandardUniform    ());  // 返回的数据可以在应用程序中修改
#endif // #ifdef REFACTOR_SHADER

  GXSTDINTERFACE(GXHRESULT      UpdateCommonUniforms  ());
};
#endif // _GRAP_X_CANVAS_3D_INTERFACE_H_