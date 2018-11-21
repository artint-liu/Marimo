#ifndef _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
#define _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
class GraphicsImpl;
class GTextureImpl;
class GSamplerStateImpl;
class RenderTargetImpl;

class GXCanvasCoreImpl : public GXCanvas
{
  friend class GraphicsImpl;
protected:
  GraphicsImpl*     m_pGraphics;
  GXSIZE              m_sExtent;
  //GXINT               m_xExt;          // 物理尺寸，不受原点位置影响
  //GXINT               m_yExt;
  RenderTargetImpl*   m_pTargetTex;
  EffectImpl*         m_pEffectImpl;
  GBlendState*        m_pBlendState;
  GDepthStencilState* m_pDepthStencilState;
  GSamplerStateImpl*  m_pSamplerState;
  GCamera*            m_pCamera;
public:
  GXCanvasCoreImpl(GraphicsImpl* pGraphics, GXUINT nPriority, GXDWORD dwType);
  virtual ~GXCanvasCoreImpl();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef              ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXSIZE*         GetTargetDimension  (GXSIZE* pSize) const override;
  virtual RenderTarget*   GetTargetUnsafe     () const override;
  virtual GXBOOL          Initialize          (RenderTarget* pTarget);
  virtual GXHRESULT       Invoke              (GRESCRIPTDESC* pDesc) override;
};


#endif // _IMPLEMENT_GRAP_X_CANVAS_CORE_H_