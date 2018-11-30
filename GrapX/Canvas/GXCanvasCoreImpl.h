#ifndef _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
#define _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
class GraphicsImpl;
class TextureImpl;
class SamplerStateImpl;
class RenderTargetImpl;

class CanvasCoreImpl : public Canvas
{
  friend class GraphicsImpl;
protected:
  GraphicsImpl*     m_pGraphics;
  GXSIZE              m_sExtent;
  //GXINT               m_xExt;          // 物理尺寸，不受原点位置影响
  //GXINT               m_yExt;
  RenderTargetImpl*   m_pTargetTex;
  //EffectImpl*         m_pEffectImpl;
  BlendState*         m_pBlendState;
  DepthStencilState*  m_pDepthStencilState;
  SamplerStateImpl*   m_pSamplerState;
  GCamera*            m_pCamera;
public:
  CanvasCoreImpl(GraphicsImpl* pGraphics, GXUINT nPriority, GXDWORD dwType);
  virtual ~CanvasCoreImpl();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef              ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXSIZE*         GetTargetDimension  (GXSIZE* pSize) const override;
  virtual RenderTarget*   GetTargetUnsafe     () const override;
  virtual GXBOOL          Initialize          (RenderTarget* pTarget);
  virtual GXHRESULT       Invoke              (GRESCRIPTDESC* pDesc) override;
};


#endif // _IMPLEMENT_GRAP_X_CANVAS_CORE_H_