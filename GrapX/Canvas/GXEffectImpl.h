#ifndef _IMPLEMENT_GRAPH_X_EFFECT_H_
#define _IMPLEMENT_GRAPH_X_EFFECT_H_

class Graphics;
//class GShaderStub;
class GraphicsImpl;
namespace GrapX
{
  class EffectImpl : public Effect
  {
    typedef clvector<ObjectT<Texture> > TextureArray;
  protected:
    Graphics*           m_pGraphics;
    Shader*             m_pShader;
    Marimo::DataPool*   m_pDataPool;
    TextureArray        m_aTextures;

  public:
    EffectImpl(Graphics* pGraphics, Shader* pShader);
    virtual ~EffectImpl();
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT    AddRef           () override;
    GXHRESULT    Release          () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc) override;
    GXBOOL      SetTexture        (GXUINT nSlot, Texture* pTexture) override;
    GXBOOL      SetTexture        (GXLPCSTR szSamplerName, Texture* pTexture) override;

    //void BindTextureSlot(GXLPCSTR szTextureName, int nSlot) override;

    Graphics*  GetGraphicsUnsafe () const override;
    Shader*  GetShaderUnsafe() const;
    Marimo::DataPool* GetDataPoolUnsafe() const;
    Marimo::DataPoolVariable GetUniform(GXLPCSTR szName) override;
    GXHRESULT Clone(Effect** ppNewEffect) override;

    GXBOOL InitEffect();
    GXBOOL Commit();


    // �ɵļ��ݽӿ�
    GXUINT GetHandle(const GXCHAR* pName) const;
  };

} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_