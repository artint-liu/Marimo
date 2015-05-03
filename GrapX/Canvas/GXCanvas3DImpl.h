// GrapX 3D 绘图对象
// 与GXCanvas不同的是, GXCanvas在绘图时锁定, 用完释放. GXCanvas3D 需要长期持有, 完全不用时才能释放

#ifndef _IMPLEMENT_GRAP_X_CANVAS_3D_H_
#define _IMPLEMENT_GRAP_X_CANVAS_3D_H_

class GXCanvas3DImpl : public GXCanvas3D
{
  friend class GXGraphicsImpl;
protected:
  GXGraphicsImpl*           m_pGraphicsImpl;
  GXINT                     m_xExt;          // 物理尺寸，不受原点位置影响
  GXINT                     m_yExt;
  GTexture*                 m_pDepthStencil;
  GTexture*                 m_pTargetTex;
  GXImage*                  m_pImage;
  GXVIEWPORT                m_Viewport;
  GCamera*                  m_pCamera;

  FrustumPlanes             m_ViewFrustum;
  GBlendStateImpl*          m_pBlendState;
  GDepthStencilStateImpl*   m_pCurDepthStencilState;
  GSamplerState*            m_pSamplerState;
#ifdef REFACTOR_SHADER
  clstd::FixedBuffer        m_CanvasUniformBuf;
  STD_CANVAS_UNIFORM        m_StdCanvasUniform;
#else
  STANDARDMTLUNIFORMTABLE   m_StdUniforms;
#endif // REFACTOR_SHADER

protected:
  GXCanvas3DImpl(GXGraphicsImpl* pGraphics);
  virtual ~GXCanvas3DImpl();

  void        SetupCanvasUniform                ();
  void        BroadcastCanvasUniformBufferSize  (GXSIZE_T cbSize);

public:

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT   AddRef                ();
  GXHRESULT   Release               ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT   Invoke                (GRESCRIPTDESC* pDesc);
  GXVOID      GetTargetDimension    (GXSIZE* pSize) GXCONST;
  GXGraphics* GetGraphicsUnsafe     () GXCONST;
  GTexture*   GetTargetUnsafe       () GXCONST;
  GXBOOL      Initialize            (GXImage* pImage, GTexture* pDepthStencil, GXLPCVIEWPORT pViewport);

  GXHRESULT   Clear                 (GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil);
  GXHRESULT   TransformPosition     (const float3* pPos, GXOUT float4* pView); // Transform world position to screen
  GXHRESULT   PositionToView        (const float3* pPos, GXOUT float3* pView); // like transform but it is float3 pos
  GXHRESULT   PositionToScreen      (const float3* pPos, GXOUT GXPOINT* ptScreen); // like transform but it is float3 pos
  GXHRESULT   PositionFromScreen    (const GXPOINT* pScreen, float fDepth, GXOUT float3* pWorldPos);
  GXHRESULT   PositionFromView      (const float3* pView, GXOUT float3* pWorldPos);
  GXHRESULT   RayFromScreen         (const GXPOINT* pScreen, GXOUT Ray* pRay);
  
  void        SetWorldMatrix        (const float4x4& matWorld);

  GXDWORD     GetGlobalHandle       (GXLPCSTR szName);
  
  template<typename _Ty>
  inline GXHRESULT   SetCanvasUniformT     (GXDWORD dwGlobalHandle, const _Ty& rUniform);

  GXHRESULT   SetCanvasFloat        (GXDWORD dwGlobalHandle, float fValue);
  GXHRESULT   SetCanvasVector       (GXDWORD dwGlobalHandle, const float4& rVector);
  GXHRESULT   SetCanvasMatrix       (GXDWORD dwGlobalHandle, const float4x4& rMatrix);
  GXHRESULT   SetCanvasFloat        (GXLPCSTR szName, float fValue);
  GXHRESULT   SetCanvasVector       (GXLPCSTR szName, const float4& rVector);
  GXHRESULT   SetCanvasMatrix       (GXLPCSTR szName, const float4x4& rMatrix);

  void        SetViewport           (GXVIEWPORT* pViewport);
  GXLPCVIEWPORT GetViewport() GXCONST;

  GXHRESULT   SetMaterialInst       (GXMaterialInst* pMaterial);
  GXHRESULT   SetPrimitive          (GPrimitive* pPrimitive);
  GXHRESULT   SetCamera             (GCamera* pCamera);
  GCamera*    GetCameraUnsafe       ();
  GXHRESULT   Activate              ();
  GXHRESULT   UpdateCommonUniforms  ();
  GXHRESULT   Draw                  (GVSequence* pSequence);

  GXHRESULT   GetDepthStencil       (GTexture** ppDepthStencil) GXCONST;

  const FrustumPlanes*      GetViewFrustum        () GXCONST;
#ifdef REFACTOR_SHADER
#else
  STANDARDMTLUNIFORMTABLE*  GetStandardUniform    ();
#endif // REFACTOR_SHADER
};

//////////////////////////////////////////////////////////////////////////
#endif // _IMPLEMENT_GRAP_X_CANVAS_3D_H_