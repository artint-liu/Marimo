#ifdef ENABLE_GRAPHICS_API_DX11
#ifndef _GRAPH_X_CANVAS3D_D3D11_IMPLEMENT_H_
#define _GRAPH_X_CANVAS3D_D3D11_IMPLEMENT_H_

namespace GrapX
{
  class MaterialImpl;
  class Canvas3DCommImpl;
  namespace D3D11
  {
#define MRT_SUPPORT_COUNT D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT

    class Canvas3DImpl : public Canvas3DCommImpl
    {
      friend class GraphicsImpl;
    protected:
      GraphicsImpl*             m_pGraphicsImpl;
      //GXINT                     m_xExt;          // 物理尺寸，不受原点位置影响
      //GXINT                     m_yExt;
      GXSIZE                    m_sExtent;
      //GTexture*                 m_pDepthStencil;
      //RenderTargetImpl*         m_pTarget;
      ObjectT<RenderTargetImpl> m_pTargets[MRT_SUPPORT_COUNT];
      GXUINT                    m_nTargetCount = NULL;
      //GXImage*                  m_pImage;
      GXVIEWPORT                m_Viewport;
      Camera*                   m_pCamera;

      FrustumPlanes             m_ViewFrustum;
      ObjectT<MaterialImpl>     m_CurMaterialImpl;
      BlendStateImpl*           m_pBlendState;
      DepthStencilStateImpl*    m_pCurDepthStencilState;
      SamplerState*             m_pSamplerState;
#ifdef REFACTOR_SHADER
      //clstd::FixedBuffer        m_CanvasUniformBuf;
      STD_CANVAS_UNIFORM        m_StdCanvasUniform;
#else
      STANDARDMTLUNIFORMTABLE   m_StdUniforms;
#endif // REFACTOR_SHADER

    protected:
      Canvas3DImpl(GraphicsImpl* pGraphics);
      virtual ~Canvas3DImpl();

      void        SetupCanvasUniform                ();
      void        BroadcastCanvasUniformBufferSize  (GXSIZE_T cbSize);

    public:

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT       AddRef                () override;
      GXHRESULT       Release               () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXBOOL          Begin                 ();
      GXBOOL          End                   ();
      GXHRESULT       Invoke                (GRESCRIPTDESC* pDesc) override;
      GXSIZE*         GetTargetDimension    (GXSIZE* pSize) const override;
      Graphics*       GetGraphicsUnsafe     () const override;
      RenderTarget*   GetTargetUnsafe       () const override;
      RenderTarget*   GetTargetUnsafe       (GXUINT index) const override;
      GXBOOL          Initialize            (RenderTarget** pTargetArray, size_t nCount, GXLPCVIEWPORT pViewport);

      GXBOOL Clear(GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags) override;
      GXBOOL Clear(const GXColor& crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags) override;

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
      GXLPCVIEWPORT GetViewport         () const override;
      float       GetAspect             () const override;

      GXHRESULT   SetMaterial           (Material* pMaterial) override;
      GXHRESULT   SetPrimitive          (Primitive* pPrimitive) override;
      GXHRESULT   SetCamera             (Camera* pCamera) override;
      Camera*     GetCameraUnsafe       () override;
      //GXHRESULT   Activate              () override;
      GXHRESULT   UpdateCommonUniforms  () override;
      GXHRESULT   Draw                  (GVSequence* pSequence) override;
      GXHRESULT   Draw                  (Shader* pShader, GVNode* pNode, const float4x4* pTransform) override;

      GXHRESULT   GetDepthStencil       (Texture** ppDepthStencil) override;

      const FrustumPlanes*      GetViewFrustum        ();
#ifdef REFACTOR_SHADER
#else
      STANDARDMTLUNIFORMTABLE*  GetStandardUniform    ();
#endif // REFACTOR_SHADER
    };
  }
}

#endif // #ifndef _GRAPH_X_CANVAS3D_D3D11_IMPLEMENT_H_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11