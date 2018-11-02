#ifndef _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
#define _IMPLEMENT_GRAP_X_CANVAS_CORE_H_

class GXGraphicsImpl;
class GTextureImpl;
class GSamplerStateImpl;
class GXRenderTargetImpl;

class GXCanvasCoreImpl : public GXCanvas
{
  friend class GXGraphicsImpl;
protected:
  GXGraphicsImpl*     m_pGraphics;
  GXSIZE              m_sExtent;
  //GXINT               m_xExt;          // 物理尺寸，不受原点位置影响
  //GXINT               m_yExt;
  GXRenderTargetImpl* m_pTargetTex;
  GXEffectImpl*       m_pEffectImpl;
  GBlendState*        m_pBlendState;
  GDepthStencilState* m_pDepthStencilState;
  GSamplerStateImpl*  m_pSamplerState;
  GCamera*            m_pCamera;
public:
  GXCanvasCoreImpl(GXGraphicsImpl* pGraphics, GXUINT nPriority, GXDWORD dwType);
  virtual ~GXCanvasCoreImpl();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef              ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXSIZE*         GetTargetDimension  (GXSIZE* pSize) const override;
  virtual GXRenderTarget* GetTargetUnsafe     () const override;
  virtual GXBOOL          Initialize          (GXRenderTarget* pTarget);
  virtual GXHRESULT       Invoke              (GRESCRIPTDESC* pDesc) override;
};

#endif // _IMPLEMENT_GRAP_X_CANVAS_CORE_H_