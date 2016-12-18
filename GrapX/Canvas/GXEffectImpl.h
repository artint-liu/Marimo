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
  GShader*      GetShaderUnsafe     () const;
  GShaderStub*  GetShaderStubUnsafe () const;
  GXUINT        GetHandle           (const GXCHAR* pName) const;
  bool          SetUniformByHandle  (GXCanvas* pCanvas, GXUINT uHandle, float* fValue, GXINT nFloatCount);

  // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT     AddRef                () override;
  virtual GXHRESULT     Release               () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT     Invoke                (GRESCRIPTDESC* pDesc) override;

  virtual GXHRESULT     SetTextureSlot        (GXLPCSTR pName, GXINT nSlot) override;
  virtual GXINT         GetTextureSlot        (GXLPCSTR pName) override;

  virtual GXUINT        GetConstantBufferSize () override;

  inline  GShader*      InlGetShaderUnsafe    () const;
  inline  GShaderStub*  InlGetShaderStubUnsafe() const;
};
//////////////////////////////////////////////////////////////////////////
inline GShader* GXEffectImpl::InlGetShaderUnsafe() const
{
  return m_pShaderStub->GetShaderUnsafe();
}

inline GShaderStub* GXEffectImpl::InlGetShaderStubUnsafe() const
{
  return m_pShaderStub;
}

#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_