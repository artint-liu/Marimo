// GrapX 3D 绘图对象
// 与GXCanvas不同的是, GXCanvas在绘图时锁定, 用完释放. GXCanvas3D 需要长期持有, 完全不用时才能释放
// GetXXX 返回指针为 Canvas3D 内部数据
#ifndef _GRAP_X_CANVAS_3D_INTERFACE_H_
#define _GRAP_X_CANVAS_3D_INTERFACE_H_

class GVSequence;
class GVNode;
struct STANDARDMTLUNIFORMTABLE;

namespace GrapX
{
  class Canvas3D : public CanvasCore
   {
  public:
    typedef clstd::geometry::Ray Ray;
    typedef clstd::geometry::FrustumPlanes FrustumPlanes;

  public:
    Canvas3D(GXUINT nPrioruty, GXDWORD dwType) : CanvasCore(nPrioruty, dwType) {}
    GXSTDINTERFACE(GXBOOL         SetTarget             (RenderTarget** pTargetArray, GXUINT count));
    GXSTDINTERFACE(GXBOOL         SetTarget             (RenderTarget* pTarget));
    GXSTDINTERFACE(RenderTarget*  GetTargetUnsafe       (GXUINT index) const);
    GXSTDINTERFACE(GXHRESULT      Invoke                (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(GXBOOL         Begin                 ());
    GXSTDINTERFACE(GXBOOL         End                   ());

    GXSTDINTERFACE(GXBOOL         Clear                 (GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags = GXCLEAR_ALL));
    GXSTDINTERFACE(GXBOOL         Clear                 (const GXColor& crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags = GXCLEAR_ALL));
    //GXSTDINTERFACE(GXHRESULT      Clear                 (GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil));

    GXSTDINTERFACE(GXHRESULT      Draw                  (GVSequence* pSequence));
    GXSTDINTERFACE(GXHRESULT      Draw                  (Material* pMaterial, GVNode* pNode, const float4x4* pTransform = NULL)); // 临时
    //GXSTDINTERFACE(GXHRESULT      Draw                  (Shader* pShader, GVNode* pNode, const float4x4* pTransform = NULL)); // 临时

    GXSTDINTERFACE(GXHRESULT      SetMaterial           (Material* pMtlInst));
    GXSTDINTERFACE(GXHRESULT      SetPrimitive          (Primitive* pPrimitive));
    GXSTDINTERFACE(void           SetCamera             (Camera* pCamera));
    GXSTDINTERFACE(Camera*        GetCameraUnsafe       ());
    GXSTDINTERFACE(GXHRESULT      GetDepthStencil       (Texture** ppDepthStencil));
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
    GXSTDINTERFACE(float          GetAspect             () const);

    GXSTDINTERFACE(const FrustumPlanes*     GetViewFrustum        ());
#ifdef REFACTOR_SHADER
#else
    GXSTDINTERFACE(STANDARDMTLUNIFORMTABLE* GetStandardUniform    ());  // 返回的数据可以在应用程序中修改
#endif // #ifdef REFACTOR_SHADER
    GXSTDINTERFACE(GXHRESULT      UpdateCommonUniforms  ());
  };
} // namespace GrapX
#endif // _GRAP_X_CANVAS_3D_INTERFACE_H_