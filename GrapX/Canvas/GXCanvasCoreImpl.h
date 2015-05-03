#ifndef _IMPLEMENT_GRAP_X_CANVAS_CORE_H_
#define _IMPLEMENT_GRAP_X_CANVAS_CORE_H_

class GXGraphicsImpl;
class GTextureImpl;
class GSamplerStateImpl;

class GXCanvasCoreImpl : public GXCanvas
{
  friend class GXGraphicsImpl;
protected:
  GXGraphicsImpl*     m_pGraphics;
  GXINT               m_xExt;          // 物理尺寸，不受原点位置影响
  GXINT               m_yExt;
  GTextureImpl*       m_pTargetTex;
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
  virtual GXVOID    GetTargetDimension  (GXSIZE* pSize) GXCONST;
  virtual GTexture* GetTargetUnsafe     () GXCONST;
  virtual GXBOOL    Initialize          (GTexture* pTexture);
  virtual GXHRESULT Invoke              (GRESCRIPTDESC* pDesc);
};

#endif // _IMPLEMENT_GRAP_X_CANVAS_CORE_H_