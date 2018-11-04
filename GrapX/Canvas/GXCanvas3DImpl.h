// GrapX 3D 绘图对象
// 与GXCanvas不同的是, GXCanvas在绘图时锁定, 用完释放. GXCanvas3D 需要长期持有, 完全不用时才能释放

#ifndef _IMPLEMENT_GRAP_X_CANVAS_3D_H_
#define _IMPLEMENT_GRAP_X_CANVAS_3D_H_

class GXCanvas3DImpl : public GXCanvas3D
{
  friend class GXGraphicsImpl;
  typedef GrapX::RenderTarget GXRenderTarget;
protected:
  GXGraphicsImpl*           m_pGraphicsImpl;
  //GXINT                     m_xExt;          // 物理尺寸，不受原点位置影响
  //GXINT                     m_yExt;
  GXSIZE                    m_sExtent;
  //GTexture*                 m_pDepthStencil;
  GXRenderTargetImpl*       m_pTarget;
  //GXImage*                  m_pImage;
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
  GXHRESULT       AddRef                () override;
  GXHRESULT       Release               () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT       Invoke                (GRESCRIPTDESC* pDesc) override;
  GXSIZE*         GetTargetDimension    (GXSIZE* pSize) const override;
  GXGraphics*     GetGraphicsUnsafe     () const override;
  GXRenderTarget* GetTargetUnsafe       () const override;
  GXBOOL          Initialize            (GXRenderTarget* pTarget, GXLPCVIEWPORT pViewport);

  GXHRESULT   Clear                 (GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil) override;
  GXHRESULT   TransformPosition     (const float3* pPos, GXOUT float4* pView) override; // Transform world position to screen
  GXHRESULT   PositionToView        (const float3* pPos, GXOUT float3* pView) override; // like transform but it is float3 pos
  GXHRESULT   PositionToScreen      (const float3* pPos, GXOUT GXPOINT* ptScreen); // like transform but it is float3 pos
  GXHRESULT   PositionFromScreen    (const GXPOINT* pScreen, float fDepth, GXOUT float3* pWorldPos) override;
  GXHRESULT   PositionFromView      (const float3* pView, GXOUT float3* pWorldPos) override;
  GXHRESULT   RayFromScreen         (const GXPOINT* pScreen, GXOUT Ray* pRay) override;
  
  void        SetWorldMatrix        (const float4x4& matWorld) override;

  GXDWORD     GetGlobalHandle       (GXLPCSTR szName) override;
  
  template<typename _Ty>
  inline GXHRESULT   SetCanvasUniformT     (GXDWORD dwGlobalHandle, const _Ty& rUniform);

  GXHRESULT   SetCanvasFloat        (GXDWORD dwGlobalHandle, float fValue) override;
  GXHRESULT   SetCanvasVector       (GXDWORD dwGlobalHandle, const float4& rVector) override;
  GXHRESULT   SetCanvasMatrix       (GXDWORD dwGlobalHandle, const float4x4& rMatrix) override;
  GXHRESULT   SetCanvasFloat        (GXLPCSTR szName, float fValue) override;
  GXHRESULT   SetCanvasVector       (GXLPCSTR szName, const float4& rVector) override;
  GXHRESULT   SetCanvasMatrix       (GXLPCSTR szName, const float4x4& rMatrix) override;

  void        SetViewport           (GXVIEWPORT* pViewport) override;
  GXLPCVIEWPORT GetViewport() const;

  GXHRESULT   SetMaterialInst       (GXMaterialInst* pMaterial) override;
  GXHRESULT   SetPrimitive          (GPrimitive* pPrimitive) override;
  GXHRESULT   SetCamera             (GCamera* pCamera) override;
  GCamera*    GetCameraUnsafe       () override;
  GXHRESULT   Activate              () override;
  GXHRESULT   UpdateCommonUniforms  () override;
  GXHRESULT   Draw                  (GVSequence* pSequence) override;

  GXHRESULT   GetDepthStencil       (GTexture** ppDepthStencil) override;

  const FrustumPlanes*      GetViewFrustum        ();
#ifdef REFACTOR_SHADER
#else
  STANDARDMTLUNIFORMTABLE*  GetStandardUniform    ();
#endif // REFACTOR_SHADER
};

//////////////////////////////////////////////////////////////////////////
#endif // _IMPLEMENT_GRAP_X_CANVAS_3D_H_