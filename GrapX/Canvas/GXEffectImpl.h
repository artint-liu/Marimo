#ifndef _IMPLEMENT_GRAPH_X_EFFECT_H_
#define _IMPLEMENT_GRAPH_X_EFFECT_H_

class Graphics;
//class GShaderStub;
class GraphicsImpl;
namespace GrapX
{
#ifdef REFACTOR_GRAPX_SHADER
  class GXEffectImpl : public GXEffect
  {
  private:
    GrapX::Graphics*    m_pGraphics;
    GShaderStub*   m_pShaderStub;

  public:
    GXEffectImpl(GrapX::Graphics* pGraphics);
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
#else
  class EffectImpl : public Effect
  {
  protected:
    Graphics*           m_pGraphics;
    Shader*             m_pShader;
    Marimo::DataPool*   m_pDataPool;

  public:
    EffectImpl(Graphics* pGraphics, Shader* pShader);
    virtual ~EffectImpl();
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT    AddRef            () override;
    GXHRESULT    Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   Invoke             (GRESCRIPTDESC* pDesc) override;
    void BindTextureSlot(GXLPCSTR szTextureName, int nSlot) override;

    Graphics*  GetGraphicsUnsafe () const override;
    Shader*  GetShaderUnsafe() const;
    Marimo::DataPool* GetDataPoolUnsafe() const;
    Marimo::DataPoolVariable GetUniform(GXLPCSTR szName) override;

    GXBOOL InitEffect();
    //GXBOOL CommitUniform();


    // 旧的兼容接口
    GXUINT GetHandle(const GXCHAR* pName) const;
  };
#endif // #ifdef REFACTOR_GRAPX_SHADER
} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_