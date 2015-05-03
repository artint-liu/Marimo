#ifndef _IMPLEMENT_GRAPH_X_EFFECT_H_
#define _IMPLEMENT_GRAPH_X_EFFECT_H_

class GXGraphics;
class GShaderStub;
class GXGraphicsImpl;

class GXEffectImpl : public GXEffect
{
private:
  GXGraphics*    m_pGraphics;
  GShaderStub*   m_pShaderStub;

public:
  GXEffectImpl(GXGraphics* pGraphics);
private:
  virtual ~GXEffectImpl();

public:
  GXHRESULT     SetShaderRef        (GShader* pShader);
  bool          CommitUniform       (GXCanvas* pCanvas, GXUINT uCommonOffset);
  GShader*      GetShaderUnsafe     () GXCONST;
  GShaderStub*  GetShaderStubUnsafe () GXCONST;
  GXUINT        GetHandle           (GXCONST GXCHAR* pName) GXCONST;
  bool          SetUniformByHandle  (GXCanvas* pCanvas, GXUINT uHandle, float* fValue, GXINT nFloatCount);

  // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT     AddRef                ();
  virtual GXHRESULT     Release               ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT     Invoke                (GRESCRIPTDESC* pDesc);

  virtual GXHRESULT     SetTextureSlot        (GXLPCSTR pName, GXINT nSlot);
  virtual GXINT         GetTextureSlot        (GXLPCSTR pName);

  virtual GXUINT        GetConstantBufferSize ();

  inline  GShader*      InlGetShaderUnsafe    () GXCONST;
  inline  GShaderStub*  InlGetShaderStubUnsafe() GXCONST;
};
//////////////////////////////////////////////////////////////////////////
inline GShader* GXEffectImpl::InlGetShaderUnsafe() GXCONST
{
  return m_pShaderStub->GetShaderUnsafe();
}

inline GShaderStub* GXEffectImpl::InlGetShaderStubUnsafe() GXCONST
{
  return m_pShaderStub;
}

#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_