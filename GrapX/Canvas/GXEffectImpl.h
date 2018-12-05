#ifndef _IMPLEMENT_GRAPH_X_EFFECT_H_
#define _IMPLEMENT_GRAPH_X_EFFECT_H_

class Graphics;
//class GShaderStub;
class GraphicsImpl;
namespace GrapX
{
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
    GXHRESULT Clone(Effect** ppNewEffect) override;

    GXBOOL InitEffect();
    //GXBOOL CommitUniform();


    // ¾ÉµÄ¼æÈÝ½Ó¿Ú
    GXUINT GetHandle(const GXCHAR* pName) const;
  };

} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_