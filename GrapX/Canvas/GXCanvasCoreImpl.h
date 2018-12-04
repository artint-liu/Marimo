#ifndef _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
#define _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
class GraphicsImpl;
class TextureImpl;
class SamplerStateImpl;
class RenderTargetImpl;
class BlendStateImpl;

class CanvasCoreImpl : public Canvas
{
  friend class GraphicsImpl;
protected:
  GraphicsImpl*       m_pGraphics;
  GXSIZE              m_sExtent;    // m_pTargetTex 的尺寸，如果m_pTargetTex为空则是默认设备缓冲区的尺寸
  RenderTargetImpl*   m_pTargetTex;
  BlendStateImpl*     m_pBlendStateImpl;
  //DepthStencilState*  m_pDepthStencilState;
  SamplerStateImpl*   m_pSamplerState;
  GCamera*            m_pCamera;

  void CommitState();
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