#ifndef _IMPLEMENT_GRAPH_X_EFFECT_H_
#define _IMPLEMENT_GRAPH_X_EFFECT_H_

class Graphics;
//class GShaderStub;
class GraphicsImpl;

namespace Marimo
{
  class DataPoolVariable;
} // namespace Marimo

namespace GrapX
{
  class EffectImpl : public Effect
  {
    struct TEXTUREUNIT
    {
      ObjectT<Texture> texture;
      MOVariable TexelSize;
      //ObjectT<SamplerState> sampler;
      //GXBOOL bTexelSize;
    };

    typedef clvector<TEXTUREUNIT> TextureUnitArray;
  protected:
    Graphics*           m_pGraphics;
    Shader*             m_pShader;
    Marimo::DataPool*   m_pDataPool;
    TextureUnitArray    m_aTextures;
    clstd::MemBuffer    m_DDBuffer;   // 设备相关

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

    GXBOOL InitEffect   ();
    GXBOOL Commit       ();
    GXBOOL UpdateTexelSize (GXUINT slot, Texture* pTexture);

    clstd::MemBuffer* GetDeviceDependBuffer();

    // 旧的兼容接口
    GXUINT GetHandle(const GXCHAR* pName) const;
  };

} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_EFFECT_H_