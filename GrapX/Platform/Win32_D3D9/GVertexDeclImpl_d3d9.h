#if defined(ENABLE_GRAPHICS_API_DX9)
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _VERTEX_DECLARATION_D3D9_IMPLEMENT_HEADER_FILE_
#define _VERTEX_DECLARATION_D3D9_IMPLEMENT_HEADER_FILE_

namespace D3D9
{
  class GVertexDeclImpl : public GVertexDeclaration
  {
    friend class GraphicsImpl;
  private:
    GraphicsImpl*                 m_pGraphics;
    IDirect3DVertexDeclaration9*    m_pDecl;
    GXUINT                          m_nStride;
    LPGXVERTEXELEMENT               m_pVertexElement;

  protected:
    GVertexDeclImpl(GraphicsImpl* pGraphics);
    virtual ~GVertexDeclImpl();

  public:
    GXHRESULT Initialize(LPCGXVERTEXELEMENT lpVertexElement);
    GXHRESULT Activate();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT           AddRef            () override;
    virtual GXHRESULT           Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    //virtual GXHRESULT           OnDeviceEvent     (DeviceEvent eEvent);
    virtual GXHRESULT           Invoke            (GRESCRIPTDESC* pDesc) override { return GX_OK; }

    virtual GXUINT              GetStride         () override;
    virtual GXINT               GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc) override;
    virtual GXLPCVERTEXELEMENT  GetVertexElement  () override;
  };
} // namespace D3D9

#endif // #ifndef _VERTEX_DECLARATION_D3D9_IMPLEMENT_HEADER_FILE_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #if defined(ENABLE_GRAPHICS_API_DX9)