#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_
#define _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_

namespace D3D11
{
  class GVertexDeclImpl : public GVertexDeclaration
  {
    friend class GXGraphicsImpl;
  private:
    typedef GrapXToDX11::GXD3D11InputElementDescArray DescArray;

    GXGraphicsImpl*       m_pGraphics;
    DescArray             m_aDescs;
    UINT                  m_NumDescs;
    GXUINT                m_nStride;
    LPGXVERTEXELEMENT     m_pVertexElement;

  protected:
    GVertexDeclImpl(GXGraphicsImpl* pGraphics);
    virtual ~GVertexDeclImpl();

  public:
    GXHRESULT Initialize(LPCGXVERTEXELEMENT lpVertexElement);
    GXHRESULT Activate();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT           AddRef            ();
    virtual GXHRESULT           Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT           Invoke            (GRESCRIPTDESC* pDesc) { return GX_OK; }

    virtual GXUINT              GetStride         ();
    virtual GXINT               GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc);
    virtual GXLPCVERTEXELEMENT  GetVertexElement  ();
  };
} // namespace D3D11

#endif // #ifndef _VERTEX_DECLARATION_D3D11_IMPLEMENT_HEADER_FILE_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
