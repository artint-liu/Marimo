// GrapX 3D ��ͼ����
// ��GXCanvas��ͬ����, GXCanvas�ڻ�ͼʱ����, �����ͷ�. GXCanvas3D ��Ҫ���ڳ���, ��ȫ����ʱ�����ͷ�

#ifndef _IMPLEMENT_GRAP_X_CANVAS_3D_H_
#define _IMPLEMENT_GRAP_X_CANVAS_3D_H_

//#define MRT_SUPPORT_COUNT 4
namespace GrapX
{
  class Canvas3DCommImpl : public Canvas3D
  {
  protected:
    Canvas3DCommImpl() : Canvas3D(2, RESTYPE_CANVAS3D) {}

    //  friend class GraphicsImpl;
    //protected:
    //  GraphicsImpl*             m_pGraphicsImpl;
    //  //GXINT                     m_xExt;          // ����ߴ磬����ԭ��λ��Ӱ��
    //  //GXINT                     m_yExt;
    //  GXSIZE                    m_sExtent;
    //  //GTexture*                 m_pDepthStencil;
    //  //RenderTargetImpl*         m_pTarget;
    //  ObjectT<RenderTargetImpl> m_pTargets[MRT_SUPPORT_COUNT];
    //  GXUINT                    m_nTargetCount = NULL;
    //  //GXImage*                  m_pImage;
    //  GXVIEWPORT                m_Viewport;
    //  Camera*                   m_pCamera;
    //
    //  FrustumPlanes             m_ViewFrustum;
    //  ObjectT<MaterialImpl>     m_CurMaterialImpl;
    //  BlendStateImpl*           m_pBlendState;
    //  DepthStencilStateImpl*    m_pCurDepthStencilState;
    //  SamplerState*             m_pSamplerState;
    //#ifdef REFACTOR_SHADER
    //  //clstd::FixedBuffer        m_CanvasUniformBuf;
    //  STD_CANVAS_UNIFORM        m_StdCanvasUniform;
    //#else
    //  STANDARDMTLUNIFORMTABLE   m_StdUniforms;
    //#endif // REFACTOR_SHADER
    //
    //protected:
    //  Canvas3DImpl(GraphicsImpl* pGraphics);
    //  virtual ~Canvas3DImpl();
    //
    //  void        SetupCanvasUniform                ();
    //  void        BroadcastCanvasUniformBufferSize  (GXSIZE_T cbSize);
    //
    //public:
    //
    //#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    //  GXHRESULT       AddRef                () override;
    //  GXHRESULT       Release               () override;
    //#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    //  GXHRESULT       Invoke                (GRESCRIPTDESC* pDesc) override;
    //  GXSIZE*         GetTargetDimension    (GXSIZE* pSize) const override;
    //  Graphics*       GetGraphicsUnsafe     () const override;
    //  RenderTarget*   GetTargetUnsafe       () const override;
    //  RenderTarget*   GetTargetUnsafe       (GXUINT index) const override;
    //  GXBOOL          Initialize            (RenderTarget** pTargetArray, size_t nCount, GXLPCVIEWPORT pViewport);
    //
    //  GXBOOL Clear(GXCOLOR crClear) override;
    //  GXBOOL Clear(const GXColor& crClear) override;
    //  GXBOOL Clear(GXFLOAT z, GXDWORD dwStencil) override;
    //  GXBOOL Clear(GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil) override;
    //  GXBOOL Clear(const GXColor& crClear, GXFLOAT z, GXDWORD dwStencil) override;
    //
    //  GXHRESULT   TransformPosition     (const float3* pPos, GXOUT float4* pView) override; // Transform world position to screen
    //  GXHRESULT   PositionToView        (const float3* pPos, GXOUT float3* pView) override; // like transform but it is float3 pos
    //  GXHRESULT   PositionToScreen      (const float3* pPos, GXOUT GXPOINT* ptScreen); // like transform but it is float3 pos
    //  GXHRESULT   PositionFromScreen    (const GXPOINT* pScreen, float fDepth, GXOUT float3* pWorldPos) override;
    //  GXHRESULT   PositionFromView      (const float3* pView, GXOUT float3* pWorldPos) override;
    //  GXHRESULT   RayFromScreen         (const GXPOINT* pScreen, GXOUT Ray* pRay) override;
    //
    //  void        SetWorldMatrix        (const float4x4& matWorld) override;
    //
    //  GXDWORD     GetGlobalHandle       (GXLPCSTR szName) override;
    //
    //  template<typename _Ty>
    //  inline GXHRESULT   SetCanvasUniformT     (GXDWORD dwGlobalHandle, const _Ty& rUniform);
    //
    //  GXHRESULT   SetCanvasFloat        (GXDWORD dwGlobalHandle, float fValue) override;
    //  GXHRESULT   SetCanvasVector       (GXDWORD dwGlobalHandle, const float4& rVector) override;
    //  GXHRESULT   SetCanvasMatrix       (GXDWORD dwGlobalHandle, const float4x4& rMatrix) override;
    //  GXHRESULT   SetCanvasFloat        (GXLPCSTR szName, float fValue) override;
    //  GXHRESULT   SetCanvasVector       (GXLPCSTR szName, const float4& rVector) override;
    //  GXHRESULT   SetCanvasMatrix       (GXLPCSTR szName, const float4x4& rMatrix) override;
    //
    //  void        SetViewport           (GXVIEWPORT* pViewport) override;
    //  GXLPCVIEWPORT GetViewport         () const override;
    //  float       GetAspect             () const override;
    //
    //  GXHRESULT   SetMaterial           (Material* pMaterial) override;
    //  GXHRESULT   SetPrimitive          (Primitive* pPrimitive) override;
    //  GXHRESULT   SetCamera             (Camera* pCamera) override;
    //  Camera*     GetCameraUnsafe       () override;
    //  GXHRESULT   Activate              () override;
    //  GXHRESULT   UpdateCommonUniforms  () override;
    //  GXHRESULT   Draw                  (GVSequence* pSequence) override;
    //  GXHRESULT   Draw                  (Shader* pShader, GVNode* pNode, const float4x4* pTransform) override;
    //
    //  GXHRESULT   GetDepthStencil       (Texture** ppDepthStencil) override;
    //
    //  const FrustumPlanes*      GetViewFrustum        ();
    //#ifdef REFACTOR_SHADER
    //#else
    //  STANDARDMTLUNIFORMTABLE*  GetStandardUniform    ();
    //#endif // REFACTOR_SHADER
  };
}

//float Canvas3DImpl::GetAspect() const
//{
//  return (float)m_Viewport.regn.w / (float)m_Viewport.regn.h;
//}

//////////////////////////////////////////////////////////////////////////
#endif // _IMPLEMENT_GRAP_X_CANVAS_3D_H_